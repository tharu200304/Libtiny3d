#ifndef RENDERER_H
#define RENDERER_H

#include "canvas.h"
#include "math3d.h"

#ifdef __cplusplus
extern "C" {
#endif

// Projects a 3D vertex using MVP matrix and viewport size
vec3_t project_vertex(const mat4_t *mvp, const vec3_t *v, int w, int h);

// Clip test: is point inside circular viewport?
int clip_to_viewport(const canvas_t *c, int x, int y);

// Render wireframe without lighting
void render_wireframe(canvas_t *canvas,
                      const vec3_t *verts, int n_verts,
                      const int (*edges)[2], int n_edges,
                      const mat4_t *mvp);

// Compute vertex normals as average of edge perpendiculars 
void compute_vertex_normals(const vec3_t *verts, int n_verts,
                            const int (*edges)[2], int n_edges,
                            vec3_t *out_normals);

// Render wireframe with Lambertian lighting on edges
void render_wireframe_lit(canvas_t *canvas,
                          const vec3_t *verts, int n_verts,
                          const int (*edges)[2], int n_edges,
                          const mat4_t *mvp,
                          const vec3_t *normals,
                          const vec3_t *light_dir);

#ifdef __cplusplus
}
#endif

#endif
