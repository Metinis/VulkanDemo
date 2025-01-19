#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <vulkan/vulkan_core.h>

typedef struct {
    uint32_t value;
    uint8_t has_value;
} OptionalUint32;
typedef struct QueueFamilyIndices {
    OptionalUint32 graphics_family;
    OptionalUint32 present_family;

}QueueFamilyIndices;

typedef struct {
    uint32_t *data;
    uint32_t count;
    uint32_t capacity;
} UniqueQueueSet;

typedef struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR *formats;
    uint32_t format_size;
    VkPresentModeKHR *present_modes;
    uint32_t present_size;
} SwapChainSupportDetails;

uint8_t is_complete(const QueueFamilyIndices* indices);

unsigned char* read_file(const char* filename, size_t* file_size);
