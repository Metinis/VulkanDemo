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
VkVertexInputBindingDescription get_binding_description_vertex() {
    VkVertexInputBindingDescription binding_description = {
        .binding = 0,
        .stride = sizeof(t_Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };
    return binding_description;
}
void get_attribute_descriptions_vertex(VkVertexInputAttributeDescription* attribute_descriptions) {
    attribute_descriptions[0] = (VkVertexInputAttributeDescription){
        .binding = 0,
        .location = 0,
        .format = VK_FORMAT_R32G32_SFLOAT, // vec2 (pos)
        .offset = offsetof(t_Vertex, pos)
    };

    attribute_descriptions[1] = (VkVertexInputAttributeDescription){
        .binding = 0,
        .location = 1,
        .format = VK_FORMAT_R32G32B32_SFLOAT, // vec3 (color)
        .offset = offsetof(t_Vertex, color)
    };
}
uint32_t find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties, VkPhysicalDevice physical_device) {
    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_properties);

    for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
        if (type_filter & (1 << i) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    printf("failed to find suitable memory type! \n");
    return -1;
}
