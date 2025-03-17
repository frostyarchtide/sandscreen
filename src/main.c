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
    bool *data;
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
    
    sand_grid->data = malloc(width * height * sizeof(bool));
    if (sand_grid->data == NULL) {
        free(sand_grid);
        return NULL;
    }

    for (unsigned int i = 0; i < width * height; i++) {
        sand_grid->data[i] = default_value;
    }

    sand_grid_string_update(sand_grid);

    return sand_grid;
}

void sand_grid_destroy(SandGrid *sand_grid) {
    free(sand_grid->data);
    free(sand_grid->string);
    free(sand_grid);
}

void sand_grid_cell_set(SandGrid *sand_grid, unsigned int x, unsigned int y, bool value) {
    sand_grid->data[y * sand_grid->width + x] = value;
}

bool sand_grid_cell_get(SandGrid *sand_grid, unsigned int x, unsigned int y) {
    return sand_grid->data[y * sand_grid->width + x];
}

void sand_grid_string_update(SandGrid *sand_grid) {
    unsigned int i = 0;
    for (unsigned int y = 0; y < sand_grid->height; y++) {
        for (unsigned int x = 0; x < sand_grid->width; x++) {
            if (sand_grid_cell_get(sand_grid, x, y)) {
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

bool sand_grid_data_update(SandGrid *sand_grid) {
    bool *data = malloc(sand_grid->width * sand_grid->height * sizeof(bool));
    if (data == NULL) return false;

    memcpy(data, sand_grid->data, sand_grid->width * sand_grid->height * sizeof(bool));

    bool updated = false;
    for (unsigned int y = 0; y < sand_grid->height; y++) {
        for (unsigned int x = 0; x < sand_grid->width; x++) {
            if (!sand_grid_cell_get(sand_grid, x, y) || y == sand_grid->height - 1) continue;

            if (!data[(y + 1) * sand_grid->width + x]) {
                data[y * sand_grid->width + x] = false;
                data[(y + 1) * sand_grid->width + x] = true;
                updated = true;
                continue;
            }
            
            if (x > 0 && !data[(y + 1) * sand_grid->width + x - 1]) {
                data[y * sand_grid->width + x] = false;
                data[(y + 1) * sand_grid->width + (x - 1)] = true;
                updated = true;
                continue;
            }

            if (x < sand_grid->width - 1 && !data[(y + 1) * sand_grid->width + x + 1]) {
                data[y * sand_grid->width + x] = false;
                data[(y + 1) * sand_grid->width + x + 1] = true;
                updated = true;
                continue;
            }
        }
    }

    memcpy(sand_grid->data, data, sand_grid->width * sand_grid->height * sizeof(bool));

    free(data);

    return updated;
}

bool sand_grid_clear_falling(SandGrid *sand_grid) {
    bool *data = malloc(sand_grid->width * sand_grid->height * sizeof(bool));
    if (data == NULL) return false;

    for (unsigned int i = 0; i < sand_grid->width * sand_grid->height; i++) {
        data[i] = false;
    }

    bool updated = false;
    for (unsigned int y = 0; y < sand_grid->height - 1; y++) {
        for (unsigned int x = 0; x < sand_grid->width; x++) {
            if (sand_grid_cell_get(sand_grid, x, y)) {
                data[(y + 1) * sand_grid->width + x] = true;
                updated = true;
            }
        }
    }

    memcpy(sand_grid->data, data, sand_grid->width * sand_grid->height * sizeof(bool));

    return updated;
}

void clear() {
    const char *clear = "\033[2J\033[H";
    write(STDOUT_FILENO, clear, strlen(clear));
}

bool should_exit = false;

void handle_signals() {
    should_exit = true;
}

int main() {
    signal(SIGINT, handle_signals);
    signal(SIGTERM, handle_signals);

    struct winsize w;
    if (ioctl(0, TIOCGWINSZ, &w) == -1) return 1;

    SandGrid *sand_grid = sand_grid_create(w.ws_col / 2, w.ws_row, false);
    sand_grid->data[0] = true;

    unsigned int spawn_frames = w.ws_row;
    bool clearing = false;
    while (!should_exit) {
        if (clearing) {
            if (!sand_grid_clear_falling(sand_grid)) {
                clearing = false;
                spawn_frames = w.ws_row;
            }
        } else if (!sand_grid_data_update(sand_grid)) {
            clearing = true;
        }

        if (spawn_frames > 0) {
            for (unsigned int x = 0; x < sand_grid->width; x++) {
                bool value = (rand() % 2 == 0) ? true : false;
                sand_grid_cell_set(sand_grid, x, 0, value);
            }

            spawn_frames--;
        }

        sand_grid_string_update(sand_grid);

        clear();
        write(STDOUT_FILENO, sand_grid->string, strlen(sand_grid->string));

        struct timespec ts = { 0, 1000 / 30 * 1000000L };
        nanosleep(&ts, NULL);
    }

    sand_grid_destroy(sand_grid);

    clear();

    return 0;
}
