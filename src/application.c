#include "application.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
static const char** get_required_extensions(const uint8_t enable_validation_layers, uint32_t* extension_count) {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    uint32_t totalExtensionCount = glfwExtensionCount + (enable_validation_layers ? 1 : 0);

    const char** extensions = (const char**)malloc(totalExtensionCount * sizeof(const char*));
    if (!extensions) {
        fprintf(stderr, "Failed to allocate memory for extensions.\n");
        exit(EXIT_FAILURE);
    }

    for (uint32_t i = 0; i < glfwExtensionCount; i++) {
        extensions[i] = glfwExtensions[i];
    }

    if (enable_validation_layers) {
        extensions[glfwExtensionCount] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    }

    return extensions;
}
static void app_enable_extensions(VkInstanceCreateInfo* create_info, const char*** extensions_ptr) {
    uint32_t glfw_extensionCount = 0;
    const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extensionCount);

    create_info->enabledExtensionCount = glfw_extensionCount;
    create_info->ppEnabledExtensionNames = glfw_extensions;

    create_info->enabledLayerCount = 0;

    //****FOR MAC VK_INCOMPATIBLE DRIVER ****

    const uint32_t extension_count = glfw_extensionCount + 1;

    const char** required_extensions = (const char**)malloc(extension_count * sizeof(const char*));

    for(uint32_t i = 0; i < glfw_extensionCount; i++) {
        required_extensions[i] = glfw_extensions[i];
    }

    required_extensions[glfw_extensionCount] = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;

    create_info->flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

    create_info->enabledExtensionCount = extension_count;
    create_info->ppEnabledExtensionNames = required_extensions;

    //****FOR MAC VK_INCOMPATIBLE DRIVER ****

    *extensions_ptr = required_extensions;
}
static void app_enable_validation(VkInstanceCreateInfo* create_info, const char*** extensions_ptr) {
    const char* validation_layers[] = {
         "VK_LAYER_KHRONOS_validation"
     };
     const size_t validation_size = 1;
     uint8_t enable_validation_layers = 1;

     #ifdef NDEBUG
         enable_validation_layers = 0;
     #endif

     if (enable_validation_layers && !check_validation_layer_support(validation_layers, validation_size)) {
         printf("validation layers requested, but not available!");
     }
     if (enable_validation_layers) {
         create_info->enabledLayerCount = (uint32_t)(validation_size);
         create_info->ppEnabledLayerNames = validation_layers;
     } else {
         create_info->enabledLayerCount = 0;
     }
     uint32_t ext_count = 0;
     const char** extensions = get_required_extensions(enable_validation_layers, &ext_count);

     create_info->enabledExtensionCount = ext_count;
     create_info->ppEnabledExtensionNames = extensions;

     *extensions_ptr = extensions;

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

    const char** extensions = NULL;
    app_enable_extensions(&create_info, &extensions);

    const VkResult result = vkCreateInstance(&create_info, nullptr, &app->m_vk_instance);
    free(extensions);

    if (result != VK_SUCCESS) {
        printf("Failed to create Vulkan instance. VkResult: %d\n", result);
    }
    extensions = NULL;
    app_enable_validation(&create_info, &extensions);
    free(extensions);
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
    vkDestroyInstance(app->m_vk_instance, nullptr);
    glfwDestroyWindow(app->m_window);
    glfwTerminate();
}
