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
VkFormat find_supported_format(const VkFormat* candidates, const size_t candidate_size, const VkImageTiling tiling, const VkFormatFeatureFlags features,
                                      const VkPhysicalDevice *physical_device) {

    for(size_t i = 0; i < candidate_size; i++) {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(*physical_device, candidates[i], &properties);

        if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features) {
            return candidates[i];
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features) {
            return candidates[i];
        }
    }
    printf("Failed to find supported format! \n");
    return -1;
}

