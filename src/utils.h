#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <cglm/vec3.h>
#include <vulkan/vulkan_core.h>


typedef struct {
    uint32_t value;
    uint8_t has_value;
} t_OptionalUint32;
typedef struct QueueFamilyIndices {
    t_OptionalUint32 graphics_family;
    t_OptionalUint32 present_family;

}t_QueueFamilyIndices;

typedef struct {
    uint32_t *data;
    uint32_t count;
    uint32_t capacity;
} t_UniqueQueueSet;

typedef struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR *formats;
    uint32_t format_size;
    VkPresentModeKHR *present_modes;
    uint32_t present_size;
} t_SwapChainSupportDetails;
typedef struct Vertex {
    vec3 pos;
    vec3 color;
    vec2 tex_coord;
}t_Vertex;

// Structure for a key-value pair
typedef struct {
    t_Vertex key;
    uint32_t value;
} KeyValuePair;

// Structure for a node in the linked list
typedef struct Node {
    KeyValuePair pair;
    struct Node* next;
} Node;

// Structure for the hash table
typedef struct {
    Node** buckets; // Array of pointers to linked lists
    size_t size; // Current size of the hash table
    size_t count; // Number of elements in the hash table
} HashTable;

// Initial size of the hash table
#define INITIAL_SIZE 16

// Load factor threshold for resizing
#define LOAD_FACTOR_THRESHOLD 0.75

uint8_t is_complete(const t_QueueFamilyIndices* indices);

unsigned char* read_file(const char* filename, size_t* file_size);

VkFormat find_supported_format(const VkFormat* candidates, size_t candidate_size, VkImageTiling tiling, VkFormatFeatureFlags features,
                                      const VkPhysicalDevice *physical_device);

unsigned int hash_vertex(const t_Vertex* vertex, size_t table_size);

int vertex_equals(const t_Vertex* a, const t_Vertex* b);

Node* create_node(const t_Vertex* key, uint32_t value);

void init_table(HashTable* table, size_t size);

void free_table(HashTable* table);

void resize_table(HashTable* table, size_t new_size);

void insert(HashTable* table, const t_Vertex* key, uint32_t value);

int search(HashTable* table, const t_Vertex* key, uint32_t* value);