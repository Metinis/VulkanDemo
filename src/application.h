#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <vulkan/vulkan_core.h>
#include "vulkan/vk_swap_chain.h"
#include "utils.h"
#include "vulkan/vk_core.h"

typedef struct GLFWwindow GLFWwindow;

typedef struct Application {
    GLFWwindow *window;
    t_VulkanCore vulkan_core;
}t_Application;

void app_init(t_Application *app);
void app_run(t_Application *app);
void app_end(const t_Application *app);