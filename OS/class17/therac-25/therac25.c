#include "thread.h"
#include "thread-sync.h"
#include <time.h>
#include <sys/select.h>
#include <termios.h>
#include <stdbool.h>
#include <stdarg.h>
#include <unistd.h>

// ── Frame buffer: build screen in memory, write atomically ────
typedef struct {
    char data[16384];
    int  len;
} framebuf_t;

static void fb_append(framebuf_t *fb, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    fb->len += vsnprintf(fb->data + fb->len, sizeof(fb->data) - fb->len, fmt, ap);
    va_end(ap);
}

static void fb_flush(framebuf_t *fb) {
    write(STDOUT_FILENO, fb->data, fb->len);
    fb->len = 0;
}

// ── ANSI escape helpers (append to frame buffer) ─────────────
#define FB_CLEAR(fb)    fb_append(fb, "\033[2J\033[H")
#define FB_GOTO(fb,r,c) fb_append(fb, "\033[%d;%dH", (r), (c))
#define FB_RESET(fb)    fb_append(fb, "\033[0m")
#define FB_RED(fb)      fb_append(fb, "\033[31;1m")
#define FB_GREEN(fb)    fb_append(fb, "\033[32;1m")
#define FB_YELLOW(fb)   fb_append(fb, "\033[33;1m")
#define FB_CYAN(fb)     fb_append(fb, "\033[36m")
#define FB_DIM(fb)      fb_append(fb, "\033[2m")
#define FB_BOLD(fb)     fb_append(fb, "\033[1m")

// ── Mode constants ───────────────────────────────────────────
enum { MODE_ELECTRON = 0, MODE_XRAY = 1 };

// Beam flattener travel: 0 = retracted (electron), 100 = inserted (xray)
#define FLATTENER_RETRACTED 0
#define FLATTENER_INSERTED  100
#define FLATTENER_SPEED     1    // units per step; 100 steps * 100ms ≈ 10s travel

// ── Shared state ─────────────────────────────────────────────
int mode           = MODE_ELECTRON;   // current commanded mode
int flattener_pos  = FLATTENER_RETRACTED;
bool beam_on       = false;
bool malfunction   = false;
bool continued     = false;  // operator pressed "continue" after malfunction
bool fatal         = false;  // lethal dose delivered

mutex_t lk = MUTEX_INIT();
cond_t  cv = COND_INIT();

// ── Keyboard input (non-blocking) ────────────────────────────
struct termios orig_termios;

static void restore_term(void) {
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
}

static void setup_term(void) {
    struct termios t;
    tcgetattr(STDIN_FILENO, &orig_termios);
    t = orig_termios;
    t.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
    atexit(restore_term);
}

static int read_key(void) {
    struct timeval tv = {0, 50000}; // 50ms
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    if (select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0) {
        char c;
        if (read(STDIN_FILENO, &c, 1) == 1)
            return c;
    }
    return -1;
}

// ── Drawing helpers ──────────────────────────────────────────
static void draw_box(framebuf_t *fb, int row, int col, int h, int w, const char *title) {
    FB_GOTO(fb, row, col);
    fb_append(fb, "┌");
    for (int i = 0; i < w - 2; i++) fb_append(fb, "─");
    fb_append(fb, "┐");
    for (int r = 1; r < h - 1; r++) {
        FB_GOTO(fb, row + r, col);
        fb_append(fb, "│");
        for (int i = 0; i < w - 2; i++) fb_append(fb, " ");
        fb_append(fb, "│");
    }
    FB_GOTO(fb, row + h - 1, col);
    fb_append(fb, "└");
    for (int i = 0; i < w - 2; i++) fb_append(fb, "─");
    fb_append(fb, "┘");
    if (title) {
        FB_GOTO(fb, row, col + 2);
        fb_append(fb, " %s ", title);
    }
}

// Draw beam flattener as a physical bar inside the beam path
static void draw_flattener(framebuf_t *fb, int row, int col, int pos) {
    // pos: 0=retracted(right side), 100=inserted(beam path)
    // The flattener slides horizontally into the beam path
    int max_slide = 16;
    int slide = pos * max_slide / 100;

    FB_GOTO(fb, row, col);
    FB_DIM(fb);
    fb_append(fb, "Beam path: ");
    FB_RESET(fb);

    // Draw beam tunnel (20 chars wide)
    int tunnel_start = col + 11;
    for (int i = 0; i < 20; i++) {
        FB_GOTO(fb, row, tunnel_start + i);
        if (i >= (20 - slide) && i < (20 - slide) + 4) {
            FB_YELLOW(fb);
            fb_append(fb, "█");
            FB_RESET(fb);
        } else {
            FB_DIM(fb);
            fb_append(fb, "·");
            FB_RESET(fb);
        }
    }

    // Status label
    FB_GOTO(fb, row + 1, col);
    if (pos >= FLATTENER_INSERTED) {
        FB_GREEN(fb);
        fb_append(fb, "  Flattener: INSERTED");
        FB_RESET(fb);
    } else if (pos <= FLATTENER_RETRACTED) {
        FB_RED(fb);
        fb_append(fb, "  Flattener: RETRACTED");
        FB_RESET(fb);
    } else {
        FB_YELLOW(fb);
        fb_append(fb, "  Flattener: MOVING... %d%%", pos);
        FB_RESET(fb);
    }
}

