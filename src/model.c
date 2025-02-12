#include "model.h"
#include <stdio.h>
#include <stdlib.h>
#include <tinyobj_loader_c.h>
#include <stdio.h>

static void my_file_reader(
    void* ctx,
    const char* filename,
    int is_mtl,
    const char* obj_filename,
    char** data_out,
    unsigned long* len_out
) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Failed to open file: %s\n", filename);
        *data_out = NULL;
        *len_out = 0;
        return;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(file_size + 1); // +1 for null terminator
    if (!buffer) {
        printf("Failed to allocate memory for file: %s\n", filename);
        fclose(file);
        *data_out = NULL;
        *len_out = 0;
        return;
    }

    fread(buffer, 1, file_size, file);
    buffer[file_size] = '\0'; // Null terminate
    fclose(file);

    *data_out = buffer; // Store the allocated buffer
    *len_out = file_size; // Store the file size
}


void load_model(char* model_path, t_Vertex **vertices, uint32_t *num_verts, uint32_t **indices, uint32_t *num_indices) {
    tinyobj_attrib_t attrib;
    tinyobj_shape_t* shapes = NULL;
    tinyobj_material_t* material = NULL;
    size_t num_shapes, num_materials;
    tinyobj_attrib_init(&attrib);

    // Load the .obj file
    int ret = tinyobj_parse_obj(
        &attrib, &shapes, &num_shapes, &material, &num_materials,
        model_path, my_file_reader, NULL, 0
    );

    if (ret != TINYOBJ_SUCCESS) {
        printf("Failed to load .obj!\n");
        exit(-1);
    }
    printf("Successfully loaded .obj file: %s\n", model_path);
    printf("Number of shapes: %zu\n", num_shapes);
    printf("Number of materials: %zu\n", num_materials);
    printf("Number of vertices: %d\n", attrib.num_vertices);

    HashTable unique_vertices;
    init_table(&unique_vertices, INITIAL_SIZE);

    *vertices = (t_Vertex*)malloc(attrib.num_vertices * sizeof(t_Vertex));
    *indices = (unsigned int*)malloc(attrib.num_faces * 3 * sizeof(unsigned int)); // Each face has 3 indices

    *num_verts = 0;
    *num_indices = 0;

    for (size_t i = 0; i < num_shapes; i++) {
        size_t face_offset = 0; // Tracks position in faces array

        for (size_t f = 0; f < shapes[i].length; f++) { // Loop through faces
            for (int v = 0; v < 3; v++) { // Each face has 3 vertices (triangulated)
                int idx = attrib.faces[face_offset].v_idx;
                int idx_vt = attrib.faces[face_offset].vt_idx;

                if (idx < 0 || idx >= attrib.num_vertices) {
                    printf("Invalid vertex index: %d\n", idx);
                    exit(-1);
                }

                t_Vertex vertex = {
                    .pos = {
                        attrib.vertices[3 * idx + 0],
                        attrib.vertices[3 * idx + 1],
                        attrib.vertices[3 * idx + 2]
                    },
                    .tex_coord = {0.0f, 0.0f},
                    .color = {1.0f, 1.0f, 1.0f}
                };

                vertex.tex_coord[0] = attrib.texcoords[2 * idx_vt + 0];
                vertex.tex_coord[1] = 1.0f - attrib.texcoords[2 * idx_vt + 1];

                uint32_t vertex_index;
                if (!search(&unique_vertices, &vertex, &vertex_index)) {
                    vertex_index = *num_verts;
                    (*vertices)[vertex_index] = vertex;
                    insert(&unique_vertices, &vertex, vertex_index);
                    (*num_verts)++;
                }

                (*indices)[*num_indices] = vertex_index;
                (*num_indices)++;

                face_offset++;
            }
        }
    }

    printf("Number of unique vertices: %d\n", *num_verts);
    printf("Number of indices: %d\n", *num_indices);

    // Clean up
    free_table(&unique_vertices);
    tinyobj_attrib_free(&attrib);
    free(shapes);
    free(material);
}