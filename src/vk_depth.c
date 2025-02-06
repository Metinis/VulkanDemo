#include "vk_depth.h"

#include <stdio.h>
#include <stdlib.h>




uint8_t has_stencil_component(const VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}
VkFormat depth_find_format(const VkPhysicalDevice *physical_device) {
    const size_t candidate_size = 3;
    const VkFormat candidates[candidate_size] = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};

    return find_supported_format(
        candidates, candidate_size,
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, physical_device
        );
}

static void depth_create_resources(t_DepthData *depth_data, const t_Device *device, const VkExtent2D extent_2d) {
    const VkFormat depth_format = depth_find_format(&device->physical_device);

    create_image(device, &depth_data->depth_image, &depth_data->depth_image_memory, depth_format, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, extent_2d.width, extent_2d.height);

    depth_data->depth_image_view = create_image_view(depth_data->depth_image, depth_format, VK_IMAGE_ASPECT_DEPTH_BIT, &device->instance);

    //transition_image_layout(depth_data->depth_image, depth_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    //    command_pool, device);
}
t_DepthData depth_init(const t_Device *device, const VkExtent2D extent_2d) {
    t_DepthData depth_data;
    depth_create_resources(&depth_data, device, extent_2d);
    return depth_data;
}
void depth_cleanup(const t_DepthData *depth_data, const VkDevice *device){
    vkDestroyImageView(*device, depth_data->depth_image_view, NULL);
    vkDestroyImage(*device, depth_data->depth_image, NULL);
    vkFreeMemory(*device, depth_data->depth_image_memory, NULL);
}