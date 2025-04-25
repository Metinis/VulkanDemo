#pragma once
#include "../utils.h"
#include "vk_depth.h"
#include "GLFW/glfw3.h"
#include "vk_color.h"

//typedef struct Application t_Application;
typedef struct ColorData t_ColorData;
typedef struct DepthData t_DepthData;
typedef struct Device t_Device;
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
void swap_chain_cleanup(const t_SwapChain *swap_chain, const t_DepthData *depth_data, const t_ColorData *color_data, const VkDevice *device);
void swap_chain_recreate(t_SwapChain *swap_chain, t_DepthData *depth_data, t_ColorData *color_data, const VkSurfaceKHR *surface, const t_Device *device,
    GLFWwindow *window, const t_QueueFamilyIndices *indices, const VkRenderPass *render_pass);
void swap_chain_create_image_views(t_SwapChain *swap_chain, const VkDevice *device);
void swap_chain_create_frame_buffers(t_SwapChain *swap_chain, const VkDevice *device, const VkRenderPass *render_pass, const VkImageView *depth_image_view, const VkImageView *color_image_view);
VkImageView create_image_view(VkImage image, VkFormat format, VkImageAspectFlags aspect_flags, uint32_t mip_levels, const VkDevice *device);