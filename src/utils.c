#include "utils.h"

uint8_t is_complete(const QueueFamilyIndices* indices) {
    return indices->graphics_family.has_value && indices->present_family.has_value;
}
void unique_set_init(UniqueQueueSet *set, uint32_t initial_capacity) {
    set->data = (uint32_t *)malloc(initial_capacity * sizeof(uint32_t));
    set->count = 0;
    set->capacity = initial_capacity;
}

void unique_set_free(UniqueQueueSet *set) {
    free(set->data);
    set->data = NULL;
    set->count = 0;
    set->capacity = 0;
}

int unique_set_contains(UniqueQueueSet *set, uint32_t value) {
    for (uint32_t i = 0; i < set->count; ++i) {
        if (set->data[i] == value) {
            return 1;
        }
    }
    return 0;
}

void unique_set_add(UniqueQueueSet *set, uint32_t value) {
    if (!unique_set_contains(set, value)) {
        if (set->count == set->capacity) {
            set->capacity *= 2;
            set->data = (uint32_t *)realloc(set->data, set->capacity * sizeof(uint32_t));
        }
        set->data[set->count++] = value;
    }
}
