#include "swap_chain.h"

#include <stdio.h>

#include "GLFW/glfw3.h"

static VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR* capabilities, GLFWwindow* window) {
    VkExtent2D extent;

    if (capabilities->currentExtent.width != UINT32_MAX) {
        extent = capabilities->currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actual_extent = {
            (uint32_t)width,
            (uint32_t)height
        };

        actual_extent.width = (actual_extent.width < capabilities->minImageExtent.width)
                                ? capabilities->minImageExtent.width
                                : (actual_extent.width > capabilities->maxImageExtent.width)
                                    ? capabilities->maxImageExtent.width
                                    : actual_extent.width;

        actual_extent.height = (actual_extent.height < capabilities->minImageExtent.height)
                                ? capabilities->minImageExtent.height
                                : (actual_extent.height > capabilities->maxImageExtent.height)
                                    ? capabilities->maxImageExtent.height
                                    : actual_extent.height;

        extent = actual_extent;
    }

    return extent;
}

static VkPresentModeKHR choose_swap_present_mode(const VkPresentModeKHR* available_present_modes, const size_t available_present_size) {
    for(size_t i = 0; i < available_present_size; i++) {
        if(available_present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            return available_present_modes[i];
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}
static VkSurfaceFormatKHR choose_swap_surface_format(const VkSurfaceFormatKHR* available_formats, const size_t available_format_size) {
    for(size_t i = 0; i < available_format_size; i++) {
        if(available_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && available_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return available_formats[i];
        }
    }
    return available_formats[0];
}
void swap_chain_query_support_details(t_SwapChainSupportDetails *details, const VkSurfaceKHR *surface, const VkPhysicalDevice *device) {

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(*device, *surface, &details->capabilities);

    uint32_t format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(*device, *surface, &format_count, NULL);

    if (format_count != 0) {
        details->formats = (VkSurfaceFormatKHR *)malloc(format_count * sizeof(VkSurfaceFormatKHR));
        vkGetPhysicalDeviceSurfaceFormatsKHR(*device, *surface, &format_count, details->formats);
    }
    else {
        details->formats = NULL;
    }
    details->format_size = format_count;


    uint32_t present_mode_count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(*device, *surface, &present_mode_count, NULL);

    if (present_mode_count != 0) {
        details->present_modes = (VkPresentModeKHR *)malloc(present_mode_count * sizeof(VkPresentModeKHR));
        vkGetPhysicalDeviceSurfacePresentModesKHR(*device, *surface, &present_mode_count, details->present_modes);
    }
    else {
        details->present_modes = NULL;
    }
    details->present_size = present_mode_count;


}
void swap_chain_free_support(t_SwapChainSupportDetails *details) {
    if(details->formats) {
        free(details->formats);
        details->formats = NULL;
    }
    if(details->present_modes) {
        free(details->present_modes);
        details->present_modes = NULL;
    }
}
SwapChain swap_chain_create(VkSurfaceKHR *surface, const VkDevice *device, const VkPhysicalDevice *physical_device,
    GLFWwindow *window, const t_QueueFamilyIndices *indices) {

    SwapChain swap_chain = {};

    t_SwapChainSupportDetails swap_details = {};

    swap_chain_query_support_details(&swap_details, surface, physical_device);

    const VkSurfaceFormatKHR surface_format = choose_swap_surface_format(swap_details.formats, swap_details.format_size);
    const VkPresentModeKHR present_mode = choose_swap_present_mode(swap_details.present_modes, swap_details.present_size);
    const VkExtent2D extent = choose_swap_extent(&swap_details.capabilities, window);

    swap_chain.image_count = swap_details.capabilities.minImageCount + 1;

    if (swap_details.capabilities.maxImageCount > 0 && swap_chain.image_count > swap_details.capabilities.maxImageCount) {
        swap_chain.image_count = swap_details.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR create_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = *surface,
        .minImageCount = swap_chain.image_count,
        .imageFormat = surface_format.format,
        .imageColorSpace = surface_format.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
        };
    const uint32_t queue_family_indices[] = {indices->graphics_family.value, indices->present_family.value};

    if (indices->graphics_family.value != indices->present_family.value) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_family_indices;
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.queueFamilyIndexCount = 0;
        create_info.pQueueFamilyIndices = NULL;
    }

    create_info.preTransform = swap_details.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = present_mode;
    create_info.clipped = VK_TRUE;

    create_info.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(*device, &create_info, NULL, &swap_chain.instance) != VK_SUCCESS) {
        printf("\nfailed to create swap chain!");
        exit(-1);
    }

    vkGetSwapchainImagesKHR(*device, swap_chain.instance, &swap_chain.image_count, NULL);
    swap_chain.images = (VkImage*)malloc(swap_chain.image_count * sizeof(VkImage));
    vkGetSwapchainImagesKHR(*device, swap_chain.instance, &swap_chain.image_count, swap_chain.images);

    swap_chain.image_format = surface_format.format;
    swap_chain.extent = extent;

    swap_chain_free_support(&swap_details);

    return swap_chain;

}