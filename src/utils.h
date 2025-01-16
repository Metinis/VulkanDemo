#pragma once
#include <stdint.h>

typedef struct {
    uint32_t value;
    uint8_t has_value;
} OptionalUint32;
typedef struct QueueFamilyIndices {
    OptionalUint32 graphicsFamily;

}QueueFamilyIndices;