#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

const char* double_full_block = "\u2588\u2588";

typedef struct SandGrid {
    unsigned int width;
    unsigned int height;
    char *string;
    bool **data;
} SandGrid;

void sand_grid_string_update(SandGrid *sand_grid);

SandGrid *sand_grid_create(unsigned int width, unsigned int height, bool default_value) {
    if (width == 0 || height == 0) return NULL;

    SandGrid *sand_grid = malloc(sizeof(SandGrid));
    if (sand_grid == NULL) return NULL;

    sand_grid->width = width;
    sand_grid->height = height;

    sand_grid->string = malloc((width * strlen(double_full_block) * height + height) * sizeof(char));
    if (sand_grid->string == NULL) {
        free(sand_grid);
        return NULL;
    }
    
    sand_grid->data = malloc(height * sizeof(bool*));
    if (sand_grid->data == NULL) {
        free(sand_grid);
        return NULL;
    }

    for (unsigned int y = 0; y < height; y++) {
        sand_grid->data[y] = malloc(width * sizeof(bool));
        if (sand_grid->data[y] == NULL) {
            for (unsigned int i = 0; i < y; i++) {
                free(sand_grid->data[i]);
            }

            free(sand_grid->data);
            free(sand_grid->string);
            free(sand_grid);
            return NULL;
        }

        for (unsigned int x = 0; x < width; x++) {
            sand_grid->data[y][x] = default_value;
        }
    }

    sand_grid_string_update(sand_grid);

    return sand_grid;
}

void sand_grid_destroy(SandGrid *sand_grid) {
    for (unsigned int i = 0; i < sand_grid->height; i++) {
        free(sand_grid->data[i]);
    }

    free(sand_grid->data);
    free(sand_grid->string);
    free(sand_grid);
}

void sand_grid_string_update(SandGrid *sand_grid) {
    unsigned int i = 0;
    for (unsigned int y = 0; y < sand_grid->height; y++) {
        for (unsigned int x = 0; x < sand_grid->width; x++) {
            if (sand_grid->data[y][x]) {
                memcpy(&sand_grid->string[i], double_full_block, strlen(double_full_block));
                i += strlen(double_full_block);
            } else {
                memcpy(&sand_grid->string[i], "  ", 2 * sizeof(char));
                i += 2;
            }

            if (x == sand_grid->width - 1) {
                if (y == sand_grid->height - 1) {
                    sand_grid->string[i++] = '\0';
                } else {
                    sand_grid->string[i++] = '\n';
                }
            }
        }
    }
}

void sand_grid_data_update(SandGrid *sand_grid) {
    for (unsigned int y = 0; y < sand_grid->height; y++) {
        for (unsigned int x = 0; x < sand_grid->width; x++) {
            if (!sand_grid->data[y][x]) continue;
        }
    }
}

void disable_input() {
    struct termios term;
    if (tcgetattr(STDIN_FILENO, &term) == -1) exit(1);
    term.c_lflag &= ~(ICANON | ECHO);
    if (tcsetattr(STDIN_FILENO, TCSANOW, &term) == -1) exit(1);
}

void enable_input() {
    struct termios term;
    if (tcgetattr(STDIN_FILENO, &term) == -1) exit(1);
    term.c_lflag |= (ICANON | ECHO);
    if (tcsetattr(STDIN_FILENO, TCSANOW, &term) == -1) exit(1);
}

void clear() {
    const char *clear = "\033[2J\033[H";
    write(STDOUT_FILENO, clear, strlen(clear));
}

bool should_exit = false;

void handle_exit() {
    should_exit = true;
}

int main() {
    signal(SIGINT, handle_exit);
    signal(SIGTERM, handle_exit);
    if (atexit(handle_exit) != 0) return 1;

    struct winsize w;
    if (ioctl(0, TIOCGWINSZ, &w) == -1) return 1;

    SandGrid *sand_grid = sand_grid_create(w.ws_col / 2, w.ws_row, true);
    
    disable_input();

    while (!should_exit) {
        sand_grid_data_update(sand_grid);
        sand_grid_string_update(sand_grid);

        clear();
        write(STDOUT_FILENO, sand_grid->string, strlen(sand_grid->string));

        struct timespec ts = { 1, 0 };
        nanosleep(&ts, NULL);
    }

    clear();
    sand_grid_destroy(sand_grid);

    enable_input();

    return 0;
}
