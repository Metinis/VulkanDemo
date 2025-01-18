#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <stdio.h>
#include <cglm/mat4.h>
#include <vulkan/vulkan_core.h>

#include "utils.h"

typedef struct GLFWwindow GLFWwindow;

typedef struct Application {
    GLFWwindow *window;
    VkInstance vk_instance;
    VkDebugUtilsMessengerEXT debug_messenger;
    uint8_t enable_validation_layers;
    const char **validation_layers;
    uint32_t validation_size;
    const char **device_extensions;
    uint32_t device_ext_size;
    VkPhysicalDevice physical_device;
    VkDevice device;
    VkQueue graphics_queue;
    VkSurfaceKHR surface;
    VkQueue present_queue;
    VkSwapchainKHR swap_chain;
    VkImage* swap_chain_images;
    VkFormat swap_chain_image_format;
    uint32_t swap_chain_image_count;
    VkExtent2D swap_chain_extent;
    VkImageView* swap_chain_image_views;
}t_Application;

void app_init(t_Application *app);

void app_run(const t_Application *app);

void app_end(const t_Application *app);