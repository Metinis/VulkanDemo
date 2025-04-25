#pragma once
#include <vulkan/vulkan_core.h>
#include "vk_swap_chain.h"
#include "../texture.h"
#include "../utils.h"

typedef struct DepthData {
    VkImage depth_image;
    VkDeviceMemory depth_image_memory;
    VkImageView depth_image_view;

} t_DepthData;
uint8_t has_stencil_component(VkFormat format);
VkFormat depth_find_format(const VkPhysicalDevice *physical_device);
t_DepthData depth_init(const t_Device *device, VkExtent2D extent_2d);
void depth_cleanup(const t_DepthData *depth_data, const VkDevice *device);