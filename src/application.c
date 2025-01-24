#include "application.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vk_debug.h"

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

static void app_create_command_pool(t_Application *app) {
    const t_QueueFamilyIndices queue_family_indices = app_find_queue_families(&app->device.surface, &app->device.physical_device);

    const VkCommandPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queue_family_indices.graphics_family.value
    };

    if (vkCreateCommandPool(app->device.instance, &pool_info, NULL, &app->command_pool) != VK_SUCCESS) {
        printf("Failed to create command pool! \n");
    }
}
static void app_create_command_buffers(t_Application *app) {
    app->command_buffers = (VkCommandBuffer*)malloc(sizeof(VkCommandBuffer) * MAX_FRAMES_IN_FLIGHT);
    const VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = app->command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = MAX_FRAMES_IN_FLIGHT,
    };

    if (vkAllocateCommandBuffers(app->device.instance, &alloc_info, app->command_buffers) != VK_SUCCESS) {
        printf("Failed to create command buffer! \n");
    }

}
static void app_record_command_buffer(const t_Application *app, const VkCommandBuffer *command_buffer, const uint32_t image_index) {
    const VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = 0, // Optional
        .pInheritanceInfo = NULL, // Optional
    };
    if (vkBeginCommandBuffer(*command_buffer, &begin_info) != VK_SUCCESS) {
        printf("Failed to begin command buffer! \n");
    }
    VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};

    const VkRenderPassBeginInfo render_pass_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = app->pipeline.render_pass,
        .framebuffer = app->swap_chain.framebuffers[image_index],
        .renderArea.offset = {0, 0},
        .renderArea.extent = app->swap_chain.extent,
        .clearValueCount = 1,
        .pClearValues = &clear_color
    };

    vkCmdBeginRenderPass(*command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(*command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, app->pipeline.graphics_pipeline);

    VkBuffer vertexBuffers[] = {app->vertex_buffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(*command_buffer, 0, 1, vertexBuffers, offsets);

    vkCmdDraw(*command_buffer, app->vertice_size, 1, 0, 0);


    const VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = (float)(app->swap_chain.extent.width),
        .height = (float)(app->swap_chain.extent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };
    vkCmdSetViewport(*command_buffer, 0, 1, &viewport);

    const VkRect2D scissor = {
    .offset = {0, 0},
    .extent = app->swap_chain.extent,
    };
    vkCmdSetScissor(*command_buffer, 0, 1, &scissor);

    vkCmdDraw(*command_buffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(*command_buffer);

    if (vkEndCommandBuffer(*command_buffer) != VK_SUCCESS) {
        printf("Failed to end command buffer! \n");
    }

}
static void app_create_sync_objects(t_Application *app) {
    app->image_available_semaphores = (VkSemaphore*)malloc(sizeof(VkSemaphore) * MAX_FRAMES_IN_FLIGHT);
    app->render_finished_semaphores = (VkSemaphore*)malloc(sizeof(VkSemaphore) * MAX_FRAMES_IN_FLIGHT);
    app->in_flight_fences = (VkFence*)malloc(sizeof(VkFence) * MAX_FRAMES_IN_FLIGHT);

    const VkSemaphoreCreateInfo semaphore_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };
    const VkFenceCreateInfo fence_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(app->device.instance, &semaphore_info, NULL, &app->image_available_semaphores[i]) != VK_SUCCESS ||
        vkCreateSemaphore(app->device.instance, &semaphore_info, NULL, &app->render_finished_semaphores[i]) != VK_SUCCESS ||
        vkCreateFence(app->device.instance, &fence_info, NULL, &app->in_flight_fences[i]) != VK_SUCCESS) {
            printf("Failed to create semaphores! \n");
        }
    }


}
static void app_create_vertex_buffer(t_Application *app) {
    VkBufferCreateInfo buffer_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = sizeof(app->vertices[0]) * app->vertice_size,
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };
    if(vkCreateBuffer(app->device.instance, &buffer_info, NULL, &app->vertex_buffer) != VK_SUCCESS) {
        printf("Failed to create a vertex buffer! \n");
    }
    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(app->device.instance, app->vertex_buffer, &mem_requirements);

    VkMemoryAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = mem_requirements.size,
        .memoryTypeIndex = find_memory_type(mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, app->device.physical_device)
    };
    if(vkAllocateMemory(app->device.instance, &alloc_info, NULL, &app->vertex_buffer_memory) != VK_SUCCESS) {
        printf("Failed to allocate vertex buffer memory! \n");
    }
    vkBindBufferMemory(app->device.instance, app->vertex_buffer, app->vertex_buffer_memory, 0);

    void* data;
    vkMapMemory(app->device.instance, app->vertex_buffer_memory, 0, buffer_info.size, 0, &data);
    memcpy(data, app->vertices, buffer_info.size);
    vkUnmapMemory(app->device.instance, app->vertex_buffer_memory);
}
static void app_vulkan_init(t_Application *app) {
    debug_print_available_extensions();
    app_create_vulkan_inst(app);
    debug_setup_messenger(ENABLE_VALIDATION_LAYERS, &app->vk_instance, &app->debug_messenger);

    app->device = device_init(&app->vk_instance, app->window, &app->indices,
        val_check_layer_support(app->validation.validation_layers, app->validation.validation_size), app->validation.validation_layers, app->validation.validation_size);

    //*****SWAP CHAIN CREATION*****
    app->indices = app_find_queue_families(&app->device.surface, &app->device.physical_device);
    app->swap_chain = swap_chain_create(&app->device.surface, &app->device.instance, &app->device.physical_device, app->window, &app->indices);

    swap_chain_create_image_views(&app->swap_chain, &app->device.instance);

    //create pipeline
    app->pipeline = pipeline_init(&app->device.instance, &app->swap_chain.image_format, &app->swap_chain.extent);


    swap_chain_create_frame_buffers(&app->swap_chain, &app->device.instance, &app->pipeline.render_pass);
    //*****SWAP CHAIN CREATION*****

    app_create_command_pool(app);
    app_create_vertex_buffer(app);
    app_create_command_buffers(app);
    app_create_sync_objects(app);
}

