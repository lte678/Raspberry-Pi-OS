#include <kernel/device_tree.h>

#include <kernel/bitmanip.h>
#include <kernel/alloc.h>
#include <kernel/print.h>
#include <kernel/mem.h>

struct device_tree *kernel_dt;


void free_device_tree(struct device_tree* dt) {
    if(dt) {
        {
            struct dt_addr_reservation* i = dt->reservations;
            while(i) {
                struct dt_addr_reservation* next = i->next;
                free(i);
                i = next;
            }
        }
        free_device_tree_node(dt->root_node);
        free(dt);
    }
}

void free_device_tree_node(struct dt_node* node) {
    if(node) {
        // First free children
        {
            struct dt_node* i = node->children;
            while(i) {
                struct dt_node* next = i->next;
                free_device_tree_node(i);
                i = next;
            }
        }
        // Free properties
        {
            struct dt_property* i = node->properties;
            while(i) {
                struct dt_property* next = i->next;
                free(i->key);
                free(i->value);
                free(i);
                i = next;
            }
        }
        // Free other members
        free(node->name);
    }
}
struct device_tree* load_device_tree(void *dtb_address) {
    // Load header
    uint32_t *dtb_address32 = dtb_address;
    struct device_tree *dt = kmalloc(sizeof(struct device_tree), ALLOC_ZERO_INIT);
    if(!dt) {
        print("load_device_tree: allocation failed!\n");
        return 0;
    }
    dt->header.magic = __builtin_bswap32(dtb_address32[0]);
    dt->header.total_size = __builtin_bswap32(dtb_address32[1]);
    dt->header.off_dt_struct = __builtin_bswap32(dtb_address32[2]);
    dt->header.off_dt_strings = __builtin_bswap32(dtb_address32[3]);
    dt->header.off_mem_rsvmap = __builtin_bswap32(dtb_address32[4]);
    dt->header.version = __builtin_bswap32(dtb_address32[5]);
    dt->header.last_comp_version = __builtin_bswap32(dtb_address32[6]);
    dt->header.boot_cpuid_phys = __builtin_bswap32(dtb_address32[7]);
    dt->header.size_dt_strings = __builtin_bswap32(dtb_address32[8]);
    dt->header.size_dt_struct = __builtin_bswap32(dtb_address32[9]);

    if(dt->header.magic != DTB_MAGIC) {
        print("DTB magic is incorrect!\n");
        free_device_tree(dt);
        return 0;
    }
    if(dt->header.last_comp_version > 16) {
        print("DTB version >{x} is incompatible\n", dt->header.last_comp_version);
        free_device_tree(dt);
        return 0;
    }

    // Load memory reservations
    uint32_t i = 0;
    while(1) {
        uint64_t address = __builtin_bswap64(((uint64_t*)(dtb_address + dt->header.off_mem_rsvmap + i*16))[0]);
        uint64_t size = __builtin_bswap64(((uint64_t*)(dtb_address + dt->header.off_mem_rsvmap + i*16))[1]);
        if(address == 0 && size == 0) {
            break;
        }
        if(i >= DTB_MAX_RESERVATIONS) {
            print("Reached address reservation limit.\n");
            break;
        }
        struct dt_addr_reservation* new_rsv = kmalloc(16, 0);
        if(!new_rsv) {
            print("load_device_tree:2: allocation failed!\n");
            free_device_tree(dt);
            return 0;
        }
        new_rsv->address = address;
        new_rsv->size = size;
        // Prepend to linked list
        new_rsv->next = dt->reservations;
        dt->reservations = new_rsv;
        i++;
    }

    // Load device node structures 
    uint32_t* dt_token = dtb_address + dt->header.off_dt_struct;
    uint32_t* dt_token_end = dt_token + dt->header.size_dt_struct / 4;
    char* prop_strings = dtb_address + dt->header.off_dt_strings;
    // Allocate root node
    dt->root_node = kmalloc(sizeof(struct dt_node), ALLOC_ZERO_INIT);
    if(!dt->root_node) {
        print("Failed to allocate root node!\n");
        free_device_tree(dt);
        return 0;
    }
    if(load_device_tree_node(dt_token, dt_token_end, dt->root_node, prop_strings) == 0) {
        print("Failed to parse device tree structure\n");
        free_device_tree(dt);
        return 0;
    }

