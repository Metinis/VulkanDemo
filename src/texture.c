#include "texture.h"
#include <stb_image.h>
#include <stdio.h>
#include <string.h>

#include "vk_buffers.h"
#include "vk_renderer.h"
static void transition_image_layout(VkImage image, VkFormat format, const VkImageLayout old_layout, const VkImageLayout new_layout,
    const VkCommandPool *command_pool, const t_Device *device) {
    VkCommandBuffer command_buffer = begin_single_time_commands(command_pool, &device->instance);

    VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .oldLayout = old_layout,
        .newLayout = new_layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .subresourceRange.baseMipLevel = 0,
        .subresourceRange.levelCount = 1,
        .subresourceRange.baseArrayLayer = 0,
        .subresourceRange.layerCount = 1,
        .srcAccessMask = 0, //todo
        .dstAccessMask = 0 //todo
    };
    VkPipelineStageFlags source_stage;
    VkPipelineStageFlags destination_stage;

    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        printf("unsupported layout transition!\n");
    }
    vkCmdPipelineBarrier(
        command_buffer,
        0 /* TODO */, 0 /* TODO */,
        0,
        0, NULL,
        0, NULL,
        1, &barrier
    );

    end_single_time_commands(command_pool, &command_buffer, device);
}
static void copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height,
    const VkCommandPool *command_pool, const t_Device *device) {
    VkCommandBuffer command_buffer = begin_single_time_commands(command_pool, &device->instance);

    VkBufferImageCopy region = {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,

        .imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .imageSubresource.mipLevel = 0,
        .imageSubresource.baseArrayLayer = 0,
        .imageSubresource.layerCount = 1,

        .imageOffset = {0, 0, 0},
        .imageExtent = {
            width,
            height,
            1
        }
    };
    vkCmdCopyBufferToImage(
        command_buffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    end_single_time_commands(command_pool, &command_buffer, device);
}

static void create_image(const t_Device *device, t_Texture *texture, const int tex_width, const int tex_height) {
    const VkImageCreateInfo image_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .extent.width = tex_width,
        .extent.height = tex_height,
        .extent.depth = 1,
        .mipLevels = 1,
        .arrayLayers = 1,
        .format = VK_FORMAT_R8G8B8A8_SRGB,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .flags = 0
    };

    if(vkCreateImage(device->instance, &image_info, NULL, &texture->texture_image) != VK_SUCCESS) {
        printf("Failed to create a texture image! \n");
    }

    VkMemoryRequirements mem_requirements;
    vkGetImageMemoryRequirements(device->instance, texture->texture_image, &mem_requirements);

    const VkMemoryAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = mem_requirements.size,
        .memoryTypeIndex = find_memory_type(mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, device->physical_device)
    };

    if (vkAllocateMemory(device->instance, &alloc_info, NULL, &texture->texture_image_memory) != VK_SUCCESS) {
        printf("Failed to allocate image memory! \n");
    }

    vkBindImageMemory(device->instance, texture->texture_image, texture->texture_image_memory, 0);
}
static void create_texture_image(const char* filepath, t_Texture *texture, const t_Device *device, const VkCommandPool *command_pool) {
    int tex_width, tex_height, tex_channels;
    stbi_uc* pixels = stbi_load(filepath, &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
    const VkDeviceSize image_size = tex_width * tex_height * 4;

    if (!pixels) {
        printf("Failed to load texture image! \n");
    }
    buffer_create(image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &texture->staging_buffer, &texture->staging_buffer_memory, device);
    void* data;
    vkMapMemory(device->instance, texture->staging_buffer_memory, 0, image_size, 0, &data);
    memcpy(data, pixels, (size_t)(image_size));
    vkUnmapMemory(device->instance, texture->staging_buffer_memory);

    stbi_image_free(pixels);

    create_image(device, texture, tex_width, tex_height);

    transition_image_layout(texture->texture_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, command_pool, device);

    copy_buffer_to_image(texture->staging_buffer, texture->texture_image, tex_width, tex_height, command_pool, device);

    transition_image_layout(texture->texture_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, command_pool, device);

    vkDestroyBuffer(device->instance, texture->staging_buffer, NULL);
    vkFreeMemory(device->instance, texture->staging_buffer_memory, NULL);

}
static void texture_create_image_view(t_Texture *texture, const t_Device *device) {
    texture->image_view = create_image_view(texture->texture_image, VK_FORMAT_R8G8B8A8_SRGB, &device->instance);
}
static void texture_create_sampler(t_Texture *texture, const t_Device *device) {
    //retrieve properties for maxAnistropy
    VkPhysicalDeviceProperties properties = {};
    vkGetPhysicalDeviceProperties(device->physical_device, &properties);

    const VkSamplerCreateInfo sampler_info = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .mipLodBias = 0.0f,
        .minLod = 0.0f,
        .maxLod = 0.0f
    };
    if (vkCreateSampler(device->instance, &sampler_info, NULL, &texture->texture_sampler) != VK_SUCCESS) {
        printf("Failed to create texture sample! \n");
    }
}
t_Texture texture_init(const char* filepath, const t_Device *device, const VkCommandPool *command_pool) {
    t_Texture texture;
    create_texture_image(filepath, &texture, device, command_pool);
    texture_create_image_view(&texture, device);
    texture_create_sampler(&texture, device);
    return texture;
}
void texture_cleanup(const VkDevice *device, const t_Texture *texture) {
    vkDestroySampler(*device, texture->texture_sampler, NULL);
    vkDestroyImageView(*device, texture->image_view, NULL);
    vkDestroyImage(*device, texture->texture_image, NULL);
    vkFreeMemory(*device, texture->texture_image_memory, NULL);
}


