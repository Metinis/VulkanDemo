#pragma once
#include <vulkan/vulkan_core.h>
#include "application.h"

void debug_print_available_extensions();

void debug_print_available_validation_layers();

void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT *createInfo);

void setup_debug_messenger(t_Application *app);

void destroy_debug_utils_messenger_ext(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);