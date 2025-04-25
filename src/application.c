#include "application.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdio.h>
#include "vulkan/vk_debug.h"
#include "vulkan/vk_renderer.h"

static void framebuffer_resize_callback(GLFWwindow *window, int width, int height) {
    t_Application *app = (t_Application *)glfwGetWindowUserPointer(window);

    vulkan_handle_resize(&app->vulkan_core);
}
void app_init(t_Application *app) {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    app->window = glfwCreateWindow(800 ,600, "VulkanDemo", NULL, NULL);
    glfwMakeContextCurrent(app->window);

    glfwSetWindowUserPointer(app->window, app);
    glfwSetFramebufferSizeCallback(app->window, framebuffer_resize_callback);

    t_VulkanCore vulkan_core;
    vulkan_core_init(&vulkan_core, app->window);
    app->vulkan_core = vulkan_core;
}
void app_run(t_Application *app) {
    while(!glfwWindowShouldClose(app->window)) {
        glfwPollEvents();
        vulkan_handle_draw_frame(&app->vulkan_core, app->window);
    }
    vulkan_handle_exit(&app->vulkan_core);
}
void app_end(const t_Application *app) {
    glfwDestroyWindow(app->window);
    glfwTerminate();
}