static void framebuffer_resize_callback(GLFWwindow *window, int width, int height) {
    t_Application *app = (t_Application *)glfwGetWindowUserPointer(window);

    app->framebuffer_resized = 1;

}
void app_init(t_Application *app) {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    app->window = glfwCreateWindow(1280,720, "VulkanDemo", NULL, NULL);
    glfwSetWindowUserPointer(app->window, app);
    glfwSetFramebufferSizeCallback(app->window, framebuffer_resize_callback);

    app->current_frame = 0;
    app->framebuffer_resized = 0;

    //initialise vertices

    //todo move vertex buffer to own file
    app->vertice_size = 3;
    app->vertices = (t_Vertex*)malloc(sizeof(t_Vertex) * app->vertice_size);
    app->vertices[0] = (t_Vertex){{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}};
    app->vertices[1] = (t_Vertex){{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}};
    app->vertices[2] = (t_Vertex){{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}};

    //todo maybe pass macro enable here
    app->validation = val_init();

    app_vulkan_init(app);
}

static void app_draw_frame(t_Application *app) {
    vkWaitForFences(app->device.instance, 1, &app->in_flight_fences[app->current_frame], VK_TRUE, UINT64_MAX);

    uint32_t image_index;
    VkResult result = vkAcquireNextImageKHR(app->device.instance, app->swap_chain.instance, UINT64_MAX, app->image_available_semaphores[app->current_frame], VK_NULL_HANDLE, &image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || app->framebuffer_resized) {
        app->framebuffer_resized = 0;
        swap_chain_recreate(&app->swap_chain, &app->device.surface, &app->device.instance, &app->device.physical_device, app->window, &app->indices);
        return;
    } else if (result != VK_SUCCESS) {
        printf("Failed to acquire swap chain! \n");
    }
    vkResetFences(app->device.instance, 1, &app->in_flight_fences[app->current_frame]);

    vkResetCommandBuffer(app->command_buffers[app->current_frame], 0);

    app_record_command_buffer(app, &app->command_buffers[app->current_frame], image_index);
    const VkSemaphore wait_semaphores[] = {app->image_available_semaphores[app->current_frame]};
    const VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    const VkSemaphore signal_semaphores[] = {app->render_finished_semaphores[app->current_frame]};

    const VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = wait_semaphores,
        .pWaitDstStageMask = wait_stages,
        .commandBufferCount = 1,
        .pCommandBuffers = &app->command_buffers[app->current_frame],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = signal_semaphores,
    };

    if (vkQueueSubmit(app->device.graphics_queue, 1, &submit_info, app->in_flight_fences[app->current_frame]) != VK_SUCCESS) {
        printf("Failed to submit draw buffer! \n");
    }

    const VkSwapchainKHR swap_chains[] = {app->swap_chain.instance};

    const VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signal_semaphores,
        .swapchainCount = 1,
        .pSwapchains = swap_chains,
        .pImageIndices = &image_index,
    };

    result = vkQueuePresentKHR(app->device.present_queue, &present_info);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        swap_chain_recreate(&app->swap_chain, &app->device.surface, &app->device.instance, &app->device.physical_device, app->window, &app->indices);
    } else if (result != VK_SUCCESS) {
        printf("Failed to present swap chain image!\n");
    }

    app->current_frame = (app->current_frame + 1) % MAX_FRAMES_IN_FLIGHT;

}
void app_run(t_Application *app) {
    while(!glfwWindowShouldClose(app->window)) {
        glfwPollEvents();
        app_draw_frame(app);
    }
    vkDeviceWaitIdle(app->device.instance);
}
void app_end(const t_Application *app) {
    swap_chain_cleanup(&app->swap_chain, &app->device.instance);

    vkDestroyBuffer(app->device.instance, app->vertex_buffer, NULL);
    vkFreeMemory(app->device.instance, app->vertex_buffer_memory, NULL);
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(app->device.instance, app->render_finished_semaphores[i], NULL);
        vkDestroySemaphore(app->device.instance, app->image_available_semaphores[i], NULL);
        vkDestroyFence(app->device.instance, app->in_flight_fences[i], NULL);
    }
    //todo move renderer to own file
    free(app->command_buffers);
    free(app->image_available_semaphores);
    free(app->render_finished_semaphores);
    free(app->in_flight_fences);
    free(app->vertices);
    vkDestroyCommandPool(app->device.instance, app->command_pool, NULL);

    pipeline_destroy(&app->pipeline, &app->device.instance);
    val_cleanup(&app->validation, &app->vk_instance, &app->debug_messenger);
    device_destroy(&app->device, &app->vk_instance);

    vkDestroyInstance(app->vk_instance, NULL);
    glfwDestroyWindow(app->window);
    glfwTerminate();
}