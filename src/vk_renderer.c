#include "vk_renderer.h"

#include <stdio.h>
#include <stdlib.h>



static void renderer_create_command_pool(t_Renderer *renderer, const VkDevice *device, const t_QueueFamilyIndices *indices) {
    const VkCommandPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = indices->graphics_family.value
    };

    if (vkCreateCommandPool(*device, &pool_info, NULL, &renderer->command_pool) != VK_SUCCESS) {
        printf("Failed to create command pool! \n");
    }
}
static void renderer_create_command_buffers(t_Renderer *renderer, const VkDevice *device) {
    renderer->command_buffers = (VkCommandBuffer*)malloc(sizeof(VkCommandBuffer) * MAX_FRAMES_IN_FLIGHT);
    const VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = renderer->command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = MAX_FRAMES_IN_FLIGHT,
    };

    if (vkAllocateCommandBuffers(*device, &alloc_info, renderer->command_buffers) != VK_SUCCESS) {
        printf("Failed to create command buffer! \n");
    }

}
void renderer_record_command_buffer(const VkCommandBuffer *command_buffer, const uint32_t image_index, const t_Pipeline *pipeline,
    const t_SwapChain *swap_chain, const VkBuffer vertex_buffer, const uint32_t vertice_size) {
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
        .renderPass = pipeline->render_pass,
        .framebuffer = swap_chain->framebuffers[image_index],
        .renderArea.offset = {0, 0},
        .renderArea.extent = swap_chain->extent,
        .clearValueCount = 1,
        .pClearValues = &clear_color
    };

    vkCmdBeginRenderPass(*command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(*command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->graphics_pipeline);

    VkBuffer vertexBuffers[] = {vertex_buffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(*command_buffer, 0, 1, vertexBuffers, offsets);

    vkCmdDraw(*command_buffer, vertice_size, 1, 0, 0);


    const VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = (float)(swap_chain->extent.width),
        .height = (float)(swap_chain->extent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };
    vkCmdSetViewport(*command_buffer, 0, 1, &viewport);

    const VkRect2D scissor = {
    .offset = {0, 0},
    .extent = swap_chain->extent,
    };
    vkCmdSetScissor(*command_buffer, 0, 1, &scissor);

    vkCmdDraw(*command_buffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(*command_buffer);

    if (vkEndCommandBuffer(*command_buffer) != VK_SUCCESS) {
        printf("Failed to end command buffer! \n");
    }

}
static void renderer_create_sync_objects(t_Renderer *renderer, const VkDevice *device) {
    renderer->image_available_semaphores = (VkSemaphore*)malloc(sizeof(VkSemaphore) * MAX_FRAMES_IN_FLIGHT);
    renderer->render_finished_semaphores = (VkSemaphore*)malloc(sizeof(VkSemaphore) * MAX_FRAMES_IN_FLIGHT);
    renderer->in_flight_fences = (VkFence*)malloc(sizeof(VkFence) * MAX_FRAMES_IN_FLIGHT);

    const VkSemaphoreCreateInfo semaphore_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };
    const VkFenceCreateInfo fence_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(*device, &semaphore_info, NULL, &renderer->image_available_semaphores[i]) != VK_SUCCESS ||
        vkCreateSemaphore(*device, &semaphore_info, NULL, &renderer->render_finished_semaphores[i]) != VK_SUCCESS ||
        vkCreateFence(*device, &fence_info, NULL, &renderer->in_flight_fences[i]) != VK_SUCCESS) {
            printf("Failed to create semaphores! \n");
        }
    }
}
t_Renderer renderer_init(const t_QueueFamilyIndices *indices, const VkDevice *device) {
    t_Renderer renderer;
    renderer.current_frame = 0;
    renderer.framebuffer_resized = 0;
    renderer_create_command_pool(&renderer, device, indices);
    renderer_create_command_buffers(&renderer, device);
    renderer_create_sync_objects(&renderer, device);
    return renderer;
}
void renderer_draw_frame(t_Renderer *renderer, const t_Device *device, const t_SwapChain *swap_chain, GLFWwindow *window,
    const t_QueueFamilyIndices *indices, const t_Pipeline *pipeline, const VkBuffer *vertex_buffer, const uint32_t vertice_size) {
    vkWaitForFences(device->instance, 1, &renderer->in_flight_fences[renderer->current_frame], VK_TRUE, UINT64_MAX);

    uint32_t image_index;
    VkResult result = vkAcquireNextImageKHR(device->instance, swap_chain->instance, UINT64_MAX, renderer->image_available_semaphores[renderer->current_frame], VK_NULL_HANDLE, &image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || renderer->framebuffer_resized) {
        renderer->framebuffer_resized = 0;
        swap_chain_recreate(swap_chain, &device->surface, &device->instance, &device->physical_device, window, indices);
        return;
    } else if (result != VK_SUCCESS) {
        printf("Failed to acquire swap chain! \n");
    }
    vkResetFences(device->instance, 1, &renderer->in_flight_fences[renderer->current_frame]);

    vkResetCommandBuffer(renderer->command_buffers[renderer->current_frame], 0);

    renderer_record_command_buffer(&renderer->command_buffers[renderer->current_frame], image_index, pipeline, swap_chain, *vertex_buffer, vertice_size);
    const VkSemaphore wait_semaphores[] = {renderer->image_available_semaphores[renderer->current_frame]};
    const VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    const VkSemaphore signal_semaphores[] = {renderer->render_finished_semaphores[renderer->current_frame]};

    const VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = wait_semaphores,
        .pWaitDstStageMask = wait_stages,
        .commandBufferCount = 1,
        .pCommandBuffers = &renderer->command_buffers[renderer->current_frame],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = signal_semaphores,
    };

    if (vkQueueSubmit(device->graphics_queue, 1, &submit_info, renderer->in_flight_fences[renderer->current_frame]) != VK_SUCCESS) {
        printf("Failed to submit draw buffer! \n");
    }

    const VkSwapchainKHR swap_chains[] = {swap_chain->instance};

    const VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signal_semaphores,
        .swapchainCount = 1,
        .pSwapchains = swap_chains,
        .pImageIndices = &image_index,
    };

    result = vkQueuePresentKHR(device->present_queue, &present_info);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        swap_chain_recreate(swap_chain, &device->surface, &device->instance, &device->physical_device, window, indices);
    } else if (result != VK_SUCCESS) {
        printf("Failed to present swap chain image!\n");
    }

    renderer->current_frame = (renderer->current_frame + 1) % MAX_FRAMES_IN_FLIGHT;

}
void renderer_cleanup(const t_Renderer *renderer, const VkDevice *device) {
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(*device, renderer->render_finished_semaphores[i], NULL);
        vkDestroySemaphore(*device, renderer->image_available_semaphores[i], NULL);
        vkDestroyFence(*device, renderer->in_flight_fences[i], NULL);
    }
    free(renderer->command_buffers);
    free(renderer->image_available_semaphores);
    free(renderer->render_finished_semaphores);
    free(renderer->in_flight_fences);

    vkDestroyCommandPool(*device, renderer->command_pool, NULL);
}