    return dt;
}

uint32_t* load_device_tree_node(uint32_t *node_tokens, uint32_t *node_tokens_end, struct dt_node* node, char* prop_strings) {
    if(*node_tokens == __builtin_bswap32(DTB_STRUCT_END)) {
        return node_tokens;
    }
    node_tokens += 1;
    // Before the next tokens begin, the node identifier string must be read
    uint32_t node_name_len = strlen((char*)node_tokens);
    node->name = kmalloc(node_name_len + 1, 0);
    if(!node->name) {
        return 0;
    }
    strncpy(node->name, (char*)node_tokens, node_name_len + 1);
    node_tokens += (node_name_len + 4) / 4;

    while(node_tokens < node_tokens_end) {
        uint32_t tok = __builtin_bswap32(*node_tokens);
        if(tok == DTB_BEGIN_NODE) {
            struct dt_node* new_node =  kmalloc(sizeof(struct dt_node), ALLOC_ZERO_INIT);
            if(!new_node) {
                return 0;
            }
            node_tokens = load_device_tree_node(node_tokens, node_tokens_end, new_node, prop_strings);
            if(!node_tokens) {
                return 0;
            }
            // Insert node into child linked list
            new_node->next = node->children;
            node->children = new_node;
        } else if(tok == DTB_END_NODE) {
            return node_tokens;
        } else if(tok == DTB_PROPERTY) {
            node_tokens += 1;
            uint32_t property_length = __builtin_bswap32(*node_tokens);
            node_tokens += 1;
            uint32_t property_name_off = __builtin_bswap32(*node_tokens);
            struct dt_property* new_property = kmalloc(sizeof(struct dt_property), 0);
            if(!new_property) {
                return 0;
            }
            char* new_property_key_ptr = prop_strings + property_name_off;
            new_property->length = property_length;
            new_property->value = kmalloc(property_length, 0);
            if(!new_property->value ) {
                free(new_property);
                return 0;
            }
            new_property->key = kmalloc(strlen(new_property_key_ptr) + 1, 0);
            if(!new_property->key) {
                free(new_property->value);
                free(new_property);
                return 0;
            }
            strncpy(new_property->key, new_property_key_ptr, strlen(new_property_key_ptr) + 1);
            memcpy(new_property->value, (char*)(node_tokens + 1), property_length);
            // Append new property to linked list
            new_property->next = node->properties;
            node->properties = new_property;
            node_tokens += (property_length + 3) / 4;

        } else if(tok == DTB_NOP) {
            // Do nothing
        } else {
            print("Unknown token {x}\n", __builtin_bswap32(*node_tokens));
        }
        
        node_tokens += 1;
    }
    print("Overflowed DTB struct!\n");
    return 0;
}

void print_device_tree(struct device_tree *dt) {
    print("Reserved memory regions:\n");
    {
        struct dt_addr_reservation* i = dt->reservations;
        if(!i) {
            print("  None\n");
        }
        while(i) {
            print("  0x{xl} bytes @ 0x{xl}\n", i->size, i->address);
            i = i->next;
        }
    }
    {
        print("Device Tree Structure:\n");
        print_device_tree_node(dt->root_node, 1);
    }
}

void print_device_tree_node(struct dt_node* node, uint32_t depth) {
    for(uint32_t i = 0; i < depth; i++) {
        print("  ");
    }
    if(*node->name) {
        print("{s} \\{\n", node->name);
    } else {
        print("(anonymous) \\{\n");
    }

    struct dt_property* p = node->properties;
    while(p) {
        for(uint32_t i = 0; i < depth + 1; i++) {
            print("  ");
        }
        char *value_string = escape_string(p->value, p->length);
        if(!value_string) {
            return;
        }
        print("{s} = {s};\n", p->key, value_string);
        free(value_string);
        p = p->next;
    }
    struct dt_node* i = node->children;
    while(i) {
        print_device_tree_node(i, depth + 1);
        i = i->next;
    }
    for(uint32_t i = 0; i < depth; i++) {
        print("  ");
    }
    print("\\};\n");
}