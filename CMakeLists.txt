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
        src/vulkan_debug.c
        src/vulkan_debug.h
        src/utils.c
        src/utils.h
        src/swap_chain.c
        src/swap_chain.h)

# Link GLFW
target_link_libraries(VulkanDemo glfw Vulkan::Vulkan)
