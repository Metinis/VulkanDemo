#include "application.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vulkan_debug.h"

static uint8_t app_check_validation_layer_support(const char** validation_layers, const size_t validation_size) {
    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, NULL);

    VkLayerProperties available_layers[layer_count];
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers);

    for(size_t i = 0; i < validation_size; i++) {
        uint8_t layer_found = 0;

        for(size_t j = 0; j < layer_count; j++) {
            if (strcmp(validation_layers[i], available_layers[j].layerName) == 0) {
                layer_found = 1;
                break;
            }
        }
        if(!layer_found) {
            return 0;
        }
    }
    return 1;
}
static uint8_t app_check_device_extension_support(const t_Application *app, const VkPhysicalDevice device) {
    //enumerate and find if they exist
    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, NULL);

    VkExtensionProperties available_extensions[extension_count];
    vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, available_extensions);

    uint32_t matching_ext_count = 0;

    for(size_t i = 0; i < extension_count; i++) {
        for(size_t j = 0; j < app->device_ext_size; j++) {
            if (strcmp(available_extensions[i].extensionName, app->device_extensions[j]) == 0) {
                matching_ext_count++;
            }
        }
    }

    return matching_ext_count == app->device_ext_size;
}
static void app_enable_extensions(VkInstanceCreateInfo* create_info, const char*** extensions_ptr,
    const uint8_t enable_validation_layers) {

    uint32_t glfw_extensionCount = 0;
    const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extensionCount);

    create_info->enabledExtensionCount = glfw_extensionCount;
    create_info->ppEnabledExtensionNames = glfw_extensions;

    create_info->enabledLayerCount = 0;

    //****FOR MAC VK_INCOMPATIBLE DRIVER ****

    uint32_t extension_count = glfw_extensionCount + 1;
    if(enable_validation_layers) {
        extension_count++;
    }

    const char** required_extensions = (const char**)malloc(extension_count * sizeof(const char*));

    for(size_t i = 0; i < glfw_extensionCount; i++) {
        required_extensions[i] = glfw_extensions[i];
    }

    required_extensions[glfw_extensionCount] = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;

    if (enable_validation_layers) {
        required_extensions[glfw_extensionCount + 1] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    }

    create_info->flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

    create_info->enabledExtensionCount = extension_count;
    create_info->ppEnabledExtensionNames = required_extensions;

    //****FOR MAC VK_INCOMPATIBLE DRIVER ****

    *extensions_ptr = required_extensions;
}
static void app_enable_validation(const t_Application *app, VkInstanceCreateInfo* create_info) {

    if (app_check_validation_layer_support(app->validation_layers, app->validation_size)) {
        create_info->enabledLayerCount = (uint32_t)(app->validation_size);
        create_info->ppEnabledLayerNames = app->validation_layers;
    } else {
        create_info->enabledLayerCount = 0;
        create_info->ppEnabledLayerNames = NULL;
    }

}

static void app_create_vulkan_inst(t_Application *app) {
    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "VulkanDemo";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "Vetox";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;

    app->enable_validation_layers = 1;

#ifdef NDEBUG
    app->enable_validation_layers = 0;
#endif

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {};

    if(app->enable_validation_layers) {
        app_enable_validation(app, &create_info);

        debug_populate_messenger_create_info(&debug_create_info);
        create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debug_create_info;

        debug_print_available_validation_layers();
    }

    const char** extensions = NULL;
    app_enable_extensions(&create_info, &extensions, app->enable_validation_layers);

    const VkResult result = vkCreateInstance(&create_info, NULL, &app->vk_instance);

    if(app->enable_validation_layers) {
        printf("Enabled Vulkan extensions:\n");
        for (uint32_t i = 0; i < create_info.enabledExtensionCount; i++) {
            printf("  %s\n", extensions[i]);
        }
    }

    free(extensions);

    if (result != VK_SUCCESS) {
        printf("Failed to create Vulkan instance. VkResult: %d\n", result);
    }
}
t_QueueFamilyIndices app_find_queue_families(const VkSurfaceKHR *surface, const VkPhysicalDevice *device) {
    t_QueueFamilyIndices indices = {
        .graphics_family = { .value = 0, .has_value = 0 },
        .present_family = { .value = 0, .has_value = 0 }
    };

    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(*device, &queue_family_count, NULL);

    VkQueueFamilyProperties queue_families[queue_family_count];
    vkGetPhysicalDeviceQueueFamilyProperties(*device, &queue_family_count, queue_families);

    for(size_t i = 0; i < queue_family_count; i++) {
        if(queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphics_family.has_value = 1;
            indices.graphics_family.value = i;
        }
        VkBool32 present_support = 0;
        vkGetPhysicalDeviceSurfaceSupportKHR(*device, i, *surface, &present_support);
        if (present_support) {
            indices.present_family.has_value = 1;
            indices.present_family.value = i;
        }
        if(is_complete(&indices)) {
            break;
        }
        i++;
    }

    return indices;
}

