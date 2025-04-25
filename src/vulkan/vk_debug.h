#pragma once
#include <vulkan/vulkan_core.h>

void debug_print_available_extensions();

void debug_print_available_validation_layers();

void debug_populate_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT *create_info);

void debug_setup_messenger(uint8_t enable_validation_layers, const VkInstance *instance, VkDebugUtilsMessengerEXT *debug_messenger);

void debug_destroy_utils_messenger_ext(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger, const VkAllocationCallbacks* p_allocator);