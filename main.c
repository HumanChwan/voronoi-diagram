#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define HEIGHT 1080
#define WIDTH 1920
#define OUTPUT_FILE "output.ppm"

#define COLOR_WHITE 0xFFFFFFFF
#define COLOR_BLACK 0xFF000000
#define COLOR_RED 0xFF0000FF

#define SEED_COUNT 30
#define SEED_RADIUS 5

typedef uint32_t Color32;

typedef struct Coordinate {
    uint16_t x, y;
} Coordinate;

static Color32 image[HEIGHT][WIDTH];
static Coordinate seeds[SEED_COUNT];

void save_to_ppm(char* filename) {
    FILE* f = fopen(filename, "wb");
    fprintf(f, "P6\n%d %d 255\n", WIDTH, HEIGHT);

    for (size_t y = 0; y < HEIGHT; ++y) {
        for (size_t x = 0; x < WIDTH; ++x) {
            uint8_t bytes[3] = {
                (uint8_t)((image[y][x] & 0x0000FF) >> 0),
                (uint8_t)((image[y][x] & 0x00FF00) >> 8),
                (uint8_t)((image[y][x] & 0xFF0000) >> 16),
            };
            fwrite(bytes, sizeof(bytes), 1, f);
        }
    }

    fclose(f);
}

void generate_seeds(void) {
    srand(time(0));
    for (size_t i = 0; i < SEED_COUNT; ++i) {
        seeds[i].y = rand() % HEIGHT;
        seeds[i].x = rand() % WIDTH;
    }
}

Color32 coordinate_to_color32(Coordinate* coordinate) {
    uint16_t X = coordinate->x, Y = coordinate->y;
    X = ~X;
    return (((Color32)Y << 16u) | (Color32)X);
}

int sqr_dist_from_coordinate(uint16_t y, uint16_t x, Coordinate* coordinate) {
    int dy = y - coordinate->y;
    int dx = x - coordinate->x;
    return (dx * dx + dy * dy);
}

void render_voronoi(void) {
    for (size_t y = 0; y < HEIGHT; ++y) {
        for (size_t x = 0; x < WIDTH; ++x) {
            int j = 0;
            for (size_t i = 1; i < SEED_COUNT; ++i) {
                if (sqr_dist_from_coordinate(y, x, seeds + i) <
                    sqr_dist_from_coordinate(y, x, seeds + j))
                    j = i;
            }
            image[y][x] = coordinate_to_color32(seeds + j);
        }
    }
}

void render_circle(Coordinate* center, int16_t radius, Color32 color) {
    int y_neg = center->y - radius, y_pos = center->y + radius;
    int x_neg = center->x - radius, x_pos = center->x + radius;

    for (int y = y_neg; y <= y_pos; ++y) {
        if (y < 0 || HEIGHT <= y) continue;
        for (int x = x_neg; x <= x_pos; ++x) {
            if (x < 0 || WIDTH <= x) continue;
            int dx = center->x - x, dy = center->y - y;
            if (dx * dx + dy * dy <= radius * radius) {
                image[y][x] = color;
            }
        }
    }
}

void render_seeds(void) {
    for (size_t i = 0; i < SEED_COUNT; ++i)
        render_circle(seeds + i, SEED_RADIUS, COLOR_BLACK);
}

int main(void) {
    generate_seeds();
    render_voronoi();
    render_seeds();
    save_to_ppm(OUTPUT_FILE);
    return 0;
}
