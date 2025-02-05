#pragma once
#include <stdint.h>
#include <vulkan/vulkan_core.h>
#include "utils.h"
#include "vk_pipeline.h"
#include "vk_swap_chain.h"
#include "vk_device.h"
#include "vk_buffers.h"
#include "vk_descriptor.h"

#define MAX_FRAMES_IN_FLIGHT 2

typedef struct Renderer {
    VkCommandPool command_pool;
    VkCommandBuffer* command_buffers;
    VkSemaphore* image_available_semaphores;
    VkSemaphore* render_finished_semaphores;
    VkFence* in_flight_fences;
    uint32_t current_frame;
    uint8_t framebuffer_resized;
}t_Renderer;
VkCommandBuffer begin_single_time_commands(const VkCommandPool *command_pool, const VkDevice *device);
void end_single_time_commands(const VkCommandPool *command_pool, VkCommandBuffer *command_buffer, const t_Device *device);
void renderer_record_command_buffer(const VkCommandBuffer *command_buffer, uint32_t image_index, const t_Pipeline *pipeline,
    const t_SwapChain *swap_chain, const t_VertexBuffer *vertex_buffer, const t_IndexBuffer *index_buffer, const t_DescriptorData *desc_data,
    uint32_t current_frame);
t_Renderer renderer_init(const t_QueueFamilyIndices *indices, const VkDevice *device);
void renderer_draw_frame(t_Renderer *renderer, const t_Device *device, const t_SwapChain *swap_chain, GLFWwindow *window,
    const t_QueueFamilyIndices *indices, const t_Pipeline *pipeline, const t_VertexBuffer *vertex_buffer, const t_IndexBuffer *index_buffer,
    const t_UniformBufferData *ubo_data, const t_DescriptorData *desc_data);
void renderer_cleanup(const t_Renderer *renderer, const VkDevice *device);
