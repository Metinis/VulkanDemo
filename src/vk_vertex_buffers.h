#pragma once
#include <stdint.h>
#include <vulkan/vulkan_core.h>
#include "vk_device.h"
#include "utils.h"

typedef struct VertexBuffer {
    uint32_t size;
    t_Vertex *vertices;
    VkBuffer instance;
    VkDeviceMemory buffer_memory;
}t_VertexBuffer;
VkVertexInputBindingDescription get_binding_description_vertex();
void get_attribute_descriptions_vertex(VkVertexInputAttributeDescription* attribute_descriptions);
void buffer_create(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer *buffer,
    VkDeviceMemory *buffer_memory, const t_Device *device);
t_VertexBuffer vb_init(const t_Device *device, const VkCommandPool *command_pool);
void vb_cleanup(const t_VertexBuffer *vertex_buffer, const VkDevice *device);
