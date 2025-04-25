#pragma once
#include <vulkan/vulkan_core.h>

#include "vk_descriptor.h"
#include "vk_pipeline.h"
#include "vk_renderer.h"
#include "vk_swap_chain.h"
#include "vk_validation.h"

#ifdef NDEBUG
    #define ENABLE_VALIDATION_LAYERS 0
#else
    #define ENABLE_VALIDATION_LAYERS 1
#endif

#define TEXTURE_PATH "../resources/textures/viking_room.png"
#define MODEL_PATH "../resources/models/viking_room.obj"

typedef struct VulkanCore {

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

} t_VulkanCore;

t_QueueFamilyIndices vk_find_queue_families(const VkSurfaceKHR *surface, const VkPhysicalDevice *device);
void vulkan_core_init(t_VulkanCore *vulkan_core, GLFWwindow *window);
void vulkan_handle_resize(t_VulkanCore *vulkan_core);
void vulkan_handle_draw_frame(t_VulkanCore *vulkan_core, GLFWwindow *window);
void vulkan_handle_exit(const t_VulkanCore *vulkan_core);