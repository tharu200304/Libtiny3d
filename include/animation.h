#ifndef ANIMATION_H
#define ANIMATION_H

#include "math3d.h"
#include <stdbool.h>

// Animation structure to handle time-based animations
typedef struct {
    float time;
    float duration;
    bool loop;
} animation_t;

void animation_init(animation_t *a, float duration, bool loop);
float animation_update(animation_t *a, float dt);

// Cubic Bézier interpolation
vec3_t bezier3(const vec3_t *p0, const vec3_t *p1,
               const vec3_t *p2, const vec3_t *p3, float t);

#endif
