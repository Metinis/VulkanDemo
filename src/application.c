
#include "application.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

void app_init(t_Application *app) {

    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    app->window = glfwCreateWindow(1280,720, "VulkanDemo", nullptr, nullptr);

    uint32_t extensionCount = 0;

    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    printf("Extensions supported %d\n", extensionCount);
}
void app_run(const t_Application *app) {

    while(!glfwWindowShouldClose(app->window)) {
        glfwPollEvents();
    }
}
void app_end(const t_Application *app) {

    glfwDestroyWindow(app->window);
    glfwTerminate();
}
