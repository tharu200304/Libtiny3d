#include <stdio.h>
#include <math.h>
#include "math3d.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif
#define DEG2RAD(x) ((x) * M_PI / 180.0f)

int main() {
    // Define 8 vertices of a cube centered at origin, size 2
    vec3_t cube[8] = {
        { -1, -1, -1, 0,0,0, false,false },
        {  1, -1, -1, 0,0,0, false,false },
        {  1,  1, -1, 0,0,0, false,false },
        { -1,  1, -1, 0,0,0, false,false },
        { -1, -1,  1, 0,0,0, false,false },
        {  1, -1,  1, 0,0,0, false,false },
        {  1,  1,  1, 0,0,0, false,false },
        { -1,  1,  1, 0,0,0, false,false }
    };

    // Manually apply transforms:
    // Scale by 1.5
    mat4_t scale = mat4_scale(1.5f, 1.5f, 1.5f);

    // Rotate 30 degrees around each axis
    float angle = DEG2RAD(30);
    mat4_t rotation = mat4_rotate_xyz(angle, angle, angle);

    // Translate by (0, 0, -5)
    mat4_t translation = mat4_translate(0, 0, -5);

    // Combine transformations: M = T * R * S
    mat4_t model = mat4_multiply(&translation, &rotation);
    model = mat4_multiply(&model, &scale);

    // Perspective frustum params
    float near = 1.0f;
    float far = 100.0f;
    float fov = DEG2RAD(60);
    float aspect = 1.0f; // square viewport
    float top = near * tanf(fov / 2);
    float bottom = -top;
    float right = top * aspect;
    float left = -right;

    mat4_t projection = mat4_frustum(left, right, bottom, top, near, far);

    // Project all points
    printf("Projected 2D points (x, y):\n");
    for (int i = 0; i < 8; i++) {
        // Model transform
        vec3_t transformed = mat4_mul_vec3(&model, &cube[i]);

        // Projection transform
        vec3_t projected = mat4_mul_vec3(&projection, &transformed);

        // Normalize device coordinates to screen coords (assuming viewport -1..1)
        // For simplicity, print NDC coords here (x,y in -1..1)
        printf("Vertex %d: (%.3f, %.3f)\n", i, projected.x, projected.y);
    }

    return 0;
}
