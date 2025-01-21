#include "vulkan_debug.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan_core.h>



void debug_print_available_extensions() {
    uint32_t extension_count = 0;

    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

    printf("Extensions supported %d\n", extension_count);

    VkExtensionProperties extensions[extension_count] = {};

    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions);

    for(int i = 0; i < extension_count; ++i) {
        printf("%s \n", extensions[i].extensionName);
    }
}
void debug_print_available_validation_layers() {
    uint32_t layer_count = 0;

    VkResult result = vkEnumerateInstanceLayerProperties(&layer_count, NULL);
    if (result != VK_SUCCESS) {
        printf("Failed to enumerate instance layers!\n");
        return;
    }

    VkLayerProperties* available_layers = (VkLayerProperties*)malloc(layer_count * sizeof(VkLayerProperties));

    result = vkEnumerateInstanceLayerProperties(&layer_count, available_layers);
    if (result != VK_SUCCESS) {
        printf("Failed to retrieve instance layer properties!\n");
        free(available_layers);
        return;
    }

    printf("Available validation layers:\n");
    for (uint32_t i = 0; i < layer_count; ++i) {
        printf("  %s\n", available_layers[i].layerName);
    }

    free(available_layers);
}
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_type,
    const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
    void* pUserData) {

    printf("validation layer: %s", p_callback_data->pMessage);

    return VK_FALSE;
}
void debug_populate_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT *create_info) {
    memset(create_info, 0, sizeof(VkDebugUtilsMessengerCreateInfoEXT));
    create_info->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    create_info->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create_info->pfnUserCallback = debug_callback;
}
static VkResult create_debug_utils_messenger_ext(const VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* p_create_info,
    const VkAllocationCallbacks* p_allocator, VkDebugUtilsMessengerEXT* p_debug_messenger)
{
    const PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance,
        "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, p_create_info, p_allocator, p_debug_messenger);
    } else {
        printf("Failed to load vkCreateDebugUtilsMessengerEXT function pointer!\n");
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void debug_setup_messenger(t_Application *app) {
    if (!app->enable_validation_layers) return;

    VkDebugUtilsMessengerCreateInfoEXT create_info;
    debug_populate_messenger_create_info(&create_info);

    if (create_debug_utils_messenger_ext(app->vk_instance, &create_info, nullptr, &app->debug_messenger) != VK_SUCCESS) {
        printf("failed to setup debug messenger!\n");
    }
}
void debug_destroy_utils_messenger_ext(const VkInstance instance, const VkDebugUtilsMessengerEXT debug_messenger, const VkAllocationCallbacks* p_allocator) {
    const PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debug_messenger, p_allocator);
    }
}