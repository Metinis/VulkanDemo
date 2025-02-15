#include "vk_validation.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vk_debug.h"


t_Validation val_init(const uint8_t is_enabled) {
    t_Validation validation;
    const char *default_validation_layers[] = {"VK_LAYER_KHRONOS_validation"};
    validation.is_enabled = is_enabled;
    validation.size = 1;
    validation.layers = malloc(validation.size * sizeof(char *));
    for (size_t i = 0; i < validation.size; i++) {
        validation.layers[i] = default_validation_layers[i];
    }
    return validation;
}
uint8_t val_check_layer_support(const char** validation_layers, const size_t validation_size) {
    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, NULL);

    VkLayerProperties available_layers[layer_count];
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers);

    for(size_t i = 0; i < validation_size; i++) {
        uint8_t layer_found = 0;

        for(size_t j = 0; j < layer_count; j++) {
            if (strcmp(validation_layers[i], available_layers[j].layerName) == 0) {
                layer_found = 1;
                break;
            }
        }
        if(!layer_found) {
            return 0;
        }
    }
    return 1;
}
void val_enable(const t_Validation *validation, VkInstanceCreateInfo* create_info) {

    if (val_check_layer_support(validation->layers, validation->size)) {
        create_info->enabledLayerCount = (uint32_t)(validation->size);
        create_info->ppEnabledLayerNames = validation->layers;
        printf("Validation layers enabled! \n");
    } else {
        create_info->enabledLayerCount = 0;
        create_info->ppEnabledLayerNames = NULL;
    }

}
void val_cleanup(const t_Validation *validation, const VkInstance *instance, const VkDebugUtilsMessengerEXT *debug_messenger) {
    if (validation->is_enabled) {
        debug_destroy_utils_messenger_ext(*instance, *debug_messenger, NULL);
    }
    free(validation->layers);
}