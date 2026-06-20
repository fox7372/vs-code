#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define LED_PATH "/sys/class/leds/PWR"

static volatile sig_atomic_t running = 1;

static void sigint_handler(int sig) {
    (void)sig;
    running = 0;
}

int main(int argc, char **argv) {
    int interval_ms = 200;
    if (argc > 1)
        interval_ms = atoi(argv[1]);

    int brightness_fd, trigger_fd;
    char path[256];

    /* Set trigger to "none" to take manual control */
    snprintf(path, sizeof(path), "%s/trigger", LED_PATH);
    trigger_fd = open(path, O_WRONLY);
    if (trigger_fd < 0) {
        perror("open trigger");
        return 1;
    }
    if (write(trigger_fd, "none", 4) < 0) {
        perror("write trigger");
        close(trigger_fd);
        return 1;
    }
    close(trigger_fd);

    /* Open brightness for manual blinking */
    snprintf(path, sizeof(path), "%s/brightness", LED_PATH);
    brightness_fd = open(path, O_WRONLY);
    if (brightness_fd < 0) {
        perror("open brightness");
        return 1;
    }

    signal(SIGINT, sigint_handler);
    signal(SIGTERM, sigint_handler);

    printf("Blinking PWR LED every %d ms. Press Ctrl-C to stop.\n", interval_ms);

    int state = 1;
    while (running) {
        char val = state ? '1' : '0';
        if (write(brightness_fd, &val, 1) < 0) {
            perror("write brightness");
            break;
        }
        state = !state;
        usleep(interval_ms * 1000);
    }

    /* Restore LED to on before exit */
    char on = '1';
    write(brightness_fd, &on, 1);
    close(brightness_fd);

    printf("Done.\n");
    return 0;
}
