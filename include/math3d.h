#ifndef MATH3D_H
#define MATH3D_H

//Use of boolean types
#include <stdbool.h>

typedef struct {
    float x, y, z;        // Cartesian coordinates
    float r;              // radius 
    float theta;          // polar angle (0 to pi)
    float phi;            // azimuthal angle (0 to 2pi)
    bool dirty_cartesian;
    bool dirty_spherical;
} vec3_t;

typedef struct {
    float m[16];          // 4x4 matrix column-major order
} mat4_t;

// vec3 functions
vec3_t vec3_from_spherical(float r, float theta, float phi);
void vec3_update_spherical(vec3_t *v);
void vec3_update_cartesian(vec3_t *v);
void vec3_normalize_fast(vec3_t *v);
vec3_t vec3_slerp(const vec3_t *a, const vec3_t *b, float t);

// mat4 functions
mat4_t mat4_identity(void);
mat4_t mat4_multiply(const mat4_t *a, const mat4_t *b);
mat4_t mat4_translate(float tx, float ty, float tz);
mat4_t mat4_scale(float sx, float sy, float sz);
mat4_t mat4_rotate_xyz(float rx, float ry, float rz);
mat4_t mat4_frustum(float l, float r, float b, float t, float n, float f);
vec3_t mat4_mul_vec3(const mat4_t *m, const vec3_t *v);

#endif // MATH3D_H
