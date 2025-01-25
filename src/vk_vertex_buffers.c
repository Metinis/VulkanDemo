#include "vk_vertex_buffers.h"

#include <stdio.h>
#include <string.h>

VkVertexInputBindingDescription get_binding_description_vertex() {
    static VkVertexInputBindingDescription binding_description = {
        .binding = 0,
        .stride = sizeof(t_Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };
    return binding_description;
}
void get_attribute_descriptions_vertex(VkVertexInputAttributeDescription* attribute_descriptions) {
    attribute_descriptions[0] = (VkVertexInputAttributeDescription){
        .binding = 0,
        .location = 0,
        .format = VK_FORMAT_R32G32_SFLOAT, // vec2 (pos)
        .offset = offsetof(t_Vertex, pos)
    };

    attribute_descriptions[1] = (VkVertexInputAttributeDescription){
        .binding = 0,
        .location = 1,
        .format = VK_FORMAT_R32G32B32_SFLOAT, // vec3 (color)
        .offset = offsetof(t_Vertex, color)
    };
}
static uint32_t find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties, VkPhysicalDevice physical_device) {
    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_properties);

    for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
        if (type_filter & (1 << i) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    printf("failed to find suitable memory type! \n");
    return -1;
}

void vb_create(t_VertexBuffer *vertex_buffer, const VkDevice *device, const VkPhysicalDevice *physical_device) {
    const VkBufferCreateInfo buffer_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = sizeof(vertex_buffer->vertices[0]) * vertex_buffer->size,
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };
    if(vkCreateBuffer(*device, &buffer_info, NULL, &vertex_buffer->instance) != VK_SUCCESS) {
        printf("Failed to create a vertex buffer! \n");
    }
    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(*device, vertex_buffer->instance, &mem_requirements);

    const VkMemoryAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = mem_requirements.size,
        .memoryTypeIndex = find_memory_type(mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, *physical_device)
    };
    if(vkAllocateMemory(*device, &alloc_info, NULL, &vertex_buffer->buffer_memory) != VK_SUCCESS) {
        printf("Failed to allocate vertex buffer memory! \n");
    }
    vkBindBufferMemory(*device, vertex_buffer->instance, vertex_buffer->buffer_memory, 0);

    void* data;
    vkMapMemory(*device, vertex_buffer->buffer_memory, 0, buffer_info.size, 0, &data);
    memcpy(data, vertex_buffer->vertices, buffer_info.size);
    vkUnmapMemory(*device, vertex_buffer->buffer_memory);
}
t_VertexBuffer vb_init(const VkDevice *device, const VkPhysicalDevice *physical_device) {
    t_VertexBuffer vertex_buffer;
    vertex_buffer.size = 3;
    vertex_buffer.vertices = (t_Vertex*)malloc(sizeof(t_Vertex) * vertex_buffer.size);
    vertex_buffer.vertices[0] = (t_Vertex){{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}};
    vertex_buffer.vertices[1] = (t_Vertex){{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}};
    vertex_buffer.vertices[2] = (t_Vertex){{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}};
    vb_create(&vertex_buffer, device, physical_device);
    return vertex_buffer;
}
void vb_cleanup(const t_VertexBuffer *vertex_buffer, const VkDevice *device) {
    vkDestroyBuffer(*device, vertex_buffer->instance, NULL);
    vkFreeMemory(*device, vertex_buffer->buffer_memory, NULL);

    free(vertex_buffer->vertices);
}
