#include "application.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vk_debug.h"
#include "vk_renderer.h"

static void app_enable_extensions(VkInstanceCreateInfo* create_info, const char*** extensions_ptr) {

    uint32_t glfw_extensionCount = 0;
    const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extensionCount);

    create_info->enabledExtensionCount = glfw_extensionCount;
    create_info->ppEnabledExtensionNames = glfw_extensions;

    create_info->enabledLayerCount = 0;

    //****FOR MAC VK_INCOMPATIBLE DRIVER ****

    uint32_t extension_count = glfw_extensionCount + 1;
    if(ENABLE_VALIDATION_LAYERS) {
        extension_count++;
    }

    const char** required_extensions = (const char**)malloc(extension_count * sizeof(const char*));

    for(size_t i = 0; i < glfw_extensionCount; i++) {
        required_extensions[i] = glfw_extensions[i];
    }

    required_extensions[glfw_extensionCount] = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;

    if (ENABLE_VALIDATION_LAYERS) {
        required_extensions[glfw_extensionCount + 1] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    }

    create_info->flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

    create_info->enabledExtensionCount = extension_count;
    create_info->ppEnabledExtensionNames = required_extensions;

    //****FOR MAC VK_INCOMPATIBLE DRIVER ****

    *extensions_ptr = required_extensions;
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

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {};

    if(ENABLE_VALIDATION_LAYERS) {
        val_enable(&app->validation, &create_info);

        debug_populate_messenger_create_info(&debug_create_info);
        create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debug_create_info;

        debug_print_available_validation_layers();
    }

    const char** extensions = NULL;
    app_enable_extensions(&create_info, &extensions);

    const VkResult result = vkCreateInstance(&create_info, NULL, &app->vk_instance);

    if(ENABLE_VALIDATION_LAYERS) {
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
t_QueueFamilyIndices app_find_queue_families(const VkSurfaceKHR *surface, const VkPhysicalDevice *device) {
    t_QueueFamilyIndices indices = {
        .graphics_family = { .value = 0, .has_value = 0 },
        .present_family = { .value = 0, .has_value = 0 }
    };

    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(*device, &queue_family_count, NULL);

    VkQueueFamilyProperties queue_families[queue_family_count];
    vkGetPhysicalDeviceQueueFamilyProperties(*device, &queue_family_count, queue_families);

    for(size_t i = 0; i < queue_family_count; i++) {
        if(queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphics_family.has_value = 1;
            indices.graphics_family.value = i;
        }
        VkBool32 present_support = 0;
        vkGetPhysicalDeviceSurfaceSupportKHR(*device, i, *surface, &present_support);
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



static void app_vulkan_init(t_Application *app) {
    debug_print_available_extensions();
    app_create_vulkan_inst(app);
    debug_setup_messenger(ENABLE_VALIDATION_LAYERS, &app->vk_instance, &app->debug_messenger);

    app->device = device_init(&app->vk_instance, app->window, &app->indices,
        val_check_layer_support(app->validation.layers, app->validation.size), app->validation.layers, app->validation.size);

    //*****SWAP CHAIN CREATION*****
    app->indices = app_find_queue_families(&app->device.surface, &app->device.physical_device);
    app->swap_chain = swap_chain_create(&app->device.surface, &app->device.instance, &app->device.physical_device, app->window, &app->indices);

    swap_chain_create_image_views(&app->swap_chain, &app->device.instance);

    //descriptor_create_set_layout(&app->device.instance, &app->descriptor_set_layout);
    app->descriptor = descriptor_init(&app->device.instance);
    //create pipeline
    app->pipeline = pipeline_init(&app->device.instance, &app->swap_chain.image_format, &app->swap_chain.extent, &app->descriptor.set_layout);


    swap_chain_create_frame_buffers(&app->swap_chain, &app->device.instance, &app->pipeline.render_pass);
    //*****SWAP CHAIN CREATION*****

    app->renderer = renderer_init(&app->indices, &app->device.instance);
    //create texture image

    app->vertex_buffer = buffer_vertex_init(&app->device, &app->renderer.command_pool);
    app->index_buffer = buffer_index_init(&app->device, &app->renderer.command_pool);
    app->ubo_data = buffer_ubo_init(&app->device);

    app->texture = texture_init("../resources/textures/texture.jpg", &app->device, &app->renderer.command_pool);

    descriptor_populate(&app->device.instance, &app->descriptor, &app->ubo_data, &app->texture);


}

static void framebuffer_resize_callback(GLFWwindow *window, int width, int height) {
    t_Application *app = (t_Application *)glfwGetWindowUserPointer(window);

    app->renderer.framebuffer_resized = 1;

}
void app_init(t_Application *app) {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    app->window = glfwCreateWindow(800,600, "VulkanDemo", NULL, NULL);
    glfwSetWindowUserPointer(app->window, app);
    glfwSetFramebufferSizeCallback(app->window, framebuffer_resize_callback);

    app->validation = val_init(ENABLE_VALIDATION_LAYERS);

    app_vulkan_init(app);
}
void app_run(t_Application *app) {
    while(!glfwWindowShouldClose(app->window)) {
        glfwPollEvents();
        //if(!app->renderer.framebuffer_resized)
            renderer_draw_frame(&app->renderer, &app->device, &app->swap_chain, app->window, &app->indices,
            &app->pipeline, &app->vertex_buffer, &app->index_buffer, &app->ubo_data, &app->descriptor);
    }
    vkDeviceWaitIdle(app->device.instance);
}
void app_end(const t_Application *app) {
    swap_chain_cleanup(&app->swap_chain, &app->device.instance);
    texture_cleanup(&app->device.instance, &app->texture);

    buffer_ubo_cleanup(&app->device.instance, &app->ubo_data);
    descriptor_cleanup(&app->device.instance, &app->descriptor);

    buffer_vertex_cleanup(&app->vertex_buffer, &app->device.instance);
    buffer_index_cleanup(&app->index_buffer, &app->device.instance);

    renderer_cleanup(&app->renderer, &app->device.instance);

    pipeline_destroy(&app->pipeline, &app->device.instance);
    val_cleanup(&app->validation, &app->vk_instance, &app->debug_messenger);
    device_destroy(&app->device, &app->vk_instance);

    vkDestroyInstance(app->vk_instance, NULL);
    glfwDestroyWindow(app->window);
    glfwTerminate();
}