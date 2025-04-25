#pragma once
#include <cglm/mat4.h>
#include <vulkan/vulkan_core.h>
#include "../texture.h"
#include "vk_buffers.h"

typedef struct DescriptorData {
    VkDescriptorPool pool;
    VkDescriptorSetLayout set_layout;
    VkDescriptorSet* desc_sets;
}t_DescriptorData;

void descriptor_cleanup(const VkDevice *device, const t_DescriptorData *desc);

void descriptor_populate(const VkDevice *device, const t_DescriptorData *desc_data, const t_UniformBufferData *ubo_data, const t_Texture *texture);

t_DescriptorData descriptor_init(const VkDevice *device);