#ifndef CANVAS_H
#define CANVAS_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif


/* canvas_t – floating-point grayscale image buffer. */
typedef struct {
    int   width;
    int   height;
    float **pixels;   /* pixels[y][x] */
} canvas_t;

/* width×height canvas and zero-initialize its pixels. */
canvas_t *create_canvas(int width, int height);

/** Free all memory belonging to a canvas created with create_canvas(). */
void free_canvas(canvas_t *canvas);


 /* Write intensity into (x,y) using bilinear filtering.(x,y) may be fractional. Intensity is clamped to [0,1].*/

void set_pixel_f(canvas_t *canvas, float x, float y, float intensity);

/**Draw an anti-aliased line of given thickness between (x0,y0) and (x1,y1).*/

void draw_line_f(canvas_t *canvas,
                 float   x0, float y0,
                 float   x1, float y1,
                 float   thickness);

/** Save the canvas as an ASCII PGM file (portable graymap). */
int save_canvas_pgm(const canvas_t *canvas, const char *filename);

/** Clear every pixel to the given brightness (0.0 to 1.0). */
void canvas_clear(canvas_t *canvas, float value);

#ifdef __cplusplus
}
#endif

#endif /* CANVAS_H */
