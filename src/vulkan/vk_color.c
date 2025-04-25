#include "vk_color.h"

static void create_color_resources(t_ColorData *color_data, const VkFormat *color_format, const t_Device *device, const VkExtent2D swap_chain_extent_2d) {
    create_image(device, &color_data->color_image, &color_data->color_image_memory, *color_format, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        1, device->msaa_samples, swap_chain_extent_2d.width, swap_chain_extent_2d.height);

    color_data->color_image_view = create_image_view(color_data->color_image, *color_format, VK_IMAGE_ASPECT_COLOR_BIT, 1, &device->instance);
}
t_ColorData color_data_init(const VkFormat *color_format, const t_Device *device, const VkExtent2D swap_chain_extent_2d) {
    t_ColorData color_data;
    create_color_resources(&color_data, color_format, device, swap_chain_extent_2d);
    return color_data;
}
void color_cleanup(const t_ColorData *color_data, const VkDevice *device) {
    vkDestroyImageView(*device, color_data->color_image_view, NULL);
    vkDestroyImage(*device, color_data->color_image, NULL);
    vkFreeMemory(*device, color_data->color_image_memory, NULL);
}