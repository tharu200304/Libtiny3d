#include "renderer.h"
#include "lighting.h" //  Required for lambert_intensity()
#include <stdlib.h>
#include <math.h>

// Helper struct for sorting edges by average depth (z)
typedef struct { int a, b; float z; } EdgeZ;
static int cmp_edge(const void *pa, const void *pb)
{
    const EdgeZ *A = pa, *B = pb;
    return (A->z < B->z) - (A->z > B->z);
}

// Project a 3D vertex using the MVP matrix and viewport size
vec3_t project_vertex(const mat4_t *mvp, const vec3_t *v, int w, int h)
{
    vec3_t t = mat4_mul_vec3(mvp, v);
    vec3_t out = {
        .x = (t.x * 0.5f + 0.5f) * w,
        .y = (1.0f - (t.y * 0.5f + 0.5f)) * h,
        .z = t.z,
        .r = 0, .theta = 0, .phi = 0,
        .dirty_cartesian = false,
        .dirty_spherical = false
    };
    return out;
}

// Clip test: is point inside circular viewport
int clip_to_viewport(const canvas_t *c, int x, int y)
{
    int cx = c->width / 2;
    int cy = c->height / 2;
    int r = (c->width < c->height ? c->width : c->height) / 2;
    int dx = x - cx, dy = y - cy;
    return dx * dx + dy * dy <= r * r;
}

// Render wireframe without lighting
void render_wireframe(canvas_t *c,
                      const vec3_t *verts, int n,
                      const int (*edges)[2], int ne,
                      const mat4_t *mvp)
{

    vec3_t *p = malloc(sizeof(vec3_t) * n);
    for (int i = 0; i < n; i++)
        p[i] = project_vertex(mvp, &verts[i], c->width, c->height);

    EdgeZ *list = malloc(sizeof(EdgeZ) * ne);
    for (int i = 0; i < ne; i++) {
        list[i].a = edges[i][0];
        list[i].b = edges[i][1];
        list[i].z = (p[list[i].a].z + p[list[i].b].z) * 0.5f;
    }
    qsort(list, ne, sizeof(EdgeZ), cmp_edge);

    // Draw edges sorted by average depth
    for (int i = 0; i < ne; i++) {
        vec3_t a = p[list[i].a], b = p[list[i].b];
        if (!clip_to_viewport(c, (int)a.x, (int)a.y) &&
            !clip_to_viewport(c, (int)b.x, (int)b.y)) continue;
        draw_line_f(c, a.x, a.y, b.x, b.y, 1.3f);
    }
    free(p);
    free(list);
}

// Compute vertex normals as average of edge perpendiculars
void compute_vertex_normals(const vec3_t *verts, int n_verts,
                            const int (*edges)[2], int n_edges,
                            vec3_t *out_normals)
{

    for (int i = 0; i < n_verts; i++) {
        out_normals[i].x = 0.0f;
        out_normals[i].y = 0.0f;
        out_normals[i].z = 0.0f;
        out_normals[i].dirty_cartesian = false;
        out_normals[i].dirty_spherical = false;
    }

    for (int i = 0; i < n_edges; i++) {
        int a = edges[i][0];
        int b = edges[i][1];

        vec3_t e = {
            verts[b].x - verts[a].x,
            verts[b].y - verts[a].y,
            verts[b].z - verts[a].z,
            0, 0, 0, false, false
        };

        float len = sqrtf(e.x*e.x + e.y*e.y + e.z*e.z);
        if (len > 1e-6f) {
            e.x /= len; e.y /= len; e.z /= len;
        }

        // Use a fixed up vector (0, 0, 1) to compute the normal
        vec3_t up = {0, 0, 1, 0, 0, 0, false, false};
        vec3_t n = {
            e.y * up.z - e.z * up.y,
            e.z * up.x - e.x * up.z,
            e.x * up.y - e.y * up.x,
            0, 0, 0, false, false
        };

        len = sqrtf(n.x*n.x + n.y*n.y + n.z*n.z);
        if (len > 1e-6f) {
            n.x /= len; n.y /= len; n.z /= len;
        }

        out_normals[a].x += n.x;
        out_normals[a].y += n.y;
        out_normals[a].z += n.z;

        out_normals[b].x += n.x;
        out_normals[b].y += n.y;
        out_normals[b].z += n.z;
    }
     
    // Normalize the normals
    for (int i = 0; i < n_verts; i++) {
        vec3_t *n = &out_normals[i];
        float len = sqrtf(n->x*n->x + n->y*n->y + n->z*n->z);
        if (len > 1e-6f) {
            n->x /= len;
            n->y /= len;
            n->z /= len;
        }
    }
}

// Render wireframe with Lambertian lighting on edges
void render_wireframe_lit(canvas_t *c,
                          const vec3_t *verts, int n_verts,
                          const int (*edges)[2], int n_edges,
                          const mat4_t *mvp,
                          const vec3_t *normals,
                          const vec3_t *light_dir)
{


    vec3_t *p = malloc(sizeof(vec3_t) * n_verts);
    for (int i = 0; i < n_verts; i++)
        p[i] = project_vertex(mvp, &verts[i], c->width, c->height);

    EdgeZ *list = malloc(sizeof(EdgeZ) * n_edges);
    for (int i = 0; i < n_edges; i++) {
        list[i].a = edges[i][0];
        list[i].b = edges[i][1];
        list[i].z = (p[list[i].a].z + p[list[i].b].z) * 0.5f;
    }
    qsort(list, n_edges, sizeof(EdgeZ), cmp_edge);

    // Draw edges sorted by average depth
    for (int i = 0; i < n_edges; i++) {
        vec3_t a = p[list[i].a], b = p[list[i].b];
        if (!clip_to_viewport(c, (int)a.x, (int)a.y) &&
            !clip_to_viewport(c, (int)b.x, (int)b.y)) continue;

        // Calculate Lambertian intensity for the edge
        float ia = lambert_intensity(&normals[list[i].a], light_dir);
        float ib = lambert_intensity(&normals[list[i].b], light_dir);
        float avg_intensity = 0.5f + 0.5f * (ia + ib) * 0.5f;
        float brightness = 0.3f + 0.9f * powf(avg_intensity, 1.2f);

        draw_line_f(c, a.x, a.y, b.x, b.y, 1.3f * brightness);
    }

    free(p);
    free(list);
}