static uint8_t app_is_device_suitable(t_Application *app, const VkPhysicalDevice *device) {
    const t_QueueFamilyIndices indices = app_find_queue_families(&app->surface, device);

    //check swap chain support

    t_SwapChainSupportDetails swap_details = {};
    swap_chain_query_support_details(&swap_details, &app->surface, device);

    const uint8_t swap_chain_adequate = swap_details.formats && swap_details.present_modes;


    swap_chain_free_support(&swap_details);

    return is_complete(&indices) && app_check_device_extension_support(app, *device) && swap_chain_adequate;
}

static void app_pick_physical_device(t_Application *app) {
    app->physical_device = VK_NULL_HANDLE;

    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(app->vk_instance, &device_count, NULL);

    if (device_count == 0) {
        printf("Failed to find device with vulkan support!\n");
        exit(-1);
    }

    VkPhysicalDevice devices[device_count];
    vkEnumeratePhysicalDevices(app->vk_instance, &device_count, devices);

    for(size_t i = 0; i < device_count; i++) {
        if(app_is_device_suitable(app, &devices[i])) {
            app->physical_device = devices[i];
            break;
        }
    }
    if(app->physical_device == VK_NULL_HANDLE) {
        printf("Failed to find suitable device!\n");
        exit(-1);
    }
}
static void app_enable_validation_device(const t_Application *app, VkDeviceCreateInfo* create_info) {

    if (app_check_validation_layer_support(app->validation_layers, app->validation_size)) {
        create_info->enabledLayerCount = (uint32_t)(app->validation_size);
        create_info->ppEnabledLayerNames = app->validation_layers;
    } else {
        create_info->enabledLayerCount = 0;
        create_info->ppEnabledLayerNames = NULL;
    }

}
static void app_create_logical_device(t_Application *app) {
    const t_QueueFamilyIndices indices = app_find_queue_families(&app->surface, &app->physical_device);

    if (!indices.graphics_family.has_value || !indices.present_family.has_value) {
        printf("Error: Required queue families are not available!\n");
        exit(-1);
    }

    const uint32_t unique_queue_families[2] = {indices.graphics_family.value, indices.present_family.value};
    const uint32_t queue_size = (indices.graphics_family.value != indices.present_family.value) ? 2 : 1;

    float queue_priority = 1.0f;

    VkDeviceQueueCreateInfo queue_create_infos[queue_size];
    const VkDeviceQueueCreateInfo base_queue_create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .queueCount = 1,
        .pQueuePriorities = &queue_priority,
    };

    for (uint32_t i = 0; i < queue_size; i++) {
        queue_create_infos[i] = base_queue_create_info;
        queue_create_infos[i].queueFamilyIndex = unique_queue_families[i];
    }

    // Specify device features
    VkPhysicalDeviceFeatures device_features = {};

    VkDeviceCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pQueueCreateInfos = queue_create_infos,
        .queueCreateInfoCount = queue_size,
        .pEnabledFeatures = &device_features,
        .enabledExtensionCount = app->device_ext_size,
        .ppEnabledExtensionNames = app->device_extensions
    };

    app_enable_validation_device(app, &create_info);

    if (vkCreateDevice(app->physical_device, &create_info, NULL, &app->device) != VK_SUCCESS) {
        printf("\nError: Failed to create Vulkan logical device!");
        exit(-1);
    }

    // Retrieve queue handles
    vkGetDeviceQueue(app->device, indices.graphics_family.value, 0, &app->graphics_queue);
    vkGetDeviceQueue(app->device, indices.present_family.value, 0, &app->present_queue);

    printf("\nLogical device created successfully.");
}

