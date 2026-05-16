#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "canvas.h"
#include "math3d.h"
#include "renderer.h"


#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif


#define W        480
#define H        480
#define FRAMES   100
#define RADIUS   200.0f

// Edge structure for storing edges as pairs of vertex indices
typedef struct { int a, b; } edge_t;

// Function to generate a soccer ball mesh from an OBJ file
static int generate_soccer_ball(const char* filename,
    vec3_t** out_verts, int* out_vcount,
    int (**out_edges)[2], int* out_ecount)
{

    // Open the OBJ file for reading
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
            verts = realloc(verts, sizeof(vec3_t) * (vcount + 1));
            verts[vcount++] = v;
        } else if (strncmp(line, "f ", 2) == 0) {
            int idx[16]; int n = 0;
            char* tok = strtok(line + 2, " ");
            while (tok && n < 16) {
                int vi = 0;
                sscanf(tok, "%d", &vi);
                // convert to 0-based index
                idx[n++] = vi - 1; 
                tok = strtok(NULL, " ");
            }
            for (int i = 0; i < n; ++i) {
                int a = idx[i];
                int b = idx[(i + 1) % n]; // loop back
                if (a > b) { int t = a; a = b; b = t; }

                // skip duplicate edge
                int dup = 0;
                for (int j = 0; j < ecount; ++j) {
                    if (edges[j].a == a && edges[j].b == b) {
                        dup = 1;
                        break;
                    }
                }
                if (!dup) {
                    edges = realloc(edges, sizeof(edge_t) * (ecount + 1));
                    edges[ecount++] = (edge_t){a, b};
                }
            }
        }
    }
    fclose(f);

    // Check if we found any vertices or edges
    int (*edge_arr)[2] = malloc(sizeof(int[2]) * ecount);
    for (int i = 0; i < ecount; ++i) {
        edge_arr[i][0] = edges[i].a;
        edge_arr[i][1] = edges[i].b;
    }

    // If no vertices or edges were found, return failure
    free(edges);
    *out_verts = verts;
    *out_edges = edge_arr;
    *out_vcount = vcount;
    *out_ecount = ecount;
    return 1;
}

/* main() */

int main(void)
{
    // Load geometry
    vec3_t* verts = NULL;
    int (*edges)[2] = NULL;
    int vcount = 0, ecount = 0;

    if (!generate_soccer_ball("demo/soccerball.txt", &verts, &vcount, &edges, &ecount)) {
        fprintf(stderr, "Could not load demo/soccerball.txt\n");
        return 1;
    }

    // Scale the model down ( to 50% size)
    float scale_factor = 0.2f;
    for (int i = 0; i < vcount; ++i) {
        verts[i].x *= scale_factor;
        verts[i].y *= scale_factor;
        verts[i].z *= scale_factor;
    }


    // Create canvas
    canvas_t* cv = create_canvas(W, H);

    // Task 3: render soccerball animation
    mat4_t view = mat4_translate(0, 0, -2);
    mat4_t proj = mat4_frustum(-1, 1, -1, 1, 1, 10);


    for (int f = 0; f < FRAMES; ++f) {
        float a = f * 0.07f;
        mat4_t model = mat4_rotate_xyz(0.4f * a, a, 0);
        mat4_t pv = mat4_multiply(&proj, &view);
        mat4_t mvp = mat4_multiply(&pv, &model);

        // Set up light direction
        canvas_clear(cv, 0.0f);
        render_wireframe(cv, verts, vcount, edges, ecount, &mvp);


        char filename[64];
        sprintf(filename, "frames/frameb_%03d.pgm", f);
        save_canvas_pgm(cv, filename);
        printf("  wrote %s\n", filename);
    }

    // Free allocated resources
    free_canvas(cv);
    free(verts);
    free(edges);

    puts(" All done. Convert frames to MP4 or GIF as needed.");
    return 0;
}
