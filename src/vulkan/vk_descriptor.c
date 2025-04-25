#include "vk_descriptor.h"

#include <stdio.h>


#include "../utils.h"
#include "vk_renderer.h"


static void descriptor_create_set_layout(const VkDevice *device, VkDescriptorSetLayout *descriptor_set_layout) {
    const VkDescriptorSetLayoutBinding ubo_layout_binding = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT, //which shader will reference it
        .pImmutableSamplers = NULL
    };
    VkDescriptorSetLayoutBinding sampler_layout_binding = {
        .binding = 1,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImmutableSamplers = NULL,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
    };
    VkDescriptorSetLayoutBinding bindings[2] = {ubo_layout_binding, sampler_layout_binding};
    const VkDescriptorSetLayoutCreateInfo layout_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 2,
        .pBindings = bindings
    };

    if (vkCreateDescriptorSetLayout(*device, &layout_info, NULL, descriptor_set_layout) != VK_SUCCESS) {
        printf("failed to create descriptor set layout!");
    }

}
void descriptor_cleanup(const VkDevice *device, const t_DescriptorData *desc) {
    vkDestroyDescriptorPool(*device, desc->pool, NULL);
    vkDestroyDescriptorSetLayout(*device, desc->set_layout, NULL);
    free(desc->desc_sets);
}
static void descriptor_create_pool(const VkDevice *device, VkDescriptorPool *pool) {
    VkDescriptorPoolSize pool_sizes[2];
    pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_sizes[0].descriptorCount = (uint32_t)(MAX_FRAMES_IN_FLIGHT);
    pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_sizes[1].descriptorCount = (uint32_t)(MAX_FRAMES_IN_FLIGHT);
    const VkDescriptorPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .poolSizeCount = 2,
        .pPoolSizes = pool_sizes,
        .maxSets = (uint32_t)MAX_FRAMES_IN_FLIGHT
    };
    if (vkCreateDescriptorPool(*device, &pool_info, NULL, pool) != VK_SUCCESS) {
        printf("Failed to create descriptor pool! \n");
    }

}
static void descriptor_create_sets(const VkDevice *device, t_DescriptorData *desc_data) {
    VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT]; // Stack allocation
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        layouts[i] = desc_data->set_layout;
    }
    const VkDescriptorSetAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = desc_data->pool,
        .descriptorSetCount = (uint32_t)(MAX_FRAMES_IN_FLIGHT),
        .pSetLayouts = layouts
    };
    desc_data->desc_sets = (VkDescriptorSet*)malloc(MAX_FRAMES_IN_FLIGHT * sizeof(VkDescriptorSet));
    if (vkAllocateDescriptorSets(*device, &alloc_info, desc_data->desc_sets) != VK_SUCCESS) {
        printf("Failed to allocate descriptor sets!\n");
    }

}
void descriptor_populate(const VkDevice *device, const t_DescriptorData *desc_data, const t_UniformBufferData *ubo_data, const t_Texture *texture) {
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo buffer_info = {
            .buffer = ubo_data->uniform_buffers[i],
            .offset = 0,
            .range = sizeof(t_UniformBufferObject)
        };
        VkDescriptorImageInfo image_info = {
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .imageView = texture->image_view,
            .sampler = texture->texture_sampler,
        };

        VkWriteDescriptorSet descriptor_writes[2];

        descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[0].dstSet = desc_data->desc_sets[i];
        descriptor_writes[0].dstBinding = 0;
        descriptor_writes[0].dstArrayElement = 0;
        descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_writes[0].descriptorCount = 1;
        descriptor_writes[0].pBufferInfo = &buffer_info;

        descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[1].dstSet = desc_data->desc_sets[i];
        descriptor_writes[1].dstBinding = 1;
        descriptor_writes[1].dstArrayElement = 0;
        descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_writes[1].descriptorCount = 1;
        descriptor_writes[1].pImageInfo = &image_info;

        vkUpdateDescriptorSets(*device, (uint32_t)(2), descriptor_writes, 0, NULL);
    }

}
t_DescriptorData descriptor_init(const VkDevice *device) {
    t_DescriptorData desc_data = {};
    descriptor_create_set_layout(device, &desc_data.set_layout);
    descriptor_create_pool(device, &desc_data.pool);
    descriptor_create_sets(device, &desc_data);

    return desc_data;

}