static void app_create_surface(t_Application *app) {
    if (glfwCreateWindowSurface(app->vk_instance, app->window, NULL, &app->surface) != VK_SUCCESS) {
        printf("\nFailed to create window surface! \n");
    }
}
static void app_create_image_views(t_Application *app) {
    app->swap_chain.image_views = (VkImageView*)malloc(app->swap_chain.image_count * sizeof(VkImageView));
    for(size_t i = 0; i < app->swap_chain.image_count; i++) {
        VkImageViewCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = app->swap_chain.images[i],
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = app->swap_chain.image_format,
        .components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
        .components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
        .components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
        .components.a = VK_COMPONENT_SWIZZLE_IDENTITY,
        .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .subresourceRange.baseMipLevel = 0,
        .subresourceRange.levelCount = 1,
        .subresourceRange.baseArrayLayer = 0,
        .subresourceRange.layerCount = 1
            };

        if (vkCreateImageView(app->device, &create_info, NULL, &app->swap_chain.image_views[i]) != VK_SUCCESS) {
            printf("\nFailed to create image views!");
            exit(-1);
        }
    }
}
static VkShaderModule app_create_shader_module(const t_Application *app, const unsigned char* code, const size_t file_size) {
    const VkShaderModuleCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = file_size,
        .pCode = (const uint32_t*)(code)
    };
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(app->device, &create_info, NULL, &shaderModule) != VK_SUCCESS) {
        printf("Failed to create a shader module! \n");
    }
    return shaderModule;
}

