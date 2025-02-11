#include "vk_buffers.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <cglm/cglm.h>
#include "vk_renderer.h"

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
        .format = VK_FORMAT_R32G32B32_SFLOAT, // vec3 (pos)
        .offset = offsetof(t_Vertex, pos)
    };

    attribute_descriptions[1] = (VkVertexInputAttributeDescription){
        .binding = 0,
        .location = 1,
        .format = VK_FORMAT_R32G32B32_SFLOAT, // vec3 (color)
        .offset = offsetof(t_Vertex, color)
    };

    attribute_descriptions[2] = (VkVertexInputAttributeDescription){
        .binding = 0,
        .location = 2,
        .format = VK_FORMAT_R32G32_SFLOAT, // vec2 uv coord
        .offset = offsetof(t_Vertex, tex_coord)
    };
}
uint32_t find_memory_type(const uint32_t type_filter, const VkMemoryPropertyFlags properties, const VkPhysicalDevice physical_device) {
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
    VkCommandBuffer command_buffer = begin_single_time_commands(command_pool, &device->instance);

    const VkBufferCopy copy_region = {
        .srcOffset = 0, // Optional
        .dstOffset = 0, // Optional
        .size = size
    };

    vkCmdCopyBuffer(command_buffer, *src_buffer, *dst_buffer, 1, &copy_region);

    end_single_time_commands(command_pool, &command_buffer, device);

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
void buffer_vertex_create(t_VertexBuffer *vertex_buffer, const t_Device *device, const VkCommandPool *command_pool) {
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
    vertex_buffer.size = 8;
    vertex_buffer.vertices = (t_Vertex*)malloc(sizeof(t_Vertex) * vertex_buffer.size);
    const t_Vertex predefined_vertices[] = {
        {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
        {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},

    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
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
void buffer_index_create(t_IndexBuffer *index_buffer, const t_Device *device, const VkCommandPool *command_pool) {
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
    index_buffer.size = 12;
    index_buffer.indices = (uint16_t*)malloc(sizeof(uint16_t) * index_buffer.size);
    const uint16_t predefined_indices[] = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4
    };
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
static void buffer_ubo_create(t_UniformBufferData *ubo_data, const t_Device *device) {
    const VkDeviceSize buffer_size = sizeof(t_UniformBufferObject);

    ubo_data->uniform_buffers = (VkBuffer*)malloc(MAX_FRAMES_IN_FLIGHT * sizeof(VkBuffer));
    ubo_data->uniform_buffers_memory = (VkDeviceMemory*)malloc(MAX_FRAMES_IN_FLIGHT * sizeof(VkDeviceMemory));
    ubo_data->uniform_buffers_mapped = (void**)malloc(MAX_FRAMES_IN_FLIGHT * sizeof(void*));

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        buffer_create(buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &ubo_data->uniform_buffers[i], &ubo_data->uniform_buffers_memory[i], device);
        vkMapMemory(device->instance, ubo_data->uniform_buffers_memory[i], 0, buffer_size, 0, &ubo_data->uniform_buffers_mapped[i]);
    }
}
static struct timespec start_time;
t_UniformBufferData buffer_ubo_init(const t_Device *device) {
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    t_UniformBufferData ubo_data;
    buffer_ubo_create(&ubo_data, device);
    return ubo_data;
}
void buffer_ubo_cleanup(const VkDevice *device, const t_UniformBufferData *ubo_data) {
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(*device, ubo_data->uniform_buffers[i], NULL);
        vkFreeMemory(*device, ubo_data->uniform_buffers_memory[i], NULL);
    }
    free(ubo_data->uniform_buffers);
    free(ubo_data->uniform_buffers_mapped);
    free(ubo_data->uniform_buffers_memory);
}


void buffer_ubo_update(const uint32_t current_image, const VkExtent2D *extent_2d, void** uniform_buffers_mapped) {

    struct timespec current_time;
    clock_gettime(CLOCK_MONOTONIC, &current_time);

    const float elapsed = (current_time.tv_sec - start_time.tv_sec) +
                    (current_time.tv_nsec - start_time.tv_nsec) / 1e9f;
    t_UniformBufferObject ubo = {};
    glm_mat4_identity(ubo.model); // Start with identity matrix

    float angle = glm_rad(90.0f) * elapsed; // Rotate over time
    glm_rotate(ubo.model, angle, (vec3){0.0f, 0.0f, 1.0f}); // Rotate around Y-axis


    glm_lookat((vec3){2.0f, 2.0f, 2.0f},
               (vec3){0.0f, 0.0f, 0.0f},
               (vec3){0.0f, 0.0f, 1.0f},
               ubo.view);

    glm_perspective(glm_rad(45.0f), extent_2d->width / extent_2d->height, 0.1f, 10.0f, ubo.proj);
    ubo.proj[1][1] *= -1;

    memcpy(uniform_buffers_mapped[current_image], &ubo, sizeof(ubo));

}
