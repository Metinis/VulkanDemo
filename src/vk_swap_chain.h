#pragma once
#include "utils.h"
#include "GLFW/glfw3.h"

//typedef struct Application t_Application;
typedef struct SwapChain {
    VkSwapchainKHR instance;
    VkImage* images;
    VkFormat image_format;
    uint32_t image_count;
    VkExtent2D extent;
    VkImageView* image_views;
    VkFramebuffer* framebuffers;
}t_SwapChain;

void swap_chain_query_support_details(t_SwapChainSupportDetails *details, const VkSurfaceKHR *surface, const VkPhysicalDevice *device);
void swap_chain_free_support(t_SwapChainSupportDetails *details);
t_SwapChain swap_chain_create(const VkSurfaceKHR *surface, const VkDevice *device, const VkPhysicalDevice *physical_device,
    GLFWwindow *window, const t_QueueFamilyIndices *indices);
void swap_chain_cleanup(const t_SwapChain *swap_chain, const VkDevice *device);
void swap_chain_recreate(t_SwapChain *swap_chain, const VkSurfaceKHR *surface, const VkDevice *device, const VkPhysicalDevice *physical_device,
    GLFWwindow *window, const t_QueueFamilyIndices *indices, const VkRenderPass *render_pass);
void swap_chain_create_image_views(t_SwapChain *swap_chain, const VkDevice *device);
void swap_chain_create_frame_buffers(t_SwapChain *swap_chain, const VkDevice *device, const VkRenderPass *render_pass);
VkImageView create_image_view(VkImage image, VkFormat format, const VkDevice *device);