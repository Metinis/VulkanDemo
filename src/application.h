#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <stdio.h>
#include <cglm/mat4.h>
#include <vulkan/vulkan_core.h>
#include "swap_chain.h"
#include "utils.h"
#include "vulkan_device.h"
#include "vulkan_pipeline.h"

#define MAX_FRAMES_IN_FLIGHT 2


typedef struct GLFWwindow GLFWwindow;

typedef struct Application {
    GLFWwindow *window;
    VkInstance vk_instance;
    VkDebugUtilsMessengerEXT debug_messenger;
    uint8_t enable_validation_layers;
    const char **validation_layers;
    uint32_t validation_size;

    t_Device device;
    t_QueueFamilyIndices indices;
    t_SwapChain swap_chain;
    t_Pipeline pipeline;
    VkCommandPool command_pool;
    VkCommandBuffer* command_buffers;
    VkSemaphore* image_available_semaphores;
    VkSemaphore* render_finished_semaphores;
    VkFence* in_flight_fences;
    uint32_t current_frame;
    uint8_t framebuffer_resized;
    uint32_t vertice_size;
    t_Vertex *vertices;
    VkBuffer vertex_buffer;
    VkDeviceMemory vertex_buffer_memory;

}t_Application;

t_QueueFamilyIndices app_find_queue_families(const VkSurfaceKHR *surface, const VkPhysicalDevice *device);
void app_init(t_Application *app);
void app_run(t_Application *app);
void app_end(const t_Application *app);