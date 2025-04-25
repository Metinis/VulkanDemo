#pragma once
#include <vulkan/vulkan_core.h>
#include "../utils.h"
#include "GLFW/glfw3.h"

typedef struct Device {
    const char **device_extensions;
    uint32_t device_ext_size;
    VkPhysicalDevice physical_device;
    VkDevice instance;
    VkQueue graphics_queue;
    VkSurfaceKHR surface;
    VkQueue present_queue;
    VkSampleCountFlagBits msaa_samples;
}t_Device;

t_Device device_init(const VkInstance *instance, GLFWwindow *window, t_QueueFamilyIndices *indices,
    uint8_t has_validation_support, const char** validation_layers, uint32_t validation_size);

void device_destroy(const t_Device *device, const VkInstance *instance);