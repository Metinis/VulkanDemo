#include "utils.h"

#include <stdio.h>

uint8_t is_complete(const t_QueueFamilyIndices* indices) {
    return indices->graphics_family.has_value && indices->present_family.has_value;
}
unsigned char* read_file(const char* filename, size_t* file_size) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Failed to open file: %s\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    *file_size = ftell(file);
    rewind(file);

    unsigned char* buffer = (unsigned char*)malloc(*file_size);
    if (!buffer) {
        fprintf(stderr, "Failed to allocate memory for file contents.\n");
        fclose(file);
        return NULL;
    }

    fread(buffer, 1, *file_size, file);
    fclose(file);

    return buffer;
}

