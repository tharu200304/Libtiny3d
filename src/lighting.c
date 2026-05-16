#include "lighting.h"
#include <math.h>

float lambert_intensity(const vec3_t *normal, const vec3_t *light_dir)
{
    // Assume both vectors are normalized
    float dot = normal->x * light_dir->x +
                normal->y * light_dir->y +
                normal->z * light_dir->z;
    return fmaxf(dot, 0.0f);
}

    
void vec3_normalize(vec3_t *v)
{
    float len = sqrtf(v->x * v->x + v->y * v->y + v->z * v->z);
    if (len > 1e-6f) {
        v->x /= len;
        v->y /= len;
        v->z /= len;
    }
    // Optionally, update other fields if needed
}
vec3_t vec3_normalized(const vec3_t *v)
{
    vec3_t copy = *v;
    vec3_normalize(&copy);
    return copy;
}