static void app_create_graphics_pipeline(t_Application *app) {
    size_t vert_file_size;
    unsigned char* vert_shader_code = read_file("../resources/shader/shader.vert.spv", &vert_file_size);
    size_t frag_file_size;
    unsigned char* frag_shader_code = read_file("../resources/shader/shader.frag.spv", &frag_file_size);

    const VkShaderModule vert_shader_module = app_create_shader_module(app, vert_shader_code, vert_file_size);
    free(vert_shader_code);

    const VkShaderModule frag_shader_module = app_create_shader_module(app, frag_shader_code, frag_file_size);
    free(frag_shader_code);

    const VkPipelineShaderStageCreateInfo vert_shader_stage_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vert_shader_module,
        .pName = "main"
    };
    const VkPipelineShaderStageCreateInfo frag_shader_stage_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = frag_shader_module,
        .pName = "main"
    };

    VkPipelineShaderStageCreateInfo shaderStages[] = {vert_shader_stage_info, frag_shader_stage_info};



    const uint32_t dynamic_state_size = 2;
    const VkDynamicState dynamic_states[dynamic_state_size] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamic_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = dynamic_state_size,
        .pDynamicStates = dynamic_states
    };

    VkPipelineVertexInputStateCreateInfo vertex_input_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 0,
        .pVertexBindingDescriptions = NULL,
        .vertexAttributeDescriptionCount = 0,
        .pVertexAttributeDescriptions = NULL
    };

    VkPipelineInputAssemblyStateCreateInfo input_assembly = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE
    };

    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = (float) app->swap_chain.extent.width,
        .height = (float) app->swap_chain.extent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    VkRect2D scissor = {
        .offset = {0, 0},
        .extent = app->swap_chain.extent
    };

    VkPipelineViewportStateCreateInfo viewport_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor
    };

    VkPipelineRasterizationStateCreateInfo rasterizer = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .lineWidth = 1.0f,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f
    };

    VkPipelineMultisampleStateCreateInfo multi_sampling = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .sampleShadingEnable = VK_FALSE,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .minSampleShading = 1.0f,
        .pSampleMask = NULL,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE
    };

    VkPipelineColorBlendAttachmentState color_blend_attachment = {
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        .blendEnable = VK_TRUE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ONE, // Optional
        .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO, // Optional
        .colorBlendOp = VK_BLEND_OP_ADD, // Optional
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE, // Optional
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO, // Optional
        .alphaBlendOp = VK_BLEND_OP_ADD // Optional
    };

    VkPipelineColorBlendStateCreateInfo color_blending = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY, // Optional
        .attachmentCount = 1,
        .pAttachments = &color_blend_attachment,
        .blendConstants[0] = 0.0f, // Optional
        .blendConstants[1] = 0.0f, // Optional
        .blendConstants[2] = 0.0f, // Optional
        .blendConstants[3] = 0.0f, // Optional
    };

    VkPipelineLayoutCreateInfo pipeline_layout_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 0, // Optional
        .pSetLayouts = NULL, // Optional
        .pushConstantRangeCount = 0, // Optional
        .pPushConstantRanges = NULL, // Optional
    };

    if (vkCreatePipelineLayout(app->device, &pipeline_layout_info, NULL, &app->pipeline_layout) != VK_SUCCESS) {
        printf("Failed to create pipeline layout! \n");
    }

    VkGraphicsPipelineCreateInfo pipeline_info = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = &vertex_input_info,
        .pInputAssemblyState = &input_assembly,
        .pViewportState = &viewport_state,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multi_sampling,
        .pDepthStencilState = NULL, // Optional
        .pColorBlendState = &color_blending,
        .pDynamicState = &dynamic_state,
        .layout = app->pipeline_layout,
        .renderPass = app->render_pass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1
    };
    if (vkCreateGraphicsPipelines(app->device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &app->graphics_pipeline) != VK_SUCCESS) {
        printf("Failed To Create Graphics Pipeline! \n");
    }
    vkDestroyShaderModule(app->device, vert_shader_module, NULL);
    vkDestroyShaderModule(app->device, frag_shader_module, NULL);

}
static void app_create_swap_chain(t_Application *app) {
    const t_QueueFamilyIndices indices = app_find_queue_families(&app->surface, &app->physical_device);

    app->swap_chain = swap_chain_create(&app->surface, &app->device, &app->physical_device, app->window, &indices);
}
static void app_create_render_pass(t_Application *app) {
    VkAttachmentDescription color_attachment = {
        .format = app->swap_chain.image_format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    VkAttachmentReference color_attachment_ref = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment_ref
    };

    VkSubpassDependency dependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
    };

    const VkRenderPassCreateInfo render_pass_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &color_attachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency
    };


    if (vkCreateRenderPass(app->device, &render_pass_info, NULL, &app->render_pass) != VK_SUCCESS) {
        printf("Failed to create render pass! \n");
    }
}
static void app_create_frame_buffers(t_Application *app) {
    app->swap_chain.framebuffers = (VkFramebuffer*)malloc(app->swap_chain.image_count * sizeof(VkFramebuffer));
    for (size_t i = 0; i < app->swap_chain.image_count; i++) {
        VkImageView attachments[] = {
            app->swap_chain.image_views[i]
        };

        VkFramebufferCreateInfo framebuffer_info = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = app->render_pass,
            .attachmentCount = 1,
            .pAttachments = attachments,
            .width = app->swap_chain.extent.width,
            .height = app->swap_chain.extent.height,
            .layers = 1,
        };

        if (vkCreateFramebuffer(app->device, &framebuffer_info, NULL, &app->swap_chain.framebuffers[i]) != VK_SUCCESS) {
            printf("Failed to create a framebuffer! \n");
        }
    }
}
static void app_create_command_pool(t_Application *app) {
    const t_QueueFamilyIndices queue_family_indices = app_find_queue_families(&app->surface, &app->physical_device);

    const VkCommandPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queue_family_indices.graphics_family.value
    };

    if (vkCreateCommandPool(app->device, &pool_info, NULL, &app->command_pool) != VK_SUCCESS) {
        printf("Failed to create command pool! \n");
    }
}
static void app_create_command_buffers(t_Application *app) {
    app->command_buffers = (VkCommandBuffer*)malloc(sizeof(VkCommandBuffer) * MAX_FRAMES_IN_FLIGHT);
    const VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = app->command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = MAX_FRAMES_IN_FLIGHT,
    };

    if (vkAllocateCommandBuffers(app->device, &alloc_info, app->command_buffers) != VK_SUCCESS) {
        printf("Failed to create command buffer! \n");
    }

}
static void app_record_command_buffer(const t_Application *app, const VkCommandBuffer *command_buffer, const uint32_t image_index) {
    const VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = 0, // Optional
        .pInheritanceInfo = NULL, // Optional
    };
    if (vkBeginCommandBuffer(*command_buffer, &begin_info) != VK_SUCCESS) {
        printf("Failed to begin command buffer! \n");
    }
    VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};

    const VkRenderPassBeginInfo render_pass_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = app->render_pass,
        .framebuffer = app->swap_chain.framebuffers[image_index],
        .renderArea.offset = {0, 0},
        .renderArea.extent = app->swap_chain.extent,
        .clearValueCount = 1,
        .pClearValues = &clear_color
    };

    vkCmdBeginRenderPass(*command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(*command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, app->graphics_pipeline);

    const VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = (float)(app->swap_chain.extent.width),
        .height = (float)(app->swap_chain.extent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };
    vkCmdSetViewport(*command_buffer, 0, 1, &viewport);

    const VkRect2D scissor = {
    .offset = {0, 0},
    .extent = app->swap_chain.extent,
    };
    vkCmdSetScissor(*command_buffer, 0, 1, &scissor);

    vkCmdDraw(*command_buffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(*command_buffer);

    if (vkEndCommandBuffer(*command_buffer) != VK_SUCCESS) {
        printf("Failed to end command buffer! \n");
    }

}
static void app_create_sync_objects(t_Application *app) {
    app->image_available_semaphores = (VkSemaphore*)malloc(sizeof(VkSemaphore) * MAX_FRAMES_IN_FLIGHT);
    app->render_finished_semaphores = (VkSemaphore*)malloc(sizeof(VkSemaphore) * MAX_FRAMES_IN_FLIGHT);
    app->in_flight_fences = (VkFence*)malloc(sizeof(VkFence) * MAX_FRAMES_IN_FLIGHT);

    const VkSemaphoreCreateInfo semaphore_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };
    const VkFenceCreateInfo fence_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(app->device, &semaphore_info, NULL, &app->image_available_semaphores[i]) != VK_SUCCESS ||
        vkCreateSemaphore(app->device, &semaphore_info, NULL, &app->render_finished_semaphores[i]) != VK_SUCCESS ||
        vkCreateFence(app->device, &fence_info, NULL, &app->in_flight_fences[i]) != VK_SUCCESS) {
            printf("Failed to create semaphores! \n");
        }
    }


}
static void app_vulkan_init(t_Application *app) {
    debug_print_available_extensions();

    app_create_vulkan_inst(app);

    debug_setup_messenger(app);

    app_create_surface(app);

    app_pick_physical_device(app);

    app_create_logical_device(app);

    app_create_swap_chain(app);

    app_create_image_views(app);

    app_create_render_pass(app);

    app_create_graphics_pipeline(app);

    app_create_frame_buffers(app);

    app_create_command_pool(app);

    app_create_command_buffers(app);

    app_create_sync_objects(app);
}

