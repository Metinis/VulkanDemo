#include "application.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vulkan_debug.h"


static uint8_t check_validation_layer_support(const char** validation_layers, size_t validation_size) {
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
static uint8_t check_device_extension_support(VkPhysicalDevice device) {
    //set required extensions
    uint32_t device_ext_num = 1;
    const char* device_extensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    //enumerate and find if they exist
    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

    VkExtensionProperties available_extensions[extension_count];
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_extensions);

    uint32_t matching_ext_count = 0;

    for(int i = 0; i < extension_count; i++) {
        printf("\n Ext: %s", available_extensions[i].extensionName);
        for(int j = 0; j < device_ext_num; j++) {
            if (strcmp(available_extensions[i].extensionName, device_extensions[j]) == 0) {
                matching_ext_count++;
            }
        }
    }

    return matching_ext_count == device_ext_num;
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

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {};

    if(app->enable_validation_layers) {
        app_enable_validation(&create_info);

        populate_debug_messenger_create_info(&debug_create_info);
        create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debug_create_info;

        debug_print_available_validation_layers();
    }

    const char** extensions = NULL;
    app_enable_extensions(&create_info, &extensions, app->enable_validation_layers);

    const VkResult result = vkCreateInstance(&create_info, nullptr, &app->vk_instance);

    if(app->enable_validation_layers) {
        printf("Enabled Vulkan extensions:\n");
        for (uint32_t i = 0; i < create_info.enabledExtensionCount; i++) {
            printf("  %s\n", extensions[i]);
        }
    }

    free(extensions);

    if (result != VK_SUCCESS) {
        printf("Failed to create Vulkan instance. VkResult: %d\n", result);
    }
}
static QueueFamilyIndices find_queue_families(t_Application *app, VkPhysicalDevice device) {
    QueueFamilyIndices indices = {
        .graphics_family = { .value = 0, .has_value = 0 },
        .present_family = { .value = 0, .has_value = 0 }
    };

    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

    VkQueueFamilyProperties queue_families[queue_family_count];
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families);

    for(int i = 0; i < queue_family_count; i++) {
        if(queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphics_family.has_value = 1;
            indices.graphics_family.value = i;
        }
        VkBool32 presentSupport = 0;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, app->surface, &presentSupport);
        if (presentSupport) {
            indices.present_family.has_value = 1;
            indices.present_family.value = i;
        }
        if(is_complete(&indices)) {
            break;
        }
        i++;
    }

    return indices;
}
static uint8_t is_device_suitable(t_Application *app, VkPhysicalDevice device) {
    //TODO to be added to
    /*VkPhysicalDeviceProperties device_properties;
    VkPhysicalDeviceFeatures device_features;
    vkGetPhysicalDeviceProperties(device, &device_properties);
    vkGetPhysicalDeviceFeatures(device, &device_features);

    return device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
           device_features.geometryShader;*/
    const QueueFamilyIndices indices = find_queue_families(app, device);

    return is_complete(&indices) && check_device_extension_support(device);
}

static void pick_physical_device(t_Application *app) {
    app->physical_device = VK_NULL_HANDLE;

    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(app->vk_instance, &device_count, nullptr);

    if (device_count == 0) {
        printf("Failed to find device with vulkan support!\n");
        exit(-1);
    }

    VkPhysicalDevice devices[device_count];
    vkEnumeratePhysicalDevices(app->vk_instance, &device_count, devices);

    for(int i = 0; i < device_count; i++) {
        if(is_device_suitable(app, devices[i])) {
            app->physical_device = devices[i];
            break;
        }
    }
    if(app->physical_device == VK_NULL_HANDLE) {
        printf("Failed to find suitable device!\n");
        exit(-1);
    }
}
static void app_enable_validation_device(VkDeviceCreateInfo* create_info) {
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
static void create_logical_device(t_Application *app) {
    QueueFamilyIndices indices = find_queue_families(app, app->physical_device);

    if (!indices.graphics_family.has_value || !indices.present_family.has_value) {
        printf("Error: Required queue families are not available!\n");
        exit(-1);
    }

    uint32_t unique_queue_families[2] = {indices.graphics_family.value, indices.present_family.value};
    uint32_t queue_size = (indices.graphics_family.value != indices.present_family.value) ? 2 : 1;

    float queue_priority = 1.0f;

    VkDeviceQueueCreateInfo queue_create_infos[queue_size];
    VkDeviceQueueCreateInfo base_queue_create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .queueCount = 1,
        .pQueuePriorities = &queue_priority,
    };

    for (uint32_t i = 0; i < queue_size; i++) {
        queue_create_infos[i] = base_queue_create_info;
        queue_create_infos[i].queueFamilyIndex = unique_queue_families[i];
    }

    // Specify device features
    VkPhysicalDeviceFeatures device_features = {};

    //TODO move this to be a variable outside the func
    uint32_t device_ext_num = 1;
    const char* device_extensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkDeviceCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pQueueCreateInfos = queue_create_infos,
        .queueCreateInfoCount = queue_size,
        .pEnabledFeatures = &device_features,
        .enabledExtensionCount = device_ext_num,
        .ppEnabledExtensionNames = device_extensions
    };

    app_enable_validation_device(&create_info);

    if (vkCreateDevice(app->physical_device, &create_info, NULL, &app->device) != VK_SUCCESS) {
        printf("Error: Failed to create Vulkan logical device!\n");
        exit(-1);
    }

    // Retrieve queue handles
    vkGetDeviceQueue(app->device, indices.graphics_family.value, 0, &app->graphics_queue);
    vkGetDeviceQueue(app->device, indices.present_family.value, 0, &app->present_queue);

    printf("Logical device created successfully.\n");
}

static void create_surface(t_Application *app) {
    if (glfwCreateWindowSurface(app->vk_instance, app->window, nullptr, &app->surface) != VK_SUCCESS) {
        printf("Failed to create window surface! \n");
    }
}
static void app_vulkan_init(t_Application *app) {
    debug_print_available_extensions();

    app_create_vulkan_inst(app);

    setup_debug_messenger(app);

    create_surface(app);

    pick_physical_device(app);

    create_logical_device(app);
}


void app_init(t_Application *app) {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    app->window = glfwCreateWindow(1280,720, "VulkanDemo", nullptr, nullptr);

    app_vulkan_init(app);
}
void app_run(const t_Application *app) {
    while(!glfwWindowShouldClose(app->window)) {
        glfwPollEvents();
    }
}
void app_end(const t_Application *app) {
    if (app->enable_validation_layers) {
        destroy_debug_utils_messenger_ext(app->vk_instance, app->debug_messenger, nullptr);
    }
    vkDestroyDevice(app->device, nullptr);
    vkDestroySurfaceKHR(app->vk_instance, app->surface, nullptr);
    vkDestroyInstance(app->vk_instance, nullptr);
    glfwDestroyWindow(app->window);
    glfwTerminate();
}
