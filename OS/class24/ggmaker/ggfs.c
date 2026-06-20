/*
 * ggfs.c — FUSE-based visual novel filesystem
 *
 * Flat filesystem: only files at /, no directories.
 * - Reading a choice file returns its filename.
 * - Writing to a choice file transitions to the next scene.
 * - .scene.jpg serves the current scene's image.
 * - .description shows current scene text.
 * - State is kept in memory (hidden from the user).
 *
 * Reuses script.json and img/ from the symlink-based version.
 *
 * Usage: ./ggfs script.json /tmp/ggfs
 */

#define FUSE_USE_VERSION 26
#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>

#include "cJSON.h"

#define MAX_SCENES   64
#define MAX_CHOICES  16
#define MAX_NAME     64
#define MAX_DESC     512
#define MAX_FNAME    256

typedef struct {
    char filename[MAX_FNAME];
    char next[MAX_NAME];
} Choice;

typedef struct {
    char name[MAX_NAME];
    char description[MAX_DESC];
    char image[MAX_FNAME];
    char end_title[MAX_DESC];
    int  num_choices;
    Choice choices[MAX_CHOICES];
} Scene;

typedef struct {
    char  title[MAX_NAME];
    Scene scenes[MAX_SCENES];
    int   num_scenes;
    int   current;
} Game;

static Game game;
static char base_dir[4096];
static pthread_mutex_t game_lock = PTHREAD_MUTEX_INITIALIZER;

static Scene *cur_scene(void) {
    return &game.scenes[game.current];
}

static int img_path(char *buf, size_t bufsz) {
    const char *img = cur_scene()->image;
    if (!img[0]) return -1;
    const char *base = strrchr(img, '/');
    base = base ? base + 1 : img;
    return snprintf(buf, bufsz, "%s/img/%s", base_dir, base);
}

static void load_script(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) { perror(path); exit(1); }
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *data = malloc(len + 1);
    fread(data, 1, len, f);
    data[len] = '\0';
    fclose(f);

    cJSON *root = cJSON_Parse(data);
    free(data);
    if (!root) { fprintf(stderr, "JSON parse error\n"); exit(1); }

    cJSON *title = cJSON_GetObjectItem(root, "title");
    if (title && cJSON_IsString(title))
        strncpy(game.title, title->valuestring, MAX_NAME - 1);

    game.num_scenes = 0;
    cJSON *item = NULL;
    cJSON_ArrayForEach(item, root) {
        if (!cJSON_IsObject(item)) continue;
        if (strcmp(item->string, "title") == 0) continue;

        Scene *s = &game.scenes[game.num_scenes++];
        strncpy(s->name, item->string, MAX_NAME - 1);

        cJSON *d = cJSON_GetObjectItem(item, "description");
        if (d && cJSON_IsString(d))
            strncpy(s->description, d->valuestring, MAX_DESC - 1);

        cJSON *im = cJSON_GetObjectItem(item, "image");
        if (im && cJSON_IsString(im))
            strncpy(s->image, im->valuestring, MAX_FNAME - 1);

        cJSON *et = cJSON_GetObjectItem(item, "endTitle");
        if (et && cJSON_IsString(et))
            strncpy(s->end_title, et->valuestring, MAX_DESC - 1);

        s->num_choices = 0;
        cJSON *child = NULL;
        cJSON_ArrayForEach(child, item) {
            if (!cJSON_IsObject(child)) continue;
            int is_digit = 1;
            for (const char *p = child->string; *p; p++)
                if (*p < '0' || *p > '9') { is_digit = 0; break; }
            if (!is_digit) continue;

            Choice *c = &s->choices[s->num_choices++];
            cJSON *cd = cJSON_GetObjectItem(child, "description");
            cJSON *cn = cJSON_GetObjectItem(child, "next");
            snprintf(c->filename, MAX_FNAME, "%s.%s",
                     child->string,
                     (cd && cJSON_IsString(cd)) ? cd->valuestring : "");
            if (cn && cJSON_IsString(cn))
                strncpy(c->next, cn->valuestring, MAX_NAME - 1);
        }
    }

    cJSON_Delete(root);

    /* find "start" scene by name, not by position */
    game.current = 0;
    for (int i = 0; i < game.num_scenes; i++) {
        if (strcmp(game.scenes[i].name, "start") == 0) {
            game.current = i;
            break;
        }
    }
}

