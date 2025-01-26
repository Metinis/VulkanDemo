cmake_minimum_required(VERSION 3.28)
project(VulkanDemo C)

set(CMAKE_C_STANDARD 23)

# Add GLFW include directory
include_directories(include/glfw/include)

# Add GLFW source files (assuming it's in include/glfw)
add_subdirectory(include/glfw)

# Add cglm include directory
include_directories(include/cglm/include)

find_package(Vulkan REQUIRED)

# Add VulkanDemo executable
add_executable(VulkanDemo src/main.c
        src/application.c
        src/vk_debug.c
        src/vk_debug.h
        src/utils.c
        src/utils.h
        src/vk_swap_chain.c
        src/vk_swap_chain.h
        src/vk_pipeline.c
        src/vk_pipeline.h
        src/vk_device.c
        src/vk_device.h
        src/vk_validation.c
        src/vk_validation.h
        src/vk_buffers.c
        src/vk_buffers.h
        src/vk_renderer.c
        src/vk_renderer.h
        src/vk_device.h)

# Link GLFW
target_link_libraries(VulkanDemo glfw Vulkan::Vulkan)
