#pragma once

#include <kernel/types.h>

#define DTB_MAGIC 0xD00DFEED
#define DTB_MAX_RESERVATIONS 4096
#define DTB_BEGIN_NODE 0x1
#define DTB_END_NODE 0x2
#define DTB_PROPERTY 0x3
#define DTB_NOP 0x4
#define DTB_STRUCT_END 0x9


struct dt_header {
    uint32_t magic;
    uint32_t total_size;
    uint32_t off_dt_struct;
    uint32_t off_dt_strings;
    uint32_t off_mem_rsvmap;
    uint32_t version;
    uint32_t last_comp_version;
    uint32_t boot_cpuid_phys;
    uint32_t size_dt_strings;
    uint32_t size_dt_struct;
};

struct dt_addr_reservation {
    struct dt_addr_reservation* next;
    uint64_t address;
    uint64_t size;
};

struct dt_property {
    struct dt_property* next;
    char* key;
    char* value;
    uint32_t length;
};

struct dt_node {
    struct dt_node* children;
    struct dt_node* next;
    char* name;
    struct dt_property* properties;
};

struct device_tree {
    struct dt_header header;
    struct dt_addr_reservation* reservations;
    struct dt_node* root_node;
};


extern struct device_tree *kernel_dt;


void free_device_tree(struct device_tree* dt);
void free_device_tree_node(struct dt_node* node);

/**
 * @brief Loads the device tree from the specified header.
 * 
 * @param dtb_address Address of the raw dtb header pointer
 * @return dtb struct on success
 */
struct device_tree* load_device_tree(void *dtb_address);

/**
 * @brief Loads the device tree node. Called recursively.
 * 
 * @param node_tokens Address to first node token (including start node descriptor)
 * @param node_tokens_end Address to end of device tree structure
 * @param node Device tree node to populate
 * @param prop_strings Pointer to the property string array
 * @return Returns the incremented node token pointer (points to end token)
 */
uint32_t* load_device_tree_node(uint32_t *node_tokens, uint32_t *node_tokens_end, struct dt_node* node, char* prop_strings);

void print_device_tree(struct device_tree* dt);
void print_device_tree_node(struct dt_node* node, uint32_t depth);
