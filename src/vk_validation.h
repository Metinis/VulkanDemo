#pragma once
#include <stdint.h>

#include "utils.h"
#ifdef NDEBUG
    #define ENABLE_VALIDATION_LAYERS 0
#else
    #define ENABLE_VALIDATION_LAYERS 1
#endif

typedef struct Validation {
    const char **validation_layers;
    uint32_t validation_size;
}t_Validation;

t_Validation val_init();

uint8_t val_check_layer_support(const char** validation_layers, size_t validation_size);

void val_enable(const t_Validation *validation, VkInstanceCreateInfo* create_info);

void val_cleanup(const t_Validation *validation, const VkInstance *instance, const VkDebugUtilsMessengerEXT *debug_messenger);