#include "application.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static void print_available_validation_layers() {
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
static bool check_validation_layer_support(const char** validation_layers, size_t validation_size) {
    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

    VkLayerProperties available_layers[layer_count];
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers);

    for(size_t i = 0; i < validation_size; i++) {
        uint8_t layer_found = 0;

        for(size_t j = 0; j < layer_count; j++) {
            if (strcmp(validation_layers[i], available_layers[j].layerName) == 0) {
                layer_found = 1;
                break;
            }
        }
        if(!layer_found) {
            return 0;
        }
    }
    return 1;
}
static void app_enable_extensions(VkInstanceCreateInfo* create_info, const char*** extensions_ptr,
    const uint8_t enable_validation_layers) {

    uint32_t glfw_extensionCount = 0;
    const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extensionCount);

    create_info->enabledExtensionCount = glfw_extensionCount;
    create_info->ppEnabledExtensionNames = glfw_extensions;

    create_info->enabledLayerCount = 0;

    //****FOR MAC VK_INCOMPATIBLE DRIVER ****

    uint32_t extension_count = glfw_extensionCount + 1;
    if(enable_validation_layers) {
        extension_count++;
    }

    const char** required_extensions = (const char**)malloc(extension_count * sizeof(const char*));

    for(uint32_t i = 0; i < glfw_extensionCount; i++) {
        required_extensions[i] = glfw_extensions[i];
    }

    required_extensions[glfw_extensionCount] = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;

    if (enable_validation_layers) {
        required_extensions[glfw_extensionCount + 1] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    }

    create_info->flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

    create_info->enabledExtensionCount = extension_count;
    create_info->ppEnabledExtensionNames = required_extensions;

    //****FOR MAC VK_INCOMPATIBLE DRIVER ****

    *extensions_ptr = required_extensions;
}
static void app_enable_validation(VkInstanceCreateInfo* create_info) {
    const char* validation_layers[] = {
         "VK_LAYER_KHRONOS_validation"
     };
     const uint32_t validation_size = 1;

    if (check_validation_layer_support(validation_layers, validation_size)) {
        create_info->enabledLayerCount = (uint32_t)(validation_size);
        create_info->ppEnabledLayerNames = validation_layers;
    } else {
        create_info->enabledLayerCount = 0;
        create_info->ppEnabledLayerNames = nullptr;
    }

}
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    printf("validation layer: %s", pCallbackData->pMessage);

    return VK_FALSE;
}
static void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT *createInfo) {
    memset(createInfo, 0, sizeof(VkDebugUtilsMessengerCreateInfoEXT));
    createInfo->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo->pfnUserCallback = debug_callback;
}
static void app_create_vulkan_inst(t_Application *app) {
    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "VulkanDemo";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "Vetox";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;

    app->enable_validation_layers = 1;

#ifdef NDEBUG
    app->enable_validation_layers = 0;
#endif

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};



    if(app->enable_validation_layers) {
        app_enable_validation(&create_info);

        populate_debug_messenger_create_info(&debugCreateInfo);
        create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    }

    const char** extensions = NULL;
    app_enable_extensions(&create_info, &extensions, app->enable_validation_layers);

    print_available_validation_layers();



    const VkResult result = vkCreateInstance(&create_info, nullptr, &app->m_vk_instance);

    printf("Enabled Vulkan extensions:\n");
    for (uint32_t i = 0; i < create_info.enabledExtensionCount; i++) {
        printf("  %s\n", extensions[i]);
    }

    free(extensions);

    if (result != VK_SUCCESS) {
        printf("Failed to create Vulkan instance. VkResult: %d\n", result);
    }



}
static VkResult create_debug_utils_messenger_ext(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    const PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance,
        "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        printf("Failed to load vkCreateDebugUtilsMessengerEXT function pointer!\n");
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

static void setup_debug_messenger(t_Application *app) {
    if (!app->enable_validation_layers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populate_debug_messenger_create_info(&createInfo);

    if (create_debug_utils_messenger_ext(app->m_vk_instance, &createInfo, nullptr, &app->debug_messenger) != VK_SUCCESS) {
        printf("failed to setup debug messenger!\n");
    }
}
static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    const PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}
static void app_vulkan_init(t_Application *app) {
    uint32_t extension_count = 0;

    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

    printf("Extensions supported %d\n", extension_count);

    VkExtensionProperties extensions[extension_count] = {};

    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions);

    for(int i = 0; i < extension_count; ++i) {
        printf("%s \n", extensions[i].extensionName);
    }


    app_create_vulkan_inst(app);

    setup_debug_messenger(app);
}
void app_init(t_Application *app) {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    app->m_window = glfwCreateWindow(1280,720, "VulkanDemo", nullptr, nullptr);

    app_vulkan_init(app);
}
void app_run(const t_Application *app) {
    while(!glfwWindowShouldClose(app->m_window)) {
        glfwPollEvents();
    }
}
void app_end(const t_Application *app) {
    if (app->enable_validation_layers) {
        DestroyDebugUtilsMessengerEXT(app->m_vk_instance, app->debug_messenger, nullptr);
    }
    vkDestroyInstance(app->m_vk_instance, nullptr);
    glfwDestroyWindow(app->m_window);
    glfwTerminate();
}
