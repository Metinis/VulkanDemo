#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <stdio.h>
#include <cglm/mat4.h>

typedef struct GLFWwindow GLFWwindow;

typedef struct Application {
    GLFWwindow *window;
}t_Application;

void app_init(t_Application *app);

void app_run(const t_Application *app);

void app_end(const t_Application *app);