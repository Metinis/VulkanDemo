#pragma once
#include <stdint.h>
#include <vulkan/vulkan_core.h>
#include "vk_device.h"
#include "utils.h"

typedef struct Buffer {
    VkBuffer instance;
    VkDeviceMemory buffer_memory;
}t_Buffer;
typedef struct VertexBuffer {
    uint32_t size;
    t_Vertex *vertices;
    t_Buffer buffer;
}t_VertexBuffer;
typedef struct IndexBuffer {
    uint32_t size;
    uint16_t *indices;
    t_Buffer buffer;
}t_IndexBuffer;
VkVertexInputBindingDescription get_binding_description_vertex();
void get_attribute_descriptions_vertex(VkVertexInputAttributeDescription* attribute_descriptions);
void buffer_create(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer *buffer,
    VkDeviceMemory *buffer_memory, const t_Device *device);
t_VertexBuffer buffer_vertex_init(const t_Device *device, const VkCommandPool *command_pool);
void buffer_vertex_cleanup(const t_VertexBuffer *vertex_buffer, const VkDevice *device);
t_IndexBuffer buffer_index_init(const t_Device *device, const VkCommandPool *command_pool);
void buffer_index_cleanup(const t_IndexBuffer *index_buffer, const VkDevice *device);