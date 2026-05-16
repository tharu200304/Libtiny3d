#include "animation.h"

// Initialize an animation with a given duration and whether it should loop
void animation_init(animation_t *a, float duration, bool loop) {
    a->time = 0.0f;   // Start at time 0
    a->duration = duration;    // Total duration of the animation

    a->loop = loop;   // Whether the animation should repeat
}


// Update the animation by advancing the time by 'dt'
float animation_update(animation_t *a, float dt) {
    a->time += dt;
    if (a->loop) {
     // If looping is enabled, wrap time back to 0 after duration    
        while (a->time >= a->duration)
            a->time -= a->duration;
    } else if (a->time > a->duration) {
    
    // If not looping, clamp time to duration
        a->time = a->duration;
    }

    // Return the normalized time (0.0 to 1.0)
    return a->time / a->duration;
}


// Cubic Bézier curve interpolation
vec3_t bezier3(const vec3_t *p0, const vec3_t *p1,
               const vec3_t *p2, const vec3_t *p3, float t)
{
    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

     // Apply cubic Bezier formula to get the interpolated position
    vec3_t result = {
        .x = uuu * p0->x + 3 * uu * t * p1->x + 3 * u * tt * p2->x + ttt * p3->x,
        .y = uuu * p0->y + 3 * uu * t * p1->y + 3 * u * tt * p2->y + ttt * p3->y,
        .z = uuu * p0->z + 3 * uu * t * p1->z + 3 * u * tt * p2->z + ttt * p3->z,
        .r = 0, .theta = 0, .phi = 0,
        .dirty_cartesian = true, .dirty_spherical = true
    };
    
    // Update spherical coordinates
    vec3_update_spherical(&result);
    return result;
}