// Draw beam firing from source to patient
static void draw_beam(framebuf_t *fb, int row, int col, int current_mode, int fpos, bool on) {
    if (!on) return;

    int beam_start = col + 11;
    int beam_len = 20;
    int flattener_slide = fpos * 16 / 100;
    int flat_start = 20 - flattener_slide;

    for (int i = 0; i < beam_len; i++) {
        FB_GOTO(fb, row, beam_start + i);
        if (current_mode == MODE_XRAY) {
            if (i >= flat_start && i < flat_start + 4) {
                FB_YELLOW(fb);
            } else {
                FB_RED(fb);
            }
            fb_append(fb, "≡");
        } else {
            FB_GREEN(fb);
            fb_append(fb, "~");
        }
        FB_RESET(fb);
    }
}

// ── Flattener motor thread ───────────────────────────────────
void flattener_thread(int id) {
    while (1) {
        mutex_lock(&lk);
        int target = (mode == MODE_XRAY) ? FLATTENER_INSERTED : FLATTENER_RETRACTED;
        mutex_unlock(&lk);

        // Move toward target
        if (flattener_pos < target) {
            flattener_pos += FLATTENER_SPEED;
            if (flattener_pos > target) flattener_pos = target;
        } else if (flattener_pos > target) {
            flattener_pos -= FLATTENER_SPEED;
            if (flattener_pos < target) flattener_pos = target;
        }

        usleep(100000); // 100ms per step
    }
}

// ── Display thread ───────────────────────────────────────────
void display_thread(int id) {
    framebuf_t fb;

    while (1) {
        fb.len = 0;

        mutex_lock(&lk);
        int m = mode;
        int fp = flattener_pos;
        bool bo = beam_on;
        bool mf = malfunction;
        bool ft = fatal;
        mutex_unlock(&lk);

        FB_CLEAR(&fb);

        // Title
        FB_GOTO(&fb, 1, 2);
        FB_BOLD(&fb);
        fb_append(&fb, "╔══════════════════════════════════════╗\n");
        FB_GOTO(&fb, 2, 2);
        fb_append(&fb, "║    THERAC-25 Radiation Simulator     ║\n");
        FB_GOTO(&fb, 3, 2);
        fb_append(&fb, "╚══════════════════════════════════════╝\n");
        FB_RESET(&fb);

        // Mode display
        FB_GOTO(&fb, 5, 2);
        fb_append(&fb, "Current Mode: ");
        if (m == MODE_XRAY) {
            FB_RED(&fb);
            fb_append(&fb, "X-RAY (HIGH ENERGY) 25 MeV");
            FB_RESET(&fb);
        } else {
            FB_GREEN(&fb);
            fb_append(&fb, "ELECTRON (LOW ENERGY)");
            FB_RESET(&fb);
        }

        // Machine diagram
        draw_box(&fb, 7, 2, 7, 40, "Machine Cross-Section");

        // Source label
        FB_GOTO(&fb, 9, 4);
        if (bo && m == MODE_XRAY) { FB_RED(&fb); } else if (bo) { FB_GREEN(&fb); } else { FB_DIM(&fb); }
        fb_append(&fb, " SOURCE ");
        FB_RESET(&fb);

        // Patient label (below flattener, as in real Therac-25)
        FB_GOTO(&fb, 13, 20);
        FB_CYAN(&fb);
        fb_append(&fb, " PATIENT ");
        FB_RESET(&fb);

        // Draw beam
        if (bo) {
            draw_beam(&fb, 9, 4, m, fp, bo);
        }

        // Draw flattener
        draw_flattener(&fb, 11, 4, fp);

        // Beam status
        FB_GOTO(&fb, 15, 2);
        if (bo) {
            if (m == MODE_XRAY && fp < FLATTENER_INSERTED) {
                FB_RED(&fb); FB_BOLD(&fb);
                fb_append(&fb, "⚠ X-RAY BEAM ACTIVE WITHOUT FLATTENER!");
                FB_RESET(&fb);
            } else if (m == MODE_XRAY) {
                FB_YELLOW(&fb);
                fb_append(&fb, "● X-Ray beam active (flattened - safe)");
                FB_RESET(&fb);
            } else {
                FB_GREEN(&fb);
                fb_append(&fb, "● Electron beam active");
                FB_RESET(&fb);
            }
        } else {
            FB_DIM(&fb);
            fb_append(&fb, "  Beam OFF");
            FB_RESET(&fb);
        }

        // Malfunction / Fatal alerts
        if (ft) {
            FB_GOTO(&fb, 17, 2);
            FB_RED(&fb); FB_BOLD(&fb);
            fb_append(&fb, "╔══════════════════════════════════════════╗\n");
            FB_GOTO(&fb, 18, 2);
            fb_append(&fb, "║  ☠ FATAL RADIATION OVERDOSE DELIVERED!  ║\n");
            FB_GOTO(&fb, 19, 2);
            fb_append(&fb, "║     High-energy beam hit patient         ║\n");
            FB_GOTO(&fb, 20, 2);
            fb_append(&fb, "║     WITHOUT beam flattener protection!   ║\n");
            FB_GOTO(&fb, 21, 2);
            fb_append(&fb, "╚══════════════════════════════════════════╝\n");
            FB_RESET(&fb);
        } else if (mf) {
            FB_GOTO(&fb, 17, 2);
            FB_RED(&fb); FB_BOLD(&fb);
            fb_append(&fb, "╔═══════════════════════════════════════╗\n");
            FB_GOTO(&fb, 18, 2);
            fb_append(&fb, "║   MALFUNCTION 54                      ║\n");
            FB_GOTO(&fb, 19, 2);
            fb_append(&fb, "║   Treatment paused.                   ║\n");
            FB_GOTO(&fb, 20, 2);
            fb_append(&fb, "║                                       ║\n");
            FB_GOTO(&fb, 21, 2);
            fb_append(&fb, "║   Press 'c' to CONTINUE at your risk  ║\n");
            FB_GOTO(&fb, 22, 2);
            fb_append(&fb, "╚═══════════════════════════════════════╝\n");
            FB_RESET(&fb);
        }

        // Controls
        FB_GOTO(&fb, 24, 2);
        fb_append(&fb, "Commands: [e] Electron  [x] X-Ray  [b] Beam ON  [o] Beam OFF  [q] Quit");

        fb_flush(&fb);
        usleep(150000); // ~7 FPS — leisurely pace for a radiation therapy machine
    }
}

