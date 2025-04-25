#include "vk_device.h"

#include <stdio.h>
#include <string.h>

#include "../application.h"
#include "vk_swap_chain.h"
#include "GLFW/glfw3.h"
static uint8_t device_check_extension_support(const t_Device *device, const VkPhysicalDevice *physical_device) {
    //enumerate and find if they exist
    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(*physical_device, NULL, &extension_count, NULL);

    VkExtensionProperties available_extensions[extension_count];
    vkEnumerateDeviceExtensionProperties(*physical_device, NULL, &extension_count, available_extensions);

    uint32_t matching_ext_count = 0;

    for(size_t i = 0; i < extension_count; i++) {
        for(size_t j = 0; j < device->device_ext_size; j++) {
            if (strcmp(available_extensions[i].extensionName, device->device_extensions[j]) == 0) {
                matching_ext_count++;
            }
        }
    }

    return matching_ext_count == device->device_ext_size;
}
static uint8_t device_check_suitable(const t_Device *device, const VkPhysicalDevice *physical_device) {
    const t_QueueFamilyIndices indices = vk_find_queue_families(&device->surface, physical_device);

    //check swap chain support

    t_SwapChainSupportDetails swap_details = {};
    swap_chain_query_support_details(&swap_details, &device->surface, physical_device);

    const uint8_t swap_chain_adequate = swap_details.formats && swap_details.present_modes;


    swap_chain_free_support(&swap_details);

    VkPhysicalDeviceFeatures supported_features;
    vkGetPhysicalDeviceFeatures(*physical_device, &supported_features);

    return is_complete(&indices) && device_check_extension_support(device, physical_device) && swap_chain_adequate && supported_features.samplerAnisotropy;
}
static VkSampleCountFlagBits get_max_usable_sample_count(VkPhysicalDevice *physical_device) {
    VkPhysicalDeviceProperties physical_device_properties;
    vkGetPhysicalDeviceProperties(*physical_device, &physical_device_properties);

    const VkSampleCountFlags counts = physical_device_properties.limits.framebufferColorSampleCounts & physical_device_properties.limits.framebufferDepthSampleCounts;
    if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
    if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
    if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
    if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
    if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
    if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

    return VK_SAMPLE_COUNT_1_BIT;
}

static void device_pick_physical_device(t_Device *device, const VkInstance *instance) {
    device->physical_device = VK_NULL_HANDLE;

    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(*instance, &device_count, NULL);

    if (device_count == 0) {
        printf("Failed to find device with vulkan support!\n");
        exit(-1);
    }

    VkPhysicalDevice devices[device_count];
    vkEnumeratePhysicalDevices(*instance, &device_count, devices);

    for(size_t i = 0; i < device_count; i++) {
        if(device_check_suitable(device, &devices[i])) {
            device->physical_device = devices[i];
            device->msaa_samples = get_max_usable_sample_count(&devices[i]);
            break;
        }
    }
    if(device->physical_device == VK_NULL_HANDLE) {
        printf("Failed to find suitable device!\n");
        exit(-1);
    }
}
static void device_create_logical(t_Device *device, const t_QueueFamilyIndices *indices, const uint8_t has_validation_layer_support, const char **validation_layers, uint32_t validation_size) {
    if (!indices->graphics_family.has_value || !indices->present_family.has_value) {
        printf("Error: Required queue families are not available!\n");
        exit(-1);
    }

    const uint32_t unique_queue_families[2] = {indices->graphics_family.value, indices->present_family.value};
    const uint32_t queue_size = (indices->graphics_family.value != indices->present_family.value) ? 2 : 1;

    float queue_priority = 1.0f;

    VkDeviceQueueCreateInfo queue_create_infos[queue_size];
    const VkDeviceQueueCreateInfo base_queue_create_info = {
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
    VkPhysicalDeviceFeatures device_features = {
        .samplerAnisotropy = VK_TRUE
    };

    VkDeviceCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pQueueCreateInfos = queue_create_infos,
        .queueCreateInfoCount = queue_size,
        .pEnabledFeatures = &device_features,
        .enabledExtensionCount = device->device_ext_size,
        .ppEnabledExtensionNames = device->device_extensions
    };

    if (has_validation_layer_support) {
        create_info.enabledLayerCount = (validation_size);
        create_info.ppEnabledLayerNames = validation_layers;
    } else {
        create_info.enabledLayerCount = 0;
        create_info.ppEnabledLayerNames = NULL;
    }

    if (vkCreateDevice(device->physical_device, &create_info, NULL, &device->instance) != VK_SUCCESS) {
        printf("\nError: Failed to create Vulkan logical device!");
        exit(-1);
    }

    // Retrieve queue handles
    vkGetDeviceQueue(device->instance, indices->graphics_family.value, 0, &device->graphics_queue);
    vkGetDeviceQueue(device->instance, indices->present_family.value, 0, &device->present_queue);

    printf("\nLogical device created successfully.");
}
void device_create_surface(const VkInstance *instance, GLFWwindow *window, VkSurfaceKHR *surface) {
    if (glfwCreateWindowSurface(*instance, window, NULL, surface) != VK_SUCCESS) {
        printf("\nFailed to create window surface! \n");
    }
}
static void device_ext_init(t_Device *device) {
    const char *default_device_extensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };
    device->device_ext_size = 1;
    device->device_extensions = malloc(device->device_ext_size * sizeof(char *));
    for (size_t i = 0; i < device->device_ext_size; i++) {
        device->device_extensions[i] = default_device_extensions[i];
    }
}
t_Device device_init(const VkInstance *instance, GLFWwindow *window, t_QueueFamilyIndices *indices,
    const uint8_t has_validation_support, const char** validation_layers, const uint32_t validation_size) {

    t_Device device;
    device.msaa_samples = VK_SAMPLE_COUNT_1_BIT;
    device_ext_init(&device);
    device_create_surface(instance, window, &device.surface);
    device_pick_physical_device(&device, instance);

    *indices = vk_find_queue_families(&device.surface, &device.physical_device);
    device_create_logical(&device, indices, has_validation_support, validation_layers, validation_size);

    return device;
}
void device_destroy(const t_Device *device, const VkInstance *instance) {
    vkDestroyDevice(device->instance, NULL);
    vkDestroySurfaceKHR(*instance, device->surface, NULL);
    free(device->device_extensions);
}
