#include "math3d.h"
#include <string.h>   // memcpy
#include <math.h>

// Update spherical coordinates based on cartesian
void vec3_update_spherical(vec3_t *v) {
    v->r = sqrtf(v->x*v->x + v->y*v->y + v->z*v->z);
    if (v->r == 0.0f) {
        v->theta = 0.0f;
        v->phi = 0.0f;
    } else {
        v->theta = acosf(v->z / v->r);
        v->phi = atan2f(v->y, v->x);
    }
    v->dirty_spherical = false;
    v->dirty_cartesian = false;
}

// Update cartesian coords based on spherical
void vec3_update_cartesian(vec3_t *v) {
    float st = sinf(v->theta);
    v->x = v->r * st * cosf(v->phi);
    v->y = v->r * st * sinf(v->phi);
    v->z = v->r * cosf(v->theta);
    v->dirty_cartesian = false;
    v->dirty_spherical = false;
}

// Create vec3 from spherical coords
vec3_t vec3_from_spherical(float r, float theta, float phi) {
    vec3_t v = {
        .r = r, .theta = theta, .phi = phi,
        .dirty_cartesian = true,
        .dirty_spherical = false
    };
    vec3_update_cartesian(&v);
    return v;
}

// Fast inverse sqrt (safe version)
static float inv_sqrt(float x) {
    float xhalf = 0.5f * x;
    int i;
    memcpy(&i, &x, sizeof(i));
    i = 0x5f3759df - (i >> 1);
    memcpy(&x, &i, sizeof(x));
    x = x * (1.5f - xhalf * x * x);
    return x;
}

// Normalize vec3 fast
void vec3_normalize_fast(vec3_t *v) {
    float len2 = v->x*v->x + v->y*v->y + v->z*v->z;
    if (len2 > 0.0f) {
        float inv = inv_sqrt(len2);
        v->x *= inv; v->y *= inv; v->z *= inv;
        v->dirty_spherical = true;
        vec3_update_spherical(v);
    }
}

// Spherical linear interpolation between a and b by t (0 to 1)
vec3_t vec3_slerp(const vec3_t *a, const vec3_t *b, float t) {
    float dot = a->x*b->x + a->y*b->y + a->z*b->z;
    if (dot > 1.0f) dot = 1.0f;
    if (dot < -1.0f) dot = -1.0f;

    float theta = acosf(dot) * t;
    vec3_t rel = {
        b->x - a->x*dot,
        b->y - a->y*dot,
        b->z - a->z*dot,
        0, 0, 0,
        true, true
    };
    vec3_normalize_fast(&rel);

    vec3_t res = {
        a->x*cosf(theta) + rel.x*sinf(theta),
        a->y*cosf(theta) + rel.y*sinf(theta),
        a->z*cosf(theta) + rel.z*sinf(theta),
        0, 0, 0,
        true, true
    };
    vec3_update_spherical(&res);
    return res;
}

// Mat4 identity
mat4_t mat4_identity(void) {
    mat4_t m;
    memset(m.m, 0, sizeof(m.m));
    m.m[0] = m.m[5] = m.m[10] = m.m[15] = 1.0f;
    return m;
}

// Mat4 multiply (a * b)
mat4_t mat4_multiply(const mat4_t *a, const mat4_t *b) {
    mat4_t r;
    for (int c = 0; c < 4; c++) {
        for (int r0 = 0; r0 < 4; r0++) {
            r.m[c*4 + r0] = 0;
            for (int k = 0; k < 4; k++) {
                r.m[c*4 + r0] += a->m[k*4 + r0] * b->m[c*4 + k];
            }
        }
    }
    return r;
}
  
    // Creates a 4x4 translation matrix that shifts objects by (tx, ty, tz)
mat4_t mat4_translate(float tx, float ty, float tz) {
    mat4_t m = mat4_identity();
    m.m[12] = tx; m.m[13] = ty; m.m[14] = tz;
    return m;
}

// Creates a 4x4 scaling matrix that scales objects by (sx, sy, sz)
mat4_t mat4_scale(float sx, float sy, float sz) {
    mat4_t m = mat4_identity();
    m.m[0] = sx; m.m[5] = sy; m.m[10] = sz;
    return m;
}

// Creates a 4x4 rotation matrix that applies rotations around the X, Y, and Z axes
mat4_t mat4_rotate_xyz(float rx, float ry, float rz) {
    float cx = cosf(rx), sx = sinf(rx);
    float cy = cosf(ry), sy = sinf(ry);
    float cz = cosf(rz), sz = sinf(rz);

    // Rotation matrices for each axis
    mat4_t rxm = mat4_identity();
    mat4_t rym = mat4_identity();
    mat4_t rzm = mat4_identity();

    rxm.m[5] = cx; rxm.m[6] = sx;
    rxm.m[9] = -sx; rxm.m[10] = cx;

    rym.m[0] = cy; rym.m[2] = -sy;
    rym.m[8] = sy; rym.m[10] = cy;

    rzm.m[0] = cz; rzm.m[1] = sz;
    rzm.m[4] = -sz; rzm.m[5] = cz;

    // Combine rotations: Z * Y * X
    mat4_t r = mat4_multiply(&rzm, &rym);
    r = mat4_multiply(&r, &rxm);
    return r;
}

// Creates a 4x4 frustum projection matrix

mat4_t mat4_frustum(float l, float r, float b, float t, float n, float f) {
    mat4_t m;
    memset(m.m, 0, sizeof(m.m));
    m.m[0] = 2*n / (r - l);
    m.m[5] = 2*n / (t - b);
    m.m[8] = (r + l) / (r - l);
    m.m[9] = (t + b) / (t - b);
    m.m[10] = -(f + n) / (f - n);
    m.m[11] = -1.0f;
    m.m[14] = -2*f*n / (f - n);
    return m;
}


// Multiplies a 4x4 matrix with a vec3_t (homogeneous coordinates)
vec3_t mat4_mul_vec3(const mat4_t *m, const vec3_t *v) {
    float x = v->x, y = v->y, z = v->z;
    float w = m->m[3]*x + m->m[7]*y + m->m[11]*z + m->m[15];
    if (w == 0) w = 1.0f;
    vec3_t r = {
        (m->m[0]*x + m->m[4]*y + m->m[8]*z + m->m[12]) / w,
        (m->m[1]*x + m->m[5]*y + m->m[9]*z + m->m[13]) / w,
        (m->m[2]*x + m->m[6]*y + m->m[10]*z + m->m[14]) / w,
        0, 0, 0,
        true, true
    };
    vec3_update_spherical(&r);
    return r;
}
