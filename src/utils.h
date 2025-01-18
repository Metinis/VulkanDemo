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

uint8_t is_complete(const QueueFamilyIndices* indices);

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

void unique_set_init(UniqueQueueSet *set, uint32_t initial_capacity);

void unique_set_free(UniqueQueueSet *set);

int unique_set_contains(UniqueQueueSet *set, uint32_t value);

void unique_set_add(UniqueQueueSet *set, uint32_t value);
