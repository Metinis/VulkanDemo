#pragma once
#include <stdint.h>

#include "utils.h"

typedef struct Validation {
    uint8_t enable_validation_layers;
    const char **validation_layers;
    uint32_t validation_size;
}t_Validation;

t_Validation val_init(uint8_t is_enabled);

uint8_t val_check_layer_support(const char** validation_layers, size_t validation_size);

void val_enable(const t_Validation *validation, VkInstanceCreateInfo* create_info);

void val_cleanup(const t_Validation *validation, const VkInstance *instance, const VkDebugUtilsMessengerEXT *debug_messenger);