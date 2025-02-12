#include "utils.h"

#include <stdio.h>
#include <string.h>

uint8_t is_complete(const t_QueueFamilyIndices* indices) {
    return indices->graphics_family.has_value && indices->present_family.has_value;
}
unsigned char* read_file(const char* filename, size_t* file_size) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Failed to open file: %s\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    *file_size = ftell(file);
    rewind(file);

    unsigned char* buffer = (unsigned char*)malloc(*file_size);
    if (!buffer) {
        fprintf(stderr, "Failed to allocate memory for file contents.\n");
        fclose(file);
        return NULL;
    }

    fread(buffer, 1, *file_size, file);
    fclose(file);

    return buffer;
}
VkFormat find_supported_format(const VkFormat* candidates, const size_t candidate_size, const VkImageTiling tiling, const VkFormatFeatureFlags features,
                                      const VkPhysicalDevice *physical_device) {

    for(size_t i = 0; i < candidate_size; i++) {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(*physical_device, candidates[i], &properties);

        if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features) {
            return candidates[i];
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features) {
            return candidates[i];
        }
    }
    printf("Failed to find supported format! \n");
    return -1;
}


// Hash function for Vertex
unsigned int hash_vertex(const t_Vertex* vertex, size_t table_size) {
    unsigned int hash_value = 0;
    const unsigned char* data = (const unsigned char*)vertex;
    size_t size = sizeof(t_Vertex);

    for (size_t i = 0; i < size; i++) {
        hash_value = hash_value * 31 + data[i];
    }

    return hash_value % table_size;
}

// Compare two Vertex structures for equality
int vertex_equals(const t_Vertex* a, const t_Vertex* b) {
    return memcmp(a, b, sizeof(t_Vertex)) == 0;
}

// Create a new node
Node* create_node(const t_Vertex* key, uint32_t value) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->pair.key = *key; // Copy the key
    node->pair.value = value;
    node->next = NULL;
    return node;
}

// Initialize the hash table
void init_table(HashTable* table, size_t size) {
    table->buckets = (Node**)calloc(size, sizeof(Node*));
    table->size = size;
    table->count = 0;
}

// Free the hash table
void free_table(HashTable* table) {
    for (size_t i = 0; i < table->size; i++) {
        Node* current = table->buckets[i];
        while (current != NULL) {
            Node* temp = current;
            current = current->next;
            free(temp); // Free the node
        }
    }
    free(table->buckets); // Free the array of buckets
}

// Resize the hash table
void resize_table(HashTable* table, size_t new_size) {
    Node** new_buckets = (Node**)calloc(new_size, sizeof(Node*));

    // Rehash all elements into the new buckets
    for (size_t i = 0; i < table->size; i++) {
        Node* current = table->buckets[i];
        while (current != NULL) {
            Node* next = current->next;
            unsigned int new_index = hash_vertex(&current->pair.key, new_size);

            // Insert at the head of the new bucket
            current->next = new_buckets[new_index];
            new_buckets[new_index] = current;

            current = next;
        }
    }

    free(table->buckets); // Free the old buckets
    table->buckets = new_buckets;
    table->size = new_size;
}

// Insert a key-value pair into the hash table
void insert(HashTable* table, const t_Vertex* key, uint32_t value) {
    // Resize the table if the load factor exceeds the threshold
    if ((double)table->count / table->size > LOAD_FACTOR_THRESHOLD) {
        resize_table(table, table->size * 2);
    }

    unsigned int index = hash_vertex(key, table->size);
    Node* node = create_node(key, value);

    // Insert at the head of the linked list
    node->next = table->buckets[index];
    table->buckets[index] = node;
    table->count++;
}

// Search for a key in the hash table
int search(HashTable* table, const t_Vertex* key, uint32_t* value) {
    unsigned int index = hash_vertex(key, table->size);
    Node* current = table->buckets[index];

    // Traverse the linked list
    while (current != NULL) {
        if (vertex_equals(&current->pair.key, key)) {
            *value = current->pair.value; // Key found
            return 1;
        }
        current = current->next;
    }

    return 0; // Key not found
}
