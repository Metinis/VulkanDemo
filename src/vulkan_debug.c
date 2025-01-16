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
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    printf("validation layer: %s", pCallbackData->pMessage);

    return VK_FALSE;
}
void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT *createInfo) {
    memset(createInfo, 0, sizeof(VkDebugUtilsMessengerCreateInfoEXT));
    createInfo->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo->pfnUserCallback = debug_callback;
}
static VkResult create_debug_utils_messenger_ext(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    const PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance,
        "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        printf("Failed to load vkCreateDebugUtilsMessengerEXT function pointer!\n");
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void setup_debug_messenger(t_Application *app) {
    if (!app->enable_validation_layers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populate_debug_messenger_create_info(&createInfo);

    if (create_debug_utils_messenger_ext(app->vk_instance, &createInfo, nullptr, &app->debug_messenger) != VK_SUCCESS) {
        printf("failed to setup debug messenger!\n");
    }
}
void destroy_debug_utils_messenger_ext(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    const PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}