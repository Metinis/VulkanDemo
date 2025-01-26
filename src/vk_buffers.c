#include "vk_buffers.h"

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
static uint32_t find_memory_type(const uint32_t type_filter, const VkMemoryPropertyFlags properties, const VkPhysicalDevice physical_device) {
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
static void buffer_copy(const VkCommandPool *command_pool, const t_Device *device, const VkDeviceSize size, const VkBuffer *src_buffer, const VkBuffer *dst_buffer) {
    //create command buffer
    const VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandPool = *command_pool,
        .commandBufferCount = 1
    };

    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(device->instance, &alloc_info, &command_buffer);

    //record copy to command buffer

    const VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };

    vkBeginCommandBuffer(command_buffer, &begin_info);

    const VkBufferCopy copy_region = {
        .srcOffset = 0, // Optional
        .dstOffset = 0, // Optional
        .size = size
    };

    vkCmdCopyBuffer(command_buffer, *src_buffer, *dst_buffer, 1, &copy_region);

    vkEndCommandBuffer(command_buffer);

    //now execute command buffer

    const VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &command_buffer
    };

    vkQueueSubmit(device->graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(device->graphics_queue);

    //cleanup command buffer

    vkFreeCommandBuffers(device->instance, *command_pool, 1, &command_buffer);

}
void buffer_create(const VkDeviceSize size, const VkBufferUsageFlags usage, const VkMemoryPropertyFlags properties, VkBuffer *buffer,
    VkDeviceMemory *buffer_memory, const t_Device *device) {
    const VkBufferCreateInfo buffer_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };
    if(vkCreateBuffer(device->instance, &buffer_info, NULL, buffer) != VK_SUCCESS) {
        printf("Failed to create a vertex buffer! \n");
    }
    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(device->instance, *buffer, &mem_requirements);

    const VkMemoryAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = mem_requirements.size,
        .memoryTypeIndex = find_memory_type(mem_requirements.memoryTypeBits, properties, device->physical_device)
    };
    if(vkAllocateMemory(device->instance, &alloc_info, NULL, buffer_memory) != VK_SUCCESS) {
        printf("Failed to allocate vertex buffer memory! \n");
    }
    vkBindBufferMemory(device->instance, *buffer, *buffer_memory, 0);
}
static void buffer_vertex_create(t_VertexBuffer *vertex_buffer, const t_Device *device, const VkCommandPool *command_pool) {
    const VkDeviceSize buffer_size = sizeof(vertex_buffer->vertices[0]) * vertex_buffer->size;

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_mem;
    buffer_create(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &staging_buffer, &staging_buffer_mem, device);

    void* data;
    vkMapMemory(device->instance, staging_buffer_mem, 0, buffer_size, 0, &data);
    memcpy(data, vertex_buffer->vertices, buffer_size);
    vkUnmapMemory(device->instance, staging_buffer_mem);

    buffer_create(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &vertex_buffer->buffer.instance, &vertex_buffer->buffer.buffer_memory, device);
    buffer_copy(command_pool, device, buffer_size, &staging_buffer, &vertex_buffer->buffer.instance);

    //cleanup
    vkDestroyBuffer(device->instance, staging_buffer, NULL);
    vkFreeMemory(device->instance, staging_buffer_mem, NULL);
}
t_VertexBuffer buffer_vertex_init(const t_Device *device, const VkCommandPool *command_pool) {
    t_VertexBuffer vertex_buffer;
    vertex_buffer.size = 4;
    vertex_buffer.vertices = (t_Vertex*)malloc(sizeof(t_Vertex) * vertex_buffer.size);
    const t_Vertex predefined_vertices[] = {
        {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
    };
    for (size_t i = 0; i < vertex_buffer.size; ++i) {
        vertex_buffer.vertices[i] = predefined_vertices[i];
    }

    buffer_vertex_create(&vertex_buffer, device, command_pool);
    return vertex_buffer;
}
void buffer_vertex_cleanup(const t_VertexBuffer *vertex_buffer, const VkDevice *device) {
    vkDestroyBuffer(*device, vertex_buffer->buffer.instance, NULL);
    vkFreeMemory(*device, vertex_buffer->buffer.buffer_memory, NULL);

    free(vertex_buffer->vertices);
}
static void buffer_index_create(t_IndexBuffer *index_buffer, const t_Device *device, const VkCommandPool *command_pool) {
    const VkDeviceSize buffer_size = sizeof(index_buffer->indices[0]) * index_buffer->size;

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_mem;
    buffer_create(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &staging_buffer, &staging_buffer_mem, device);

    void* data;
    vkMapMemory(device->instance, staging_buffer_mem, 0, buffer_size, 0, &data);
    memcpy(data, index_buffer->indices, buffer_size);
    vkUnmapMemory(device->instance, staging_buffer_mem);

    buffer_create(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &index_buffer->buffer.instance, &index_buffer->buffer.buffer_memory, device);
    buffer_copy(command_pool, device, buffer_size, &staging_buffer, &index_buffer->buffer.instance);

    //cleanup
    vkDestroyBuffer(device->instance, staging_buffer, NULL);
    vkFreeMemory(device->instance, staging_buffer_mem, NULL);
}
t_IndexBuffer buffer_index_init(const t_Device *device, const VkCommandPool *command_pool) {
    t_IndexBuffer index_buffer;
    index_buffer.size = 6;
    index_buffer.indices = (uint16_t*)malloc(sizeof(uint16_t) * index_buffer.size);
    const uint16_t predefined_indices[] = {0, 1, 2, 2, 3, 0};
    for (size_t i = 0; i < index_buffer.size; ++i) {
        index_buffer.indices[i] = predefined_indices[i];
    }
    buffer_index_create(&index_buffer, device, command_pool);

    return index_buffer;
}
void buffer_index_cleanup(const t_IndexBuffer *index_buffer, const VkDevice *device) {
    vkDestroyBuffer(*device, index_buffer->buffer.instance, NULL);
    vkFreeMemory(*device, index_buffer->buffer.buffer_memory, NULL);

    free(index_buffer->indices);
}
