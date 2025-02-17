#pragma once
#include "vk_device.h"

typedef struct Texture {
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    VkImage texture_image;
    uint32_t mip_levels;
    VkDeviceMemory texture_image_memory;
    VkImageView image_view;
    VkSampler texture_sampler;
}t_Texture;
t_Texture texture_init(const char* filepath, const t_Device *device, const VkCommandPool *command_pool);
void texture_cleanup(const VkDevice *device, const t_Texture *texture);
void create_image(const t_Device *device, VkImage *image, VkDeviceMemory *image_memory, VkFormat format, VkImageTiling tiling,
    VkImageUsageFlags usage, VkMemoryPropertyFlags properties, uint32_t mip_levels, VkSampleCountFlagBits num_samples, int tex_width, int tex_height);
void transition_image_layout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout, uint32_t mip_levels,
    const VkCommandPool *command_pool, const t_Device *device);