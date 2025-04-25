#include "texture.h"
#include <stb_image.h>
#include <stdio.h>
#include <string.h>

#include "vulkan/vk_buffers.h"
#include "vulkan/vk_depth.h"
#include "vulkan/vk_renderer.h"
void transition_image_layout(VkImage image, VkFormat format, const VkImageLayout old_layout, const VkImageLayout new_layout,
    const uint32_t mip_levels, const VkCommandPool *command_pool, const t_Device *device) {
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
        .subresourceRange.levelCount = mip_levels,
        .subresourceRange.baseArrayLayer = 0,
        .subresourceRange.layerCount = 1,
        .srcAccessMask = 0, //todo
        .dstAccessMask = 0 //todo
    };
    VkPipelineStageFlags source_stage;
    VkPipelineStageFlags destination_stage;

    if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (has_stencil_component(format)) {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    } else {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

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
    } else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
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
static void copy_buffer_to_image(const VkBuffer buffer, const VkImage image, const uint32_t width, const uint32_t height,
    const VkCommandPool *command_pool, const t_Device *device) {
    VkCommandBuffer command_buffer = begin_single_time_commands(command_pool, &device->instance);

    const VkBufferImageCopy region = {
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

void create_image(const t_Device *device, VkImage *image, VkDeviceMemory *image_memory, const VkFormat format, const VkImageTiling tiling,
    const VkImageUsageFlags usage, const VkMemoryPropertyFlags properties, const uint32_t mip_levels, const VkSampleCountFlagBits num_samples, const int tex_width, const int tex_height) {
    const VkImageCreateInfo image_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .extent.width = tex_width,
        .extent.height = tex_height,
        .extent.depth = 1,
        .mipLevels = mip_levels,
        .arrayLayers = 1,
        .format = format,//VK_FORMAT_R8G8B8A8_SRGB,
        .tiling = tiling,//VK_IMAGE_TILING_OPTIMAL,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .usage = usage,//VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .samples = num_samples,
        .flags = 0
    };

    if(vkCreateImage(device->instance, &image_info, NULL, image) != VK_SUCCESS) {
        printf("Failed to create a texture image! \n");
    }

    VkMemoryRequirements mem_requirements;
    vkGetImageMemoryRequirements(device->instance, *image, &mem_requirements);

    const VkMemoryAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = mem_requirements.size,
        .memoryTypeIndex = find_memory_type(mem_requirements.memoryTypeBits, properties, device->physical_device)

        //.memoryTypeIndex = find_memory_type(mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, device->physical_device)
    };

    if (vkAllocateMemory(device->instance, &alloc_info, NULL, image_memory) != VK_SUCCESS) {
        printf("Failed to allocate image memory! \n");
    }

    vkBindImageMemory(device->instance, *image, *image_memory, 0);
}

static void generate_mipmaps(const VkImage image, const VkFormat image_format, const int32_t tex_width, const int32_t tex_height,
    const uint32_t mip_levels, const VkCommandPool *command_pool, const t_Device *device) {
    // Check if image format supports linear blitting
    VkFormatProperties format_properties;
    vkGetPhysicalDeviceFormatProperties(device->physical_device, image_format, &format_properties);

    if (!(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        printf("texture image format does not support linear blitting!\n");
    }

    VkCommandBuffer command_buffer = begin_single_time_commands(command_pool, &device->instance);

    VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .image = image,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .subresourceRange.baseArrayLayer = 0,
        .subresourceRange.layerCount = 1,
        .subresourceRange.levelCount = 1
    };
    int32_t mip_width = tex_width;
    int32_t mip_height = tex_height;

    for (uint32_t i = 1; i < mip_levels; i++) {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            vkCmdPipelineBarrier(command_buffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                0, NULL,
                0, NULL,
                1, &barrier);

            VkImageBlit blit = {
            .srcOffsets[0].x = 0,
            .srcOffsets[0].y = 0,
            .srcOffsets[0].z = 0,
            .srcOffsets[1].x = mip_width,
            .srcOffsets[1].y = mip_height,
            .srcOffsets[1].z = 1,
            .srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .srcSubresource.mipLevel = i - 1,
            .srcSubresource.baseArrayLayer = 0,
            .srcSubresource.layerCount = 1,
            .dstOffsets[0].x = 0,
            .dstOffsets[0].y = 0,
            .dstOffsets[0].z = 0,
            .dstOffsets[1].x = mip_width > 1 ? mip_width / 2 : 1,
            .dstOffsets[1].y = mip_height > 1 ? mip_height / 2 : 1,
            .dstOffsets[1].z = 1,
            .dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .dstSubresource.mipLevel = i,
            .dstSubresource.baseArrayLayer = 0,
            .dstSubresource.layerCount = 1,
            };

            vkCmdBlitImage(command_buffer,
                image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1, &blit,
                VK_FILTER_LINEAR);

            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(command_buffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                0, NULL,
                0, NULL,
                1, &barrier);

            if (mip_width > 1) mip_width /= 2;
            if (mip_height > 1) mip_height /= 2;
        }

        barrier.subresourceRange.baseMipLevel = mip_levels - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(command_buffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
            0, NULL,
            0, NULL,
            1, &barrier);

    end_single_time_commands(command_pool, &command_buffer, device);



}
static void create_texture_image(const char* filepath, t_Texture *texture, const t_Device *device, const VkCommandPool *command_pool) {
    int tex_width, tex_height, tex_channels;
    stbi_uc* pixels = stbi_load(filepath, &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
    const VkDeviceSize image_size = tex_width * tex_height * 4;

    if (!pixels) {
        printf("Failed to load texture image! \n");
    }
    texture->mip_levels = (uint32_t)(floor(log2(glm_max(tex_width, tex_height)))) + 1;
    buffer_create(image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &texture->staging_buffer, &texture->staging_buffer_memory, device);
    void* data;
    vkMapMemory(device->instance, texture->staging_buffer_memory, 0, image_size, 0, &data);
    memcpy(data, pixels, (size_t)(image_size));
    vkUnmapMemory(device->instance, texture->staging_buffer_memory);

    stbi_image_free(pixels);

    create_image(device, &texture->texture_image, &texture->texture_image_memory, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texture->mip_levels, VK_SAMPLE_COUNT_1_BIT, tex_width, tex_height);

    transition_image_layout(texture->texture_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, texture->mip_levels, command_pool, device);

    copy_buffer_to_image(texture->staging_buffer, texture->texture_image, tex_width, tex_height, command_pool, device);

    transition_image_layout(texture->texture_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, texture->mip_levels, command_pool, device);

    generate_mipmaps(texture->texture_image, VK_FORMAT_R8G8B8A8_SRGB, tex_width, tex_height, texture->mip_levels, command_pool, device);

    vkDestroyBuffer(device->instance, texture->staging_buffer, NULL);
    vkFreeMemory(device->instance, texture->staging_buffer_memory, NULL);

}
static void texture_create_image_view(t_Texture *texture, const t_Device *device) {
    texture->image_view = create_image_view(texture->texture_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, texture->mip_levels, &device->instance);
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
        .maxLod = (float)texture->mip_levels
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


