#pragma once
#include <vulkan/vulkan_core.h>

#include "vk_depth.h"
#define ATTRIBUTE_COUNT 3

typedef struct Pipeline {
    VkRenderPass render_pass;
    VkPipelineLayout pipeline_layout;
    VkPipeline graphics_pipeline;
}t_Pipeline;

t_Pipeline pipeline_init(const t_Device *device, const VkFormat *image_format, VkExtent2D *extent, VkDescriptorSetLayout *descriptor_set_layout);
void pipeline_destroy(const t_Pipeline *pipeline, const VkDevice *device);