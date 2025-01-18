#include "application.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vulkan_debug.h"

VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR* capabilities, GLFWwindow* window) {
    VkExtent2D extent;

    if (capabilities->currentExtent.width != UINT32_MAX) {
        extent = capabilities->currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actual_extent = {
            (uint32_t)width,
            (uint32_t)height
        };

        actual_extent.width = (actual_extent.width < capabilities->minImageExtent.width)
                                ? capabilities->minImageExtent.width
                                : (actual_extent.width > capabilities->maxImageExtent.width)
                                    ? capabilities->maxImageExtent.width
                                    : actual_extent.width;

        actual_extent.height = (actual_extent.height < capabilities->minImageExtent.height)
                                ? capabilities->minImageExtent.height
                                : (actual_extent.height > capabilities->maxImageExtent.height)
                                    ? capabilities->maxImageExtent.height
                                    : actual_extent.height;

        extent = actual_extent;
    }

    return extent;
}

static VkPresentModeKHR choose_swap_present_mode(const VkPresentModeKHR* available_present_modes, const size_t available_present_size) {
    for(size_t i = 0; i < available_present_size; i++) {
        if(available_present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            return available_present_modes[i];
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}
static VkSurfaceFormatKHR choose_swap_surface_format(const VkSurfaceFormatKHR* available_formats, const size_t available_format_size) {
    for(size_t i = 0; i < available_format_size; i++) {
        if(available_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && available_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return available_formats[i];
        }
    }
    return available_formats[0];
}
static void query_swap_chain_support_details(SwapChainSupportDetails *details, const t_Application *app, const VkPhysicalDevice device) {

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, app->surface, &details->capabilities);

    uint32_t format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, app->surface, &format_count, nullptr);

    if (format_count != 0) {
        details->formats = (VkSurfaceFormatKHR *)malloc(format_count * sizeof(VkSurfaceFormatKHR));
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, app->surface, &format_count, details->formats);
    }
    else {
        details->formats = NULL;
    }
    details->format_size = format_count;


    uint32_t present_mode_count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, app->surface, &present_mode_count, nullptr);

    if (present_mode_count != 0) {
        details->present_modes = (VkPresentModeKHR *)malloc(present_mode_count * sizeof(VkPresentModeKHR));
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, app->surface, &present_mode_count, details->present_modes);
    }
    else {
        details->present_modes = NULL;
    }
    details->present_size = present_mode_count;


}
static void free_swap_chain_support(SwapChainSupportDetails *details) {
    if(details->formats) {
        free(details->formats);
        details->formats = NULL;
    }
    if(details->present_modes) {
        free(details->present_modes);
        details->present_modes = NULL;
    }
}


