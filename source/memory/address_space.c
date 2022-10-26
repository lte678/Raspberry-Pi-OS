#include <kernel/address_space.h>
#include <kernel/pagetable.h>
#include <kernel/alloc.h>
#include <kernel/print.h>
#include <kernel/page.h>


struct address_space* kernel_address_space = 0;

/**
 * @brief Checks if two address mappings overlap
 * 
 * @param map1 
 * @param map2 
 * @return 1 if overlapping
 */
static int memory_region_overlaps(struct address_mapping *map1, struct address_mapping *map2) {
    // Check if start or end of region 1 is inside region 2
    if(map1->vaddress >= map2->vaddress && map1->vaddress < (map2->vaddress + map2->size)) {
        return 1;
    }
    if((map1->vaddress + map1->size) > map2->vaddress && (map1->vaddress + map1->size) <= (map2->vaddress + map2->size)) {
        return 1;
    }
    // If region 1 does not start within region 2, then it may still overlap it completely
    if(map1->vaddress < map2->vaddress && (map1->vaddress + map1->size) > (map2->vaddress + map2->size)) {
        return 1;
    }
    // Otherwise, region 1 does not overlap region 2
    return 0;
}


/**
 * @brief Maps virtual memory in the specified virtual address space.
 * 
 * @param aspace Address space containing the page tables to be modified.
 * @param vaddr The start of the virtual address block
 * @param paddr The start of the physical address block
 * @param size Size of the memory region to map
 * @return 0 for success
 */
int map_memory_region(struct address_space *aspace, uint64_t vaddr, uint64_t paddr, uint64_t size) {
    // Allocate new mapping
    struct address_mapping *new_map = kmalloc(sizeof(struct address_mapping), 0);
    new_map->next = 0;
    new_map->prev = 0;
    new_map->vaddress = vaddr;
    new_map->paddress = paddr;
    new_map->size = size;

    // Check for overlaps
    for(struct address_mapping *other = aspace->mappings; other; other = other->next) {
        if(memory_region_overlaps(new_map, other)) {
            print("Attempted to map overlapping memory region!\r\n");
            free(new_map);
            return 1;
        }
    }

    // No overlaps, add region to page table
    if(page_table_map_address(aspace->page_table, new_map->paddress, new_map->vaddress, new_map->size)) {
        free(new_map);
        return 1;
    }

    // Insert memory region into linked list
    if(aspace->mappings) {
        aspace->mappings->prev = new_map;
    }  
    new_map->next = aspace->mappings;
    aspace->mappings = new_map;

    return 0;
}

/**
 * @brief Unmaps virtual memory in the specified virtual address space.
 * 
 * @param aspace Address space containing the page tables to be modified.
 * @param mapping The mapping in aspace to unmap
 * @return 0 for success
 */
int unmap_memory_region(struct address_space *aspace, struct address_mapping *mapping) {
    // No overlaps, add region to page table
    if(page_table_unmap_address(aspace->page_table, mapping->vaddress, mapping->size)) {
        return 1;
    }
    // Remove from linked list, then free
    if(mapping->prev) {
        mapping->prev->next = mapping->next;
    } else {
        aspace->mappings = mapping->next;
    }
    if(mapping->next) {
        mapping->next->prev = mapping->prev;
    }

    free(mapping);
    return 0;
}



/**
 * @brief Allocates and returns an address_space struct pointer.
 */
struct address_space* allocate_address_space() {
    struct address_space *s = kmalloc(sizeof(struct address_space), ALLOC_ZERO_INIT);
    if(!s) {
        return 0;
    }
    //s->page_table = kmalloc(PAGE_SIZE, ALLOC_ZERO_INIT);
    //if(!s->page_table) {
    //    free(s);
    //    return 0;
    //}
    // This is because we cannot use an individual page table for each process!
    // Kind of terrible.
    // Each process uses the kernel page table
    s->page_table = kernel_page_table;
    return s;
}

/**
 * @brief Allocates and populates an address_space struct containing the mappings from early mem.
 * This includes the identity mapping, which can be removed from the page table by means of the mapping added here.
 * 
 * @param id_mapping Reference to a id_mapping pointer that will be populated with the identity mapping for later removal.
 * 
 * @return Kernel address space or null
 */
struct address_space* init_kernel_address_space_struct(struct address_mapping **id_mapping) {
    struct address_space* kaddrspace = allocate_address_space();
    if(!kaddrspace) {
        return 0;
    }

    struct address_mapping *new_map = kmalloc(sizeof(struct address_mapping), 0);
    if(!new_map) {
        free(kaddrspace);
        return 0;
    }
    struct address_mapping *new_id_map = kmalloc(sizeof(struct address_mapping), 0);
    if(!new_id_map) {
        free(kaddrspace);
        free(new_map);
        return 0;
    }

    new_id_map->next = new_map;
    new_id_map->prev = 0;
    new_id_map->vaddress = 0;
    new_id_map->paddress = 0;
    new_id_map->size = PAGE_SIZE * PAGE_TABLE_ENTRIES * PAGE_TABLE_ENTRIES;

     // Insert memory region into linked list
    kaddrspace->mappings = new_id_map;

    new_map->next = 0;
    new_map->prev = new_id_map;
    new_map->vaddress = VA_OFFSET;
    new_map->paddress = 0;
    new_map->size = PAGE_SIZE * PAGE_TABLE_ENTRIES * PAGE_TABLE_ENTRIES;

    kernel_address_space = kaddrspace;
    *id_mapping = new_id_map;
    return kaddrspace;
}