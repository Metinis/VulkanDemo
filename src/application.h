#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <stdio.h>
#include <cglm/mat4.h>
#include <vulkan/vulkan_core.h>

#include "texture.h"
#include "vk_swap_chain.h"
#include "utils.h"
#include "vk_device.h"
#include "vk_pipeline.h"
#include "vk_renderer.h"
#include "vk_validation.h"
#include "vk_buffers.h"
#include "vk_color.h"
#include "vk_depth.h"
#include "vk_descriptor.h"


#ifdef NDEBUG
    #define ENABLE_VALIDATION_LAYERS 0
#else
    #define ENABLE_VALIDATION_LAYERS 1
#endif

#define TEXTURE_PATH "../resources/textures/viking_room.png"
#define MODEL_PATH "../resources/models/viking_room.obj"

typedef struct GLFWwindow GLFWwindow;


typedef struct Application {
    GLFWwindow *window;
    VkInstance vk_instance;
    VkDebugUtilsMessengerEXT debug_messenger;
    t_Validation validation;
    t_Device device;
    t_QueueFamilyIndices indices;
    t_SwapChain swap_chain;
    t_DescriptorData descriptor;
    t_Pipeline pipeline;
    t_Renderer renderer;
    t_VertexBuffer vertex_buffer;
    t_IndexBuffer index_buffer;
    t_UniformBufferData ubo_data;
    t_Texture texture;
    t_DepthData depth_data;
    t_ColorData color_data;
}t_Application;

t_QueueFamilyIndices app_find_queue_families(const VkSurfaceKHR *surface, const VkPhysicalDevice *device);
void app_init(t_Application *app);
void app_run(t_Application *app);
void app_end(const t_Application *app);