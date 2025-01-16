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
    VkPhysicalDevice physical_device;
    VkDevice device;
    VkQueue graphics_queue;
}t_Application;

void app_init(t_Application *app);

void app_run(const t_Application *app);

void app_end(const t_Application *app);