static uint8_t check_validation_layer_support(const char** validation_layers, const size_t validation_size) {
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
static uint8_t check_device_extension_support(const t_Application *app, const VkPhysicalDevice device) {
    //enumerate and find if they exist
    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

    VkExtensionProperties available_extensions[extension_count];
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_extensions);

    uint32_t matching_ext_count = 0;

    for(size_t i = 0; i < extension_count; i++) {
        for(size_t j = 0; j < app->device_ext_size; j++) {
            if (strcmp(available_extensions[i].extensionName, app->device_extensions[j]) == 0) {
                matching_ext_count++;
            }
        }
    }

    return matching_ext_count == app->device_ext_size;
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

    for(size_t i = 0; i < glfw_extensionCount; i++) {
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
static void app_enable_validation(const t_Application *app, VkInstanceCreateInfo* create_info) {

    if (check_validation_layer_support(app->validation_layers, app->validation_size)) {
        create_info->enabledLayerCount = (uint32_t)(app->validation_size);
        create_info->ppEnabledLayerNames = app->validation_layers;
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
        app_enable_validation(app, &create_info);

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
static QueueFamilyIndices find_queue_families(const t_Application *app, const VkPhysicalDevice device) {
    QueueFamilyIndices indices = {
        .graphics_family = { .value = 0, .has_value = 0 },
        .present_family = { .value = 0, .has_value = 0 }
    };

    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

    VkQueueFamilyProperties queue_families[queue_family_count];
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families);

    for(size_t i = 0; i < queue_family_count; i++) {
        if(queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphics_family.has_value = 1;
            indices.graphics_family.value = i;
        }
        VkBool32 present_support = 0;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, app->surface, &present_support);
        if (present_support) {
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
static void create_swap_chain(t_Application *app) {
    SwapChainSupportDetails swap_details = {};

    query_swap_chain_support_details(&swap_details, app, app->physical_device);

    const VkSurfaceFormatKHR surfaceFormat = choose_swap_surface_format(swap_details.formats, swap_details.format_size);
    const VkPresentModeKHR presentMode = choose_swap_present_mode(swap_details.present_modes, swap_details.present_size);
    const VkExtent2D extent = choose_swap_extent(&swap_details.capabilities, app->window);

    app->swap_chain_image_count = swap_details.capabilities.minImageCount + 1;

    if (swap_details.capabilities.maxImageCount > 0 && app->swap_chain_image_count > swap_details.capabilities.maxImageCount) {
        app->swap_chain_image_count = swap_details.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR create_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = app->surface,
        .minImageCount = app->swap_chain_image_count,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
        };

    QueueFamilyIndices indices = find_queue_families(app, app->physical_device);
    const uint32_t queueFamilyIndices[] = {indices.graphics_family.value, indices.present_family.value};

    if (indices.graphics_family.value != indices.present_family.value) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.queueFamilyIndexCount = 0;
        create_info.pQueueFamilyIndices = nullptr;
    }

    create_info.preTransform = swap_details.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = presentMode;
    create_info.clipped = VK_TRUE;

    create_info.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(app->device, &create_info, nullptr, &app->swap_chain) != VK_SUCCESS) {
        printf("\nfailed to create swap chain!");
        exit(-1);
    }

    vkGetSwapchainImagesKHR(app->device, app->swap_chain, &app->swap_chain_image_count, nullptr);
    app->swap_chain_images = (VkImage*)malloc(app->swap_chain_image_count * sizeof(VkImage));
    vkGetSwapchainImagesKHR(app->device, app->swap_chain, &app->swap_chain_image_count, app->swap_chain_images);

    app->swap_chain_image_format = surfaceFormat.format;
    app->swap_chain_extent = extent;

    free_swap_chain_support(&swap_details);

}
static uint8_t is_device_suitable(const t_Application *app, const VkPhysicalDevice device) {
    const QueueFamilyIndices indices = find_queue_families(app, device);

    //check swap chain support

    SwapChainSupportDetails swap_details = {};
    query_swap_chain_support_details(&swap_details, app, device);

    const uint8_t swap_chain_adequate = swap_details.formats && swap_details.present_modes;

    free_swap_chain_support(&swap_details);

    return is_complete(&indices) && check_device_extension_support(app, device) && swap_chain_adequate;
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

    for(size_t i = 0; i < device_count; i++) {
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
static void app_enable_validation_device(const t_Application *app, VkDeviceCreateInfo* create_info) {

    if (check_validation_layer_support(app->validation_layers, app->validation_size)) {
        create_info->enabledLayerCount = (uint32_t)(app->validation_size);
        create_info->ppEnabledLayerNames = app->validation_layers;
    } else {
        create_info->enabledLayerCount = 0;
        create_info->ppEnabledLayerNames = nullptr;
    }

}
static void create_logical_device(t_Application *app) {
    const QueueFamilyIndices indices = find_queue_families(app, app->physical_device);

    if (!indices.graphics_family.has_value || !indices.present_family.has_value) {
        printf("Error: Required queue families are not available!\n");
        exit(-1);
    }

    const uint32_t unique_queue_families[2] = {indices.graphics_family.value, indices.present_family.value};
    const uint32_t queue_size = (indices.graphics_family.value != indices.present_family.value) ? 2 : 1;

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
    VkPhysicalDeviceFeatures device_features = {};

    //TODO move this to be a variable outside the func
    const uint32_t device_ext_num = 1;
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

    app_enable_validation_device(app, &create_info);

    if (vkCreateDevice(app->physical_device, &create_info, NULL, &app->device) != VK_SUCCESS) {
        printf("\nError: Failed to create Vulkan logical device!");
        exit(-1);
    }

    // Retrieve queue handles
    vkGetDeviceQueue(app->device, indices.graphics_family.value, 0, &app->graphics_queue);
    vkGetDeviceQueue(app->device, indices.present_family.value, 0, &app->present_queue);

    printf("\nLogical device created successfully.");
}

static void create_surface(t_Application *app) {
    if (glfwCreateWindowSurface(app->vk_instance, app->window, nullptr, &app->surface) != VK_SUCCESS) {
        printf("\nFailed to create window surface! \n");
    }
}
static void create_image_views(t_Application *app) {
    app->swap_chain_image_views = (VkImageView*)malloc(app->swap_chain_image_count * sizeof(VkImageView));
    for(size_t i = 0; i < app->swap_chain_image_count; i++) {
        VkImageViewCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = app->swap_chain_images[i],
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = app->swap_chain_image_format,
        .components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
        .components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
        .components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
        .components.a = VK_COMPONENT_SWIZZLE_IDENTITY,
        .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .subresourceRange.baseMipLevel = 0,
        .subresourceRange.levelCount = 1,
        .subresourceRange.baseArrayLayer = 0,
        .subresourceRange.layerCount = 1
            };

        if (vkCreateImageView(app->device, &create_info, nullptr, &app->swap_chain_image_views[i]) != VK_SUCCESS) {
            printf("\nFailed to create image views!");
            exit(-1);
        }
    }
}
static void app_vulkan_init(t_Application *app) {
    debug_print_available_extensions();

    app_create_vulkan_inst(app);

    setup_debug_messenger(app);

    create_surface(app);

    pick_physical_device(app);

    create_logical_device(app);

    create_swap_chain(app);

    create_image_views(app);
}


void app_init(t_Application *app) {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    app->window = glfwCreateWindow(1280,720, "VulkanDemo", nullptr, nullptr);

    //setup validation list

    const char *default_device_extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    app->device_ext_size = 1;
    app->device_extensions = malloc(app->device_ext_size * sizeof(char *));
    for (size_t i = 0; i < app->device_ext_size; i++) {
        app->device_extensions[i] = default_device_extensions[i];
    }

    //setup device extension list

    const char *default_validation_layers[] = {"VK_LAYER_KHRONOS_validation"};
    app->validation_size = 1;
    app->validation_layers = malloc(app->validation_size * sizeof(char *));
    for (size_t i = 0; i < app->device_ext_size; i++) {
        app->validation_layers[i] = default_validation_layers[i];
    }

    app_vulkan_init(app);

    free(app->device_extensions);
    free(app->validation_layers);
}
void app_run(const t_Application *app) {
    while(!glfwWindowShouldClose(app->window)) {
        glfwPollEvents();
    }
}
void app_end(const t_Application *app) {

    for(size_t i = 0; i < app->swap_chain_image_count; i++) {
        vkDestroyImageView(app->device, app->swap_chain_image_views[i], nullptr);
    }
    free(app->swap_chain_image_views);

    if (app->enable_validation_layers) {
        destroy_debug_utils_messenger_ext(app->vk_instance, app->debug_messenger, nullptr);
    }
    vkDestroySwapchainKHR(app->device, app->swap_chain, nullptr);
    free(app->swap_chain_images);
    vkDestroyDevice(app->device, nullptr);
    vkDestroySurfaceKHR(app->vk_instance, app->surface, nullptr);
    vkDestroyInstance(app->vk_instance, nullptr);
    glfwDestroyWindow(app->window);
    glfwTerminate();
}
