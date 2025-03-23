#include <locale.h>
#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

typedef struct SandGrid {
    unsigned int width;
    unsigned int height;
    bool *data;
} SandGrid;

SandGrid *sand_grid_create(unsigned int width, unsigned int height, bool default_value) {
    if (width == 0 || height == 0) return NULL;

    SandGrid *sand_grid = malloc(sizeof(SandGrid));
    if (sand_grid == NULL) return NULL;

    sand_grid->width = width;
    sand_grid->height = height;

    sand_grid->data = malloc(width * height * sizeof(bool));
    if (sand_grid->data == NULL) {
        free(sand_grid);
        return NULL;
    }

    for (unsigned int i = 0; i < width * height; i++) {
        sand_grid->data[i] = default_value;
    }

    return sand_grid;
}

void sand_grid_destroy(SandGrid *sand_grid) {
    free(sand_grid->data);
    free(sand_grid);
}

bool sand_grid_update(SandGrid *sand_grid) {
    bool *data = malloc(sand_grid->width * sand_grid->height * sizeof(bool));
    if (data == NULL) return false;

    memcpy(data, sand_grid->data, sand_grid->width * sand_grid->height * sizeof(bool));

    bool updated = false;
    for (unsigned int y = 0; y < sand_grid->height; y++) {
        for (unsigned int x = 0; x < sand_grid->width; x++) {
            if (y == sand_grid->height - 1 || !sand_grid->data[y * sand_grid->width + x]) continue;

            if (!data[(y + 1) * sand_grid->width + x]) {
                data[y * sand_grid->width + x] = false;
                data[(y + 1) * sand_grid->width + x] = true;
                updated = true;
                continue;
            }

            if (x > 0 && !data[(y + 1) * sand_grid->width + x - 1]) {
                data[y * sand_grid->width + x] = false;
                data[(y + 1) * sand_grid->width + x - 1] = true;
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
            bool value = sand_grid->data[y * sand_grid->width + x];
            data[(y + 1) * sand_grid->width + x] = value;
            if (value) {
                updated = true;
            }
        }
    }

    memcpy(sand_grid->data, data, sand_grid->width * sand_grid->height * sizeof(bool));

    free(data);

    return updated;
}

void sand_grid_display(SandGrid *sand_grid, bool extend_edge) {
    for (unsigned int y = 0; y < sand_grid->height; y++) {
        for (unsigned int x = 0; x < sand_grid->width; x++) {
            bool value = sand_grid->data[y * sand_grid->width + x];
            mvaddstr(y, x * 2, (value) ? "\u2588\u2588" : "  ");
            if (extend_edge) {
                mvaddstr(y, x * 2 + 2, (value) ? "\u2588" : " ");
            }
        }
    }
}

int main(int argc, char *argv[]) {
    int spawn_frames_arg = -1;
    int wait_frames_arg = -1;
    int opt;

    while ((opt = getopt(argc, argv, "s:w:?")) != -1) {
        switch (opt) {
            case '?':
                return 1;
            case 's':
                spawn_frames_arg = atoi(optarg);
                break;
            case 'w':
                wait_frames_arg = atoi(optarg);
                break;
        }
    }

    srand(time(NULL) ^ getpid());

    setlocale(LC_ALL, "");

    initscr();
    noecho();
    timeout(25);
    curs_set(0);

    int width;
    int height;
    getmaxyx(stdscr, height, width);
    bool extend_edge = width % 2 == 1;
    width /= 2;

    if (spawn_frames_arg == -1) spawn_frames_arg = height;
    if (wait_frames_arg == -1) wait_frames_arg = height / 4;

    SandGrid *sand_grid = sand_grid_create(width, height, false);

    bool clearing = false;
    printf("%d\n", spawn_frames_arg);
    int spawn_frames = spawn_frames_arg;
    int wait_frames = 0;
    while (true) {
        char character = getch();
        if (character != ERR && character == 'q') {
            break;
        }

        if (wait_frames > 0) {
            wait_frames--;

            continue;
        }
        
        if (spawn_frames > 0) {
            for (unsigned int x = 0; x < width; x++) {
                sand_grid->data[x] = rand() % 4 == 0;
            }

            spawn_frames--;
        }

        sand_grid_display(sand_grid, extend_edge);
        refresh();

        if (clearing) {
            if (!sand_grid_clear_falling(sand_grid)) {
                clearing = false;
                spawn_frames = spawn_frames_arg;
            }
        } else if (!sand_grid_update(sand_grid)) {
            clearing = true;
            wait_frames = wait_frames_arg;
        }
    }

    endwin();

    sand_grid_destroy(sand_grid);

    return 0;
}
