#pragma once
#include "vk_device.h"

typedef struct Texture {
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    VkImage texture_image;
    VkDeviceMemory texture_image_memory;
    VkImageView image_view;
    VkSampler texture_sampler;
}t_Texture;
t_Texture texture_init(const char* filepath, const t_Device *device, const VkCommandPool *command_pool);
void texture_cleanup(const VkDevice *device, const t_Texture *texture);