static void framebuffer_resize_callback(GLFWwindow *window, int width, int height) {
    t_Application *app = (t_Application *)glfwGetWindowUserPointer(window);

    app->framebuffer_resized = 1;

}
void app_init(t_Application *app) {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    app->window = glfwCreateWindow(1280,720, "VulkanDemo", NULL, NULL);
    glfwSetWindowUserPointer(app->window, app);
    glfwSetFramebufferSizeCallback(app->window, framebuffer_resize_callback);

    app->current_frame = 0;
    app->framebuffer_resized = 0;

    //setup validation list

    const char *default_device_extensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };
    app->device_ext_size = 1;
    app->device_extensions = malloc(app->device_ext_size * sizeof(char *));
    for (size_t i = 0; i < app->device_ext_size; i++) {
        app->device_extensions[i] = default_device_extensions[i];
    }

    //setup device extension list

    const char *default_validation_layers[] = {"VK_LAYER_KHRONOS_validation"};
    app->validation_size = 1;
    app->validation_layers = malloc(app->validation_size * sizeof(char *));
    for (size_t i = 0; i < app->device_ext_size; i++) {
        app->validation_layers[i] = default_validation_layers[i];
    }

    app_vulkan_init(app);

    free(app->device_extensions);
    free(app->validation_layers);
}
static void app_cleanup_swap_chain(const t_Application *app) {
    for (size_t i = 0; i < app->swap_chain.image_count; i++) {
        vkDestroyFramebuffer(app->device, app->swap_chain.framebuffers[i], NULL);
    }

    for (size_t i = 0; i < app->swap_chain.image_count; i++) {
        vkDestroyImageView(app->device, app->swap_chain.image_views[i], NULL);
    }

    vkDestroySwapchainKHR(app->device, app->swap_chain.instance, NULL);
}
static void app_recreate_swap_chain(t_Application *app) {
    //pause until window is not minimised
    int width = 0, height = 0;
    glfwGetFramebufferSize(app->window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(app->window, &width, &height);
        glfwWaitEvents();
    }
    vkDeviceWaitIdle(app->device);

    app_cleanup_swap_chain(app);

    const t_QueueFamilyIndices indices = app_find_queue_families(&app->surface, &app->physical_device);

    swap_chain_create(&app->surface, &app->device, &app->physical_device, app->window, &indices);
    app_create_image_views(app);
    app_create_frame_buffers(app);

}

