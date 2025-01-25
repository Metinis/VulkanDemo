#pragma once
#include <stdint.h>
#include <vulkan/vulkan_core.h>

#include "utils.h"

typedef struct VertexBuffer {
    uint32_t size;
    t_Vertex *vertices;
    VkBuffer instance;
    VkDeviceMemory buffer_memory;
}t_VertexBuffer;
VkVertexInputBindingDescription get_binding_description_vertex();
void get_attribute_descriptions_vertex(VkVertexInputAttributeDescription* attribute_descriptions);
t_VertexBuffer vb_init(const VkDevice *device, const VkPhysicalDevice *physical_device);
void vb_cleanup(const t_VertexBuffer *vertex_buffer, const VkDevice *device);
