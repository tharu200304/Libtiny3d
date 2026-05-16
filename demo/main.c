#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "canvas.h"
#include "math3d.h"
#include "renderer.h"
#include "lighting.h"
#include "animation.h"


#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif


#define W        600
#define H        600
#define FRAMES   200
#define RADIUS   200.0f


typedef struct { int a, b; } edge_t;

// Generate a soccer ball mesh from an OBJ file
static int generate_soccer_ball(const char* filename,
    vec3_t** out_verts, int* out_vcount,
    int (**out_edges)[2], int* out_ecount)
{
    FILE* f = fopen(filename, "r");
    if (!f) {
        perror("Failed to open TXT file");
        return 0;
    }

    // Allocate initial memory for vertices and edges
    vec3_t* verts = NULL;
    int vcount = 0;
    edge_t* edges = NULL;
    int ecount = 0;

    // Read the OBJ file line by line
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "v ", 2) == 0) {
            vec3_t v;
            sscanf(line + 2, "%f %f %f", &v.x, &v.y, &v.z);
            v.r = v.theta = v.phi = 0;
            v.dirty_cartesian = true;
            v.dirty_spherical = true;
            verts = realloc(verts, sizeof(vec3_t) * (vcount + 1));
            verts[vcount++] = v;
        } else if (strncmp(line, "f ", 2) == 0) {
            int idx[16], n = 0;
            char* tok = strtok(line + 2, " ");
            while (tok && n < 16) {
                int vi;
                sscanf(tok, "%d", &vi);
                idx[n++] = vi - 1;
                tok = strtok(NULL, " ");
            }
            for (int i = 0; i < n; ++i) {
                int a = idx[i], b = idx[(i + 1) % n];
                if (a > b) { int t = a; a = b; b = t; }
                int dup = 0;
                for (int j = 0; j < ecount; ++j) {
                    if (edges[j].a == a && edges[j].b == b) { dup = 1; break; }
                }
                if (!dup) {
                    edges = realloc(edges, sizeof(edge_t) * (ecount + 1));
                    edges[ecount++] = (edge_t){a, b};
                }
            }
        }
    }
    fclose(f);


    int (*edge_arr)[2] = malloc(sizeof(int[2]) * ecount);
    for (int i = 0; i < ecount; ++i) {
        edge_arr[i][0] = edges[i].a;
        edge_arr[i][1] = edges[i].b;
    }
    free(edges);

    *out_verts = verts;
    *out_edges = edge_arr;
    *out_vcount = vcount;
    *out_ecount = ecount;
    return 1;
}

// Main function to render the soccer ball animation
int main(void)
{
    vec3_t* verts = NULL;
    int (*edges)[2] = NULL;
    int vcount = 0, ecount = 0;

    // Load the soccer ball mesh from the OBJ file
    if (!generate_soccer_ball("demo/soccerball.txt", &verts, &vcount, &edges, &ecount)) {
        fprintf(stderr, " Could not load demo/soccerball.txt\n");
        return 1;
    }


    float base_scale = 0.4f;
    for (int i = 0; i < vcount; ++i) {
        verts[i].x *= base_scale;
        verts[i].y *= base_scale;
        verts[i].z *= base_scale;
    }

    // Compute vertex normals for lighting
    vec3_t* normals = malloc(sizeof(vec3_t) * vcount);
    for (int i = 0; i < vcount; ++i) {
        normals[i] = verts[i];
        vec3_normalize(&normals[i]);
    }


    vec3_t p0 = { 2, 0, -4, 0,0,0,true,true };
    vec3_t p1 = { 7, -6, -4, 0,0,0,true,true };
    vec3_t p2 = { -4, 7, 6, 0,0,0,true,true };
    vec3_t p3 = p0;


    animation_t anim;
    animation_init(&anim, (float)FRAMES, true);

    // Create a canvas to render the animation frames
    canvas_t* cv = create_canvas(W, H);
    canvas_clear(cv, 0.0f);

    // Set up the view and projection matrices
    mat4_t view = mat4_translate(-1, 0, -4);
    mat4_t proj = mat4_frustum(-1, 1, -1, 1, 1, 10);
    vec3_t light_dir = { 1.0f, 1.5f, -1.5f, 0,0,0,true,true };
    vec3_normalize(&light_dir);

    // Render the animation frames
    for (int f = 0; f < FRAMES; ++f) {
        float t = animation_update(&anim, 1.0f);
        float t2 = fmodf(t + 0.5f, 1.0f); // Offset for second ball

        // Calculate the Bezier curve points for the two balls
        vec3_t offset1 = bezier3(&p0, &p1, &p2, &p3, t);
        vec3_t offset2 = bezier3(&p0, &p1, &p2, &p3, t2);

        // Calculate rotation angles based on time
        float angle_x = 0.4f * t * 2 * M_PI;
        float angle_y = t * 2 * M_PI;

        // Create model matrices for the two balls
        mat4_t scale = mat4_scale(0.5, 0.5, 0.5);
        mat4_t rot = mat4_rotate_xyz(angle_x, angle_y, 0);

        // Apply translation to the model matrices for each ball
        mat4_t trans1 = mat4_translate(offset1.x, offset1.y, offset1.z);
        mat4_t model_tmp1 = mat4_multiply(&rot, &scale);
        mat4_t model1 = mat4_multiply(&trans1, &model_tmp1);

        // Second ball with offset
        mat4_t trans2 = mat4_translate(offset2.x, offset2.y, offset2.z);
        mat4_t model_tmp2 = mat4_multiply(&rot, &scale);
        mat4_t model2 = mat4_multiply(&trans2, &model_tmp2);

        // Combine the view, projection, and model matrices
        mat4_t pv = mat4_multiply(&proj, &view);
        mat4_t mvp1 = mat4_multiply(&pv, &model1);
        mat4_t mvp2 = mat4_multiply(&pv, &model2);

        // Clear the canvas and render both balls
        canvas_clear(cv, 0.0f);
        render_wireframe_lit(cv, verts, vcount, edges, ecount, &mvp1, normals, &light_dir);
        render_wireframe_lit(cv, verts, vcount, edges, ecount, &mvp2, normals, &light_dir);

        // Save the frame as a PGM image
        char filename[64];
        sprintf(filename, "frames/frame_%03d.pgm", f);
        save_canvas_pgm(cv, filename);
        printf("  wrote %s\n", filename);
    }

    // Free allocated resources
    free(normals);
    free(verts);
    free(edges);
    free_canvas(cv);

    puts(" All done. Convert frames to MP4 or GIF as needed.");
    return 0;
}
