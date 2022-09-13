#include <kernel/address_space.h>
#include <kernel/pagetable.h>
#include <kernel/alloc.h>
#include <kernel/print.h>
#include <kernel/page.h>


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
    new_map->next = aspace->mappings;
    aspace->mappings = new_map;

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
    s->page_table = kmalloc(PAGE_SIZE, ALLOC_ZERO_INIT);
    if(!s->page_table) {
        free(s);
        return 0;
    }
    return s;
}
