cmake_minimum_required(VERSION 3.28)
project(VulkanDemo C)

set(CMAKE_C_STANDARD 23)

# Add GLFW include directory
include_directories(include/glfw/include)

# Add GLFW source files (assuming it's in include/glfw)
add_subdirectory(include/glfw)

# Add cglm include directory
include_directories(include/cglm/include)

include_directories(include/stb)

include_directories(include/tinyobj_loader_c)

find_package(Vulkan REQUIRED)

# Add VulkanDemo executable
add_executable(VulkanDemo src/main.c
        src/application.c
        src/vulkan/vk_debug.c
        src/vulkan/vk_debug.h
        src/utils.c
        src/utils.h
        src/vulkan/vk_swap_chain.c
        src/vulkan/vk_swap_chain.h
        src/vulkan/vk_pipeline.c
        src/vulkan/vk_pipeline.h
        src/vulkan/vk_device.c
        src/vulkan/vk_device.h
        src/vulkan/vk_validation.c
        src/vulkan/vk_validation.h
        src/vulkan/vk_buffers.c
        src/vulkan/vk_buffers.h
        src/vulkan/vk_renderer.c
        src/vulkan/vk_renderer.h
        src/vulkan/vk_device.h
        src/vulkan/vk_descriptor.c
        src/vulkan/vk_descriptor.h
        include/stb/stb_image.h
        src/texture.c
        src/texture.h
        src/vulkan/vk_depth.c
        src/vulkan/vk_depth.h
        include/tinyobj_loader_c/tinyobj_loader_c.h
        src/model.c
        src/model.h
        src/vulkan/vk_color.c
        src/vulkan/vk_color.h
        src/vulkan/vk_core.c
        src/vulkan/vk_core.h)

# Link GLFW
target_link_libraries(VulkanDemo glfw Vulkan::Vulkan)
