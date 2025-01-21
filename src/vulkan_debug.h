#pragma once
#include <vulkan/vulkan_core.h>
#include "application.h"

void debug_print_available_extensions();

void debug_print_available_validation_layers();

void debug_populate_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT *create_info);

void debug_setup_messenger(t_Application *app);

void debug_destroy_utils_messenger_ext(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger, const VkAllocationCallbacks* p_allocator);