/* ---- Scene display (forked child writes to /dev/tty) ---- */

static void show_scene(int scene_idx) {
    pid_t pid = fork();
    if (pid != 0) return;  /* parent returns immediately */

    /* child: redirect stdout/stderr to terminal */
    int tty = open("/dev/tty", O_WRONLY);
    if (tty >= 0) {
        dup2(tty, STDOUT_FILENO);
        dup2(tty, STDERR_FILENO);
        close(tty);
    }

    Scene *sc = &game.scenes[scene_idx];

    /* clear screen */
    system("clear 2>/dev/null");

    /* display image */
    if (sc->image[0]) {
        const char *img = sc->image;
        const char *base = strrchr(img, '/');
        base = base ? base + 1 : img;
        char ip[4096];
        snprintf(ip, sizeof(ip), "%s/img/%s", base_dir, base);
        char cmd[8192];
        snprintf(cmd, sizeof(cmd),
            "kitten icat '%s' 2>/dev/null || img2sixel '%s' 2>/dev/null",
            ip, ip);
        system(cmd);
    }

    printf("\n\n");

    /* typewriter effect for description (skip if ending scene) */
    if (sc->description[0] && !sc->end_title[0]) {
        char tmpfile[] = "/tmp/gg_desc_XXXXXX";
        int tfd = mkstemp(tmpfile);
        if (tfd >= 0) {
            write(tfd, sc->description, strlen(sc->description));
            close(tfd);
            char cmd[512];
            snprintf(cmd, sizeof(cmd),
                "cat '%s' | python3 -c 'import sys,time;"
                "[print(c,end=\"\",flush=True) or time.sleep(0.03)"
                " for c in sys.stdin.read()]'",
                tmpfile);
            system(cmd);
            unlink(tmpfile);
        }
    }

    /* ending scene */
    if (sc->end_title[0]) {
        printf("\n  *** %s ***\n", sc->end_title);
        if (sc->description[0])
            printf("  %s\n", sc->description);
    }

    /* list choices */
    if (sc->num_choices > 0 && !sc->end_title[0]) {
        printf("\n---\n");
        for (int i = 0; i < sc->num_choices; i++)
            printf("%s\n", sc->choices[i].filename);
    }

    printf("\n");
    fflush(stdout);
    _exit(0);
}

/* ---- FUSE callbacks ---- */