// ── Input thread ─────────────────────────────────────────────
void input_thread(int id) {
    while (1) {
        int ch = read_key();
        if (ch < 0) continue;

        mutex_lock(&lk);
        switch (ch) {
        case 'e':
            mode = MODE_ELECTRON;
            break;
        case 'x':
            mode = MODE_XRAY;
            break;
        case 'b':
            if (!malfunction) {
                beam_on = true;
                if (mode == MODE_XRAY && flattener_pos < FLATTENER_INSERTED) {
                    fatal = true;
                }
            }
            break;
        case 'o':
            beam_on = false;
            break;
        case 'c':
            if (malfunction && !continued) {
                // Operator "continues" past the error — the fatal mistake.
                // Habit from frequent false alarms. Beam fires immediately
                // with whatever (wrong) state the machine is in.
                continued = true;
                malfunction = false;
                beam_on = true;
                if (mode == MODE_XRAY && flattener_pos < FLATTENER_INSERTED) {
                    fatal = true;
                }
            }
            break;
        case 'q':
            restore_term();
            exit(0);
        }
        mutex_unlock(&lk);
    }
}

// ── Monitor thread: detects the race condition ───────────────
// The Therac-25 bug: rapid X-Ray → Electron → X-Ray switching
// confuses the software's shared state. A stale "data entry ok"
// flag from the first X-Ray selection persists through the switch,
// so the machine thinks configuration is complete when it isn't.
// This triggers Malfunction 54 (assertion failure).
void monitor_thread(int id) {
    int prev_mode = MODE_ELECTRON;
    int xray_phase = 0; // 0=idle, 1=saw first X-Ray, 2=saw Electron after X-Ray

    while (1) {
        mutex_lock(&lk);
        int m = mode;

        if (prev_mode != m) {
            if (m == MODE_XRAY && xray_phase == 0) {
                xray_phase = 1; // First X-Ray selection
            } else if (m == MODE_ELECTRON && xray_phase == 1) {
                xray_phase = 2; // Back to Electron while flattener moving
            } else if (m == MODE_XRAY && xray_phase == 2) {
                // Second X-Ray while flattener still in transit → BUG
                if (flattener_pos < FLATTENER_INSERTED && !malfunction) {
                    malfunction = true;
                }
                xray_phase = 0;
            }
        }

        // Reset tracker once flattener reaches target
        if (flattener_pos >= FLATTENER_INSERTED) {
            xray_phase = 0;
        }

        prev_mode = m;
        mutex_unlock(&lk);

        usleep(10000);
    }
}

// ── Main ─────────────────────────────────────────────────────
int main() {
    setbuf(stdout, NULL);
    setup_term();

    spawn(flattener_thread);
    spawn(display_thread);
    spawn(input_thread);
    spawn(monitor_thread);
}