static void app_draw_frame(t_Application *app) {
    vkWaitForFences(app->device, 1, &app->in_flight_fences[app->current_frame], VK_TRUE, UINT64_MAX);

    uint32_t image_index;
    VkResult result = vkAcquireNextImageKHR(app->device, app->swap_chain.instance, UINT64_MAX, app->image_available_semaphores[app->current_frame], VK_NULL_HANDLE, &image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || app->framebuffer_resized) {
        app->framebuffer_resized = 0;
        app_recreate_swap_chain(app);
        return;
    } else if (result != VK_SUCCESS) {
        printf("Failed to acquire swap chain! \n");
    }
    vkResetFences(app->device, 1, &app->in_flight_fences[app->current_frame]);

    vkResetCommandBuffer(app->command_buffers[app->current_frame], 0);

    app_record_command_buffer(app, &app->command_buffers[app->current_frame], image_index);
    const VkSemaphore wait_semaphores[] = {app->image_available_semaphores[app->current_frame]};
    const VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    const VkSemaphore signal_semaphores[] = {app->render_finished_semaphores[app->current_frame]};

    const VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = wait_semaphores,
        .pWaitDstStageMask = wait_stages,
        .commandBufferCount = 1,
        .pCommandBuffers = &app->command_buffers[app->current_frame],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = signal_semaphores,
    };

    if (vkQueueSubmit(app->graphics_queue, 1, &submit_info, app->in_flight_fences[app->current_frame]) != VK_SUCCESS) {
        printf("Failed to submit draw buffer! \n");
    }

    const VkSwapchainKHR swap_chains[] = {app->swap_chain.instance};

    const VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signal_semaphores,
        .swapchainCount = 1,
        .pSwapchains = swap_chains,
        .pImageIndices = &image_index,
    };

    result = vkQueuePresentKHR(app->present_queue, &present_info);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        app_recreate_swap_chain(app);
    } else if (result != VK_SUCCESS) {
        printf("Failed to present swap chain image!\n");
    }



    app->current_frame = (app->current_frame + 1) % MAX_FRAMES_IN_FLIGHT;

}
void app_run(t_Application *app) {
    while(!glfwWindowShouldClose(app->window)) {
        glfwPollEvents();
        app_draw_frame(app);
    }
    vkDeviceWaitIdle(app->device);
}
void app_end(const t_Application *app) {
    app_cleanup_swap_chain(app);
    free(app->swap_chain.image_views);
    free(app->swap_chain.framebuffers);
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(app->device, app->render_finished_semaphores[i], NULL);
        vkDestroySemaphore(app->device, app->image_available_semaphores[i], NULL);
        vkDestroyFence(app->device, app->in_flight_fences[i], NULL);
    }
    free(app->command_buffers);
    free(app->image_available_semaphores);
    free(app->render_finished_semaphores);
    free(app->in_flight_fences);
    vkDestroyCommandPool(app->device, app->command_pool, NULL);
    for(size_t i = 0; i < app->swap_chain.image_count; i++) {
        vkDestroyFramebuffer(app->device, app->swap_chain.framebuffers[i], NULL);
    }
    vkDestroyPipeline(app->device, app->graphics_pipeline, NULL);
    vkDestroyPipelineLayout(app->device, app->pipeline_layout, NULL);
    vkDestroyRenderPass(app->device, app->render_pass, NULL);

    if (app->enable_validation_layers) {
        debug_destroy_utils_messenger_ext(app->vk_instance, app->debug_messenger, NULL);
    }
    free(app->swap_chain.images);
    vkDestroyDevice(app->device, NULL);
    vkDestroySurfaceKHR(app->vk_instance, app->surface, NULL);
    vkDestroyInstance(app->vk_instance, NULL);
    glfwDestroyWindow(app->window);
    glfwTerminate();
}