static int ggfs_getattr(const char *path, struct stat *st) {
    memset(st, 0, sizeof(*st));

    if (strcmp(path, "/") == 0) {
        st->st_mode = S_IFDIR | 0755;
        st->st_nlink = 2;
        st->st_uid = getuid();
        st->st_gid = getgid();
        st->st_atime = st->st_mtime = st->st_ctime = time(NULL);
        return 0;
    }

    pthread_mutex_lock(&game_lock);
    const char *name = path + 1;
    Scene *sc = cur_scene();
    int found = 0;

    /* .scene.jpg */
    if (strcmp(name, ".scene.jpg") == 0 && sc->image[0]) {
        char ip[4096];
        img_path(ip, sizeof(ip));
        struct stat imst;
        if (stat(ip, &imst) == 0) {
            st->st_mode = S_IFREG | 0444;
            st->st_nlink = 1;
            st->st_size = imst.st_size;
            st->st_uid = getuid();
            st->st_gid = getgid();
            st->st_atime = imst.st_atime;
            st->st_mtime = imst.st_mtime;
            st->st_ctime = imst.st_ctime;
            found = 1;
        }
    }

    /* .description */
    if (!found && strcmp(name, ".description") == 0 && sc->description[0]) {
        st->st_mode = S_IFREG | 0444;
        st->st_nlink = 1;
        st->st_size = strlen(sc->description) + 1;
        st->st_uid = getuid();
        st->st_gid = getgid();
        st->st_atime = st->st_mtime = st->st_ctime = time(NULL);
        found = 1;
    }

    /* .ending */
    if (!found && strcmp(name, ".ending") == 0 && sc->end_title[0]) {
        st->st_mode = S_IFREG | 0444;
        st->st_nlink = 1;
        size_t elen = strlen(sc->end_title) + strlen(sc->description) + 2;
        st->st_size = elen;
        st->st_uid = getuid();
        st->st_gid = getgid();
        st->st_atime = st->st_mtime = st->st_ctime = time(NULL);
        found = 1;
    }

    /* choice files */
    if (!found) {
        for (int i = 0; i < sc->num_choices; i++) {
            if (strcmp(name, sc->choices[i].filename) == 0) {
                st->st_mode = S_IFREG | 0666;
                st->st_nlink = 1;
                st->st_size = 4096;
                st->st_uid = getuid();
                st->st_gid = getgid();
                st->st_atime = st->st_mtime = st->st_ctime = time(NULL);
                found = 1;
                break;
            }
        }
    }

    pthread_mutex_unlock(&game_lock);
    return found ? 0 : -ENOENT;
}

static int ggfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t off, struct fuse_file_info *fi) {
    (void)off; (void)fi;
    if (strcmp(path, "/") != 0) return -ENOTDIR;

    pthread_mutex_lock(&game_lock);
    Scene *sc = cur_scene();

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    for (int i = 0; i < sc->num_choices; i++)
        filler(buf, sc->choices[i].filename, NULL, 0);
    if (sc->image[0])
        filler(buf, ".scene.jpg", NULL, 0);
    if (sc->description[0])
        filler(buf, ".description", NULL, 0);
    if (sc->end_title[0])
        filler(buf, ".ending", NULL, 0);

    pthread_mutex_unlock(&game_lock);
    return 0;
}

static int ggfs_open(const char *path, struct fuse_file_info *fi) {
    struct stat st;
    return ggfs_getattr(path, &st);
}

static int ggfs_read(const char *path, char *buf, size_t size, off_t off,
                      struct fuse_file_info *fi) {
    (void)fi;
    pthread_mutex_lock(&game_lock);
    const char *name = path + 1;
    Scene *sc = cur_scene();

    /* .scene.jpg — serve image bytes */
    if (strcmp(name, ".scene.jpg") == 0 && sc->image[0]) {
        char ip[4096];
        img_path(ip, sizeof(ip));
        FILE *f = fopen(ip, "rb");
        if (!f) { pthread_mutex_unlock(&game_lock); return -EIO; }
        fseek(f, off, SEEK_SET);
        size_t n = fread(buf, 1, size, f);
        fclose(f);
        pthread_mutex_unlock(&game_lock);
        return (int)n;
    }

    /* .description */
    if (strcmp(name, ".description") == 0 && sc->description[0]) {
        char text[MAX_DESC + 2];
        snprintf(text, sizeof(text), "%s\n", sc->description);
        size_t len = strlen(text);
        if (off >= (off_t)len) { pthread_mutex_unlock(&game_lock); return 0; }
        size_t avail = len - off;
        if (avail > size) avail = size;
        memcpy(buf, text + off, avail);
        pthread_mutex_unlock(&game_lock);
        return (int)avail;
    }

    /* .ending */
    if (strcmp(name, ".ending") == 0 && sc->end_title[0]) {
        char text[MAX_DESC * 2];
        snprintf(text, sizeof(text), "%s\n%s\n", sc->end_title, sc->description);
        size_t len = strlen(text);
        if (off >= (off_t)len) { pthread_mutex_unlock(&game_lock); return 0; }
        size_t avail = len - off;
        if (avail > size) avail = size;
        memcpy(buf, text + off, avail);
        pthread_mutex_unlock(&game_lock);
        return (int)avail;
    }

    /* choice files — return the choice text itself */
    for (int i = 0; i < sc->num_choices; i++) {
        if (strcmp(name, sc->choices[i].filename) == 0) {
            char resp[MAX_FNAME + 2];
            snprintf(resp, sizeof(resp), "%s\n", sc->choices[i].filename);
            size_t len = strlen(resp);
            if (off >= (off_t)len) { pthread_mutex_unlock(&game_lock); return 0; }
            size_t avail = len - off;
            if (avail > size) avail = size;
            memcpy(buf, resp + off, avail);
            pthread_mutex_unlock(&game_lock);
            return (int)avail;
        }
    }

    pthread_mutex_unlock(&game_lock);
    return -ENOENT;
}

static int ggfs_write(const char *path, const char *buf, size_t size, off_t off,
                       struct fuse_file_info *fi) {
    (void)buf; (void)off; (void)fi;
    pthread_mutex_lock(&game_lock);
    const char *name = path + 1;
    Scene *sc = cur_scene();

    for (int i = 0; i < sc->num_choices; i++) {
        if (strcmp(name, sc->choices[i].filename) == 0) {
            for (int j = 0; j < game.num_scenes; j++) {
                if (strcmp(game.scenes[j].name, sc->choices[i].next) == 0) {
                    game.current = j;
                    fprintf(stderr, "[ggfs] -> %s\n", game.scenes[j].name);
                    show_scene(j);
                    break;
                }
            }
            break;
        }
    }

    pthread_mutex_unlock(&game_lock);
    return (int)size;
}

static int ggfs_truncate(const char *path, off_t size) {
    (void)path; (void)size;
    return 0;
}

static int ggfs_statfs(const char *path, struct statvfs *st) {
    (void)path;
    memset(st, 0, sizeof(*st));
    st->f_bsize = 4096;
    st->f_blocks = 1;
    st->f_files = cur_scene()->num_choices;
    return 0;
}

static int ggfs_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    (void)mode; (void)fi; (void)path;
    return -EACCES;
}

static int ggfs_mkdir(const char *path, mode_t mode) {
    (void)path; (void)mode;
    return -EACCES;
}

static int ggfs_unlink(const char *path) {
    (void)path;
    return -EACCES;
}

static int ggfs_rename(const char *from, const char *to) {
    (void)from; (void)to;
    return -EACCES;
}

static struct fuse_operations ggfs_ops = {
    .getattr  = ggfs_getattr,
    .readdir  = ggfs_readdir,
    .open     = ggfs_open,
    .read     = ggfs_read,
    .write    = ggfs_write,
    .truncate = ggfs_truncate,
    .statfs   = ggfs_statfs,
    .create   = ggfs_create,
    .mkdir    = ggfs_mkdir,
    .unlink   = ggfs_unlink,
    .rename   = ggfs_rename,
};

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <script.json> <mountpoint>\n", argv[0]);
        return 1;
    }

    char script_real[4096];
    if (!realpath(argv[1], script_real)) { perror("realpath"); return 1; }
    char *slash = strrchr(script_real, '/');
    if (slash) { *slash = '\0'; strncpy(base_dir, script_real, sizeof(base_dir) - 1); }
    else { strcpy(base_dir, "."); }

    load_script(argv[1]);

    fprintf(stderr, "[ggfs] %s — mounted at %s\n", game.title, argv[2]);

    /* display initial scene */
    show_scene(game.current);
    wait(NULL);

    signal(SIGCHLD, SIG_IGN);

    char *fargv[] = { argv[0], "-f", argv[2], NULL };
    return fuse_main(3, fargv, &ggfs_ops, NULL);
}
