#include "vk_core.h"

#include <stdio.h>

#include "vk_debug.h"
#include "../model.h"

static void vk_enable_extensions(VkInstanceCreateInfo* create_info, const char*** extensions_ptr) {

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


static void vk_create_vulkan_inst(t_VulkanCore *app) {
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
    vk_enable_extensions(&create_info, &extensions);

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
t_QueueFamilyIndices vk_find_queue_families(const VkSurfaceKHR *surface, const VkPhysicalDevice *device) {
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



void vulkan_core_init(t_VulkanCore *vulkan_core, GLFWwindow *window) {

    vulkan_core->validation = val_init(ENABLE_VALIDATION_LAYERS);

    debug_print_available_extensions();
    vk_create_vulkan_inst(vulkan_core);
    debug_setup_messenger(ENABLE_VALIDATION_LAYERS, &vulkan_core->vk_instance, &vulkan_core->debug_messenger);

    vulkan_core->device = device_init(&vulkan_core->vk_instance, window, &vulkan_core->indices,
        val_check_layer_support(vulkan_core->validation.layers, vulkan_core->validation.size), vulkan_core->validation.layers, vulkan_core->validation.size);

    //*****SWAP CHAIN CREATION*****
    vulkan_core->indices = vk_find_queue_families(&vulkan_core->device.surface, &vulkan_core->device.physical_device);
    vulkan_core->swap_chain = swap_chain_create(&vulkan_core->device.surface, &vulkan_core->device.instance, &vulkan_core->device.physical_device, window, &vulkan_core->indices);

    swap_chain_create_image_views(&vulkan_core->swap_chain, &vulkan_core->device.instance);

    //descriptor_create_set_layout(&app->device.instance, &app->descriptor_set_layout);
    vulkan_core->descriptor = descriptor_init(&vulkan_core->device.instance);
    //create pipeline
    vulkan_core->pipeline = pipeline_init(&vulkan_core->device, &vulkan_core->swap_chain.image_format, &vulkan_core->swap_chain.extent, &vulkan_core->descriptor.set_layout);

    vulkan_core->color_data = color_data_init(&vulkan_core->swap_chain.image_format, &vulkan_core->device, vulkan_core->swap_chain.extent);
    vulkan_core->depth_data = depth_init(&vulkan_core->device, vulkan_core->swap_chain.extent);

    swap_chain_create_frame_buffers(&vulkan_core->swap_chain, &vulkan_core->device.instance, &vulkan_core->pipeline.render_pass, &vulkan_core->depth_data.depth_image_view,
        &vulkan_core->color_data.color_image_view);
    //*****SWAP CHAIN CREATION*****

    vulkan_core->renderer = renderer_init(&vulkan_core->indices, &vulkan_core->device.instance);
    //create texture image
    load_model(MODEL_PATH, &vulkan_core->vertex_buffer.vertices, &vulkan_core->vertex_buffer.size, &vulkan_core->index_buffer.indices, &vulkan_core->index_buffer.size);

    buffer_vertex_create(&vulkan_core->vertex_buffer, &vulkan_core->device, &vulkan_core->renderer.command_pool);
    buffer_index_create(&vulkan_core->index_buffer, &vulkan_core->device, &vulkan_core->renderer.command_pool);
    vulkan_core->ubo_data = buffer_ubo_init(&vulkan_core->device);

    vulkan_core->texture = texture_init(TEXTURE_PATH, &vulkan_core->device, &vulkan_core->renderer.command_pool);

    descriptor_populate(&vulkan_core->device.instance, &vulkan_core->descriptor, &vulkan_core->ubo_data, &vulkan_core->texture);
}
void vulkan_handle_draw_frame(t_VulkanCore *vulkan_core, GLFWwindow *window) {
    if(vulkan_core->renderer.framebuffer_resized) {
            swap_chain_recreate(&vulkan_core->swap_chain, &vulkan_core->depth_data, &vulkan_core->color_data, &vulkan_core->device.surface, &vulkan_core->device, window,
                &vulkan_core->indices, &vulkan_core->pipeline.render_pass);
            vulkan_core->renderer.framebuffer_resized = 0;
        }
    renderer_draw_frame(&vulkan_core->renderer, &vulkan_core->device, &vulkan_core->swap_chain, window, &vulkan_core->indices,
    &vulkan_core->pipeline, &vulkan_core->vertex_buffer, &vulkan_core->index_buffer, &vulkan_core->ubo_data, &vulkan_core->descriptor, &vulkan_core->depth_data, &vulkan_core->color_data);
}
void vulkan_handle_resize(t_VulkanCore *vulkan_core) {
    vulkan_core->renderer.framebuffer_resized = 1;
}
void vulkan_handle_exit(const t_VulkanCore *vulkan_core) {
    vkDeviceWaitIdle(vulkan_core->device.instance);
    swap_chain_cleanup(&vulkan_core->swap_chain, &vulkan_core->depth_data, &vulkan_core->color_data, &vulkan_core->device.instance);
    texture_cleanup(&vulkan_core->device.instance, &vulkan_core->texture);

    buffer_ubo_cleanup(&vulkan_core->device.instance, &vulkan_core->ubo_data);
    descriptor_cleanup(&vulkan_core->device.instance, &vulkan_core->descriptor);

    buffer_vertex_cleanup(&vulkan_core->vertex_buffer, &vulkan_core->device.instance);
    buffer_index_cleanup(&vulkan_core->index_buffer, &vulkan_core->device.instance);

    renderer_cleanup(&vulkan_core->renderer, &vulkan_core->device.instance);

    pipeline_destroy(&vulkan_core->pipeline, &vulkan_core->device.instance);
    val_cleanup(&vulkan_core->validation, &vulkan_core->vk_instance, &vulkan_core->debug_messenger);
    device_destroy(&vulkan_core->device, &vulkan_core->vk_instance);

    vkDestroyInstance(vulkan_core->vk_instance, NULL);
}
