#pragma once
#include "utils.h"
#include "GLFW/glfw3.h"

typedef struct Application t_Application;
typedef struct SwapChain {
    VkSwapchainKHR instance;
    VkImage* images;
    VkFormat image_format;
    uint32_t image_count;
    VkExtent2D extent;
    VkImageView* image_views;
    VkFramebuffer* framebuffers;
}SwapChain;

void swap_chain_query_support_details(SwapChainSupportDetails *details, VkSurfaceKHR *surface, const VkPhysicalDevice *device);
void swap_chain_free_support(SwapChainSupportDetails *details);
SwapChain swap_chain_create(VkSurfaceKHR *surface, const VkDevice *device, const VkPhysicalDevice *physical_device,
    GLFWwindow *window, const QueueFamilyIndices *indices);