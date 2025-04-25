#pragma once
#include <vulkan/vulkan_core.h>
#include "../texture.h"
#include "vk_swap_chain.h"

typedef struct ColorData {
    VkImage color_image;
    VkDeviceMemory color_image_memory;
    VkImageView color_image_view;
}t_ColorData;

t_ColorData color_data_init(const VkFormat *color_format, const t_Device *device, VkExtent2D swap_chain_extent_2d);

void color_cleanup(const t_ColorData *color_data, const VkDevice *device);