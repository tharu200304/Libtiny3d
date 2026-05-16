#include "canvas.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

// Allocate a width×height canvas and zero-initialize pixels
canvas_t *create_canvas(int width, int height) {
    if (width <= 0 || height <= 0) return NULL;

    // Allocate canvas structure
    canvas_t *canvas = malloc(sizeof(canvas_t));
    if (!canvas) return NULL;

    canvas->width = width;
    canvas->height = height;

    // Allocate rows pointers
    canvas->pixels = malloc(height * sizeof(float *));
    if (!canvas->pixels) {
        free(canvas);
        return NULL;
    }

    // Allocate each row and zero initialize
    for (int y = 0; y < height; y++) {
        canvas->pixels[y] = calloc(width, sizeof(float));
        if (!canvas->pixels[y]) {
            // free previously allocated rows
            for (int i = 0; i < y; i++)
                free(canvas->pixels[i]);
            free(canvas->pixels);
            free(canvas);
            return NULL;
        }
    }

    return canvas;
}

// Free all memory belonging to a canvas
void free_canvas(canvas_t *canvas) {
    if (!canvas) return;

    for (int y = 0; y < canvas->height; y++)
        free(canvas->pixels[y]);

    free(canvas->pixels);
    free(canvas);
}

// Clamp float to [0.0f, 1.0f]
static float clamp(float v) {
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

// Set pixel intensity using bilinear filtering for fractional (x,y)
void set_pixel_f(canvas_t *canvas, float x, float y, float intensity) {
    if (!canvas) return;

    intensity = clamp(intensity);

    int x0 = (int)floorf(x);
    int y0 = (int)floorf(y);
    int x1 = x0 + 1;
    int y1 = y0 + 1;

    float wx = x - x0;
    float wy = y - y0;

    // Distribute intensity to 4 surrounding pixels weighted by area
    if (x0 >= 0 && x0 < canvas->width && y0 >= 0 && y0 < canvas->height) {
        float val = intensity * (1 - wx) * (1 - wy);
        if (val > canvas->pixels[y0][x0]) canvas->pixels[y0][x0] = val;
    }
    if (x1 >= 0 && x1 < canvas->width && y0 >= 0 && y0 < canvas->height) {
        float val = intensity * wx * (1 - wy);
        if (val > canvas->pixels[y0][x1]) canvas->pixels[y0][x1] = val;
    }
    if (x0 >= 0 && x0 < canvas->width && y1 >= 0 && y1 < canvas->height) {
        float val = intensity * (1 - wx) * wy;
        if (val > canvas->pixels[y1][x0]) canvas->pixels[y1][x0] = val;
    }
    if (x1 >= 0 && x1 < canvas->width && y1 >= 0 && y1 < canvas->height) {
        float val = intensity * wx * wy;
        if (val > canvas->pixels[y1][x1]) canvas->pixels[y1][x1] = val;
    }
}

// Draw anti-aliased line using floating-point DDA algorithm and set_pixel_f
void draw_line_f(canvas_t *canvas,
                 float x0, float y0,
                 float x1, float y1,
                 float thickness) {
    if (!canvas) return;

    float dx = x1 - x0;
    float dy = y1 - y0;
    float length = sqrtf(dx*dx + dy*dy);
    if (length == 0) return;

    float nx = dx / length; // unit vector x
    float ny = dy / length; // unit vector y

    // Calculate number of steps based on length, aim for smooth coverage
    int steps = (int)(length * 2); // 2 samples per pixel length approx

    // Perpendicular vector for thickness direction
    float px = -ny;
    float py = nx;

    for (int i = 0; i <= steps; i++) {
        float t = (float)i / steps;
        float cx = x0 + t * dx;
        float cy = y0 + t * dy;

        // Draw line thickness as multiple samples perpendicular to line center
        int samples = (int)(thickness * 2);
        if (samples < 1) samples = 1;

        for (int s = -samples; s <= samples; s++) {
            float offset = s * 0.5f;  // subpixel offset
            float px_off = cx + px * offset;
            float py_off = cy + py * offset;

            // Calculate intensity falloff with distance from center line
            float dist = fabsf(offset);
            float intensity = 1.0f - (dist / (samples * 0.5f));
            if (intensity < 0) intensity = 0;

            set_pixel_f(canvas, px_off, py_off, intensity);
        }
    }
}

// Save the canvas as an ASCII PGM file (portable graymap)
int save_canvas_pgm(const canvas_t *canvas, const char *filename) {
    if (!canvas || !filename) return -1;

    FILE *f = fopen(filename, "w");
    if (!f) return -1;

    // Write PGM header
    fprintf(f, "P2\n%d %d\n255\n", canvas->width, canvas->height);

    for (int y = 0; y < canvas->height; y++) {
        for (int x = 0; x < canvas->width; x++) {
            int val = (int)(clamp(canvas->pixels[y][x]) * 255.0f + 0.5f);
            fprintf(f, "%d ", val);
        }
        fprintf(f, "\n");
    }

    fclose(f);
    return 0;
}

// Clear every pixel to the given brightness (0.0 to 1.0)
void canvas_clear(canvas_t *canvas, float value) {
    if (!canvas || !canvas->pixels) return;

    // Clamp value
    if (value < 0.0f) value = 0.0f;
    else if (value > 1.0f) value = 1.0f;

    for (int y = 0; y < canvas->height; ++y)
        for (int x = 0; x < canvas->width; ++x)
            canvas->pixels[y][x] = value;
}
