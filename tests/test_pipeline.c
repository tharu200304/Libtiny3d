#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "canvas.h"
#include "math3d.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#define WIDTH 256
#define HEIGHT 256
#define DEG2RAD(x) ((x) * M_PI / 180.0f)
#define FRAME_COUNT 180  // 3 seconds animation at 60 fps

// Cube edges
const int edges[12][2] = {
    {0,1},{1,2},{2,3},{3,0},
    {4,5},{5,6},{6,7},{7,4},
    {0,4},{1,5},{2,6},{3,7}
};

// Draw cube edges
void draw_wireframe(canvas_t *canvas, const vec3_t *points) {
    for (int i = 0; i < 12; i++) {
        int a = edges[i][0];
        int b = edges[i][1];
        draw_line_f(canvas,
            points[a].x, points[a].y,
            points[b].x, points[b].y,
            1.5f);
    }
}

// Save canvas to PGM file
int main() {

    
    vec3_t cube[8] = {
        {-0.5f,-0.5f,-0.5f,0,0,0,true,true}, {0.5f,-0.5f,-0.5f,0,0,0,true,true},
        {0.5f, 0.5f,-0.5f,0,0,0,true,true}, {-0.5f, 0.5f,-0.5f,0,0,0,true,true},
        {-0.5f,-0.5f, 0.5f,0,0,0,true,true}, {0.5f,-0.5f, 0.5f,0,0,0,true,true},
        {0.5f, 0.5f, 0.5f,0,0,0,true,true}, {-0.5f, 0.5f, 0.5f,0,0,0,true,true}
    };

    // Perspective projection matrix
    float near = 1.0f, far = 100.0f;
    float fov = DEG2RAD(60.0f);
    float aspect = (float)WIDTH / (float)HEIGHT;
    float top = near * tanf(fov / 2);
    float right = top * aspect;
    mat4_t proj = mat4_frustum(-right, right, -top, top, near, far);

    // Create output directory
    for (int frame = 0; frame < FRAME_COUNT; frame++) {
        char filename[64];
        snprintf(filename, sizeof(filename), "Frames/cube/framecube_%03d.pgm", frame);

        // Create canvas
        canvas_t *canvas = create_canvas(WIDTH, HEIGHT);
        canvas_clear(canvas, 0.0f);

        float t = (float)frame / FRAME_COUNT;  // normalized time 0 to 1
        float angle = t * 2.0f * M_PI;

        // Move on X axis between -2 and +2 smoothly
        float x_move = 2.0f * sinf(t * 2.0f * M_PI);
        // Smaller oscillation on Y axis
        float y_move = 0.7f * cosf(t * 4.0f * M_PI);
        // Z oscillation moving cube forward/back a bit
        float z_move = -5.0f + 1.5f * sinf(t * 2.0f * M_PI);

        // Rotation matrix
        mat4_t rot = mat4_rotate_xyz(angle, angle * 0.7f, angle * 0.5f);

        // Scale matrix
        mat4_t scale = mat4_scale(1.2f, 1.2f, 1.2f);

        // Translation matrix with animated x, y, z
        mat4_t translate = mat4_translate(x_move, y_move, z_move);

        // Model matrix = translate * rotate * scale
        mat4_t model = mat4_multiply(&translate, &rot);
        model = mat4_multiply(&model, &scale);

        // MVP matrix = projection * model
        mat4_t mvp = mat4_multiply(&proj, &model);

        // Project cube vertices
        vec3_t projected[8];
        for (int i = 0; i < 8; i++) {
            vec3_t v = mat4_mul_vec3(&mvp, &cube[i]);
            // Convert NDC (-1..1) to screen coordinates
            v.x = (v.x + 1.0f) * 0.5f * WIDTH;
            v.y = (1.0f - (v.y + 1.0f) * 0.5f) * HEIGHT;
            projected[i] = v;
        }

        // Draw the wireframe cube
        draw_wireframe(canvas, projected);
        save_canvas_pgm(canvas, filename);
        free_canvas(canvas);
        printf("Saved %s\n", filename);
    }

    printf("Animation frames generated.\n");
    return 0;
}
