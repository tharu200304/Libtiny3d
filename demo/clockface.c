#include <stdio.h>
#include <math.h>
#include "canvas.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// Canvas dimensions and radius for clock face
#define W        600
#define H        600
#define RADIUS   200.0f

int main(void)
{
    // Create a canvas with specified width and height
    canvas_t* cv = create_canvas(W, H);

    // Clear the canvas with black (0.0f brightness)
    canvas_clear(cv, 0.0f);

    // Set the center of the canvas (clock center)
    float cx = W * 0.5f, cy = H * 0.5f;

    // Draw 24 radial lines from the center like clock hour markers
    for (int i = 0; i < 24; ++i) {
        float angle = i * (M_PI / 12.0f); // 15-degree steps
        float x = cx + cosf(angle) * RADIUS;
        float y = cy + sinf(angle) * RADIUS;
        draw_line_f(cv, cx, cy, x, y, 2.0f);
    }

    // Save the canvas to a PGM image file
    save_canvas_pgm(cv, "clockface.pgm");
    printf("Saved clockface.pgm\n");

    // Free the canvas memory
    free_canvas(cv);
    return 0;
}
