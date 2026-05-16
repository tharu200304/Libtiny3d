#ifndef LIGHTING_H
#define LIGHTING_H

#include "math3d.h"

// Calculate Lambert lighting intensity given normal and light direction
float lambert_intensity(const vec3_t *normal, const vec3_t *light_dir);

// Normalize a vec3_t in place
void vec3_normalize(vec3_t *v);
vec3_t vec3_normalized(const vec3_t *v);

#endif
