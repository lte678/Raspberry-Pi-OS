#include <kernel/pagetable.h>

#include <kernel/page.h>
#include <kernel/print.h>
#include <kernel/alloc.h>

// There are 4 page table levels. 0 is the uppermost level. 3 is the most detailled level

extern uint64_t *_kernel_page_table;
uint64_t *kernel_page_table;

static uint16_t address_index(uint64_t addr, uint8_t level) {
    uint64_t index = addr >> ((3 - level) * 9 + 12);
    return index & (PAGE_TABLE_ENTRIES - 1);
}


static int is_table_entry(uint64_t entry) {
    return (entry & 0b11) == 0b11;
}


static int is_leaf_entry(uint64_t entry, int8_t level) {
    if(level == 3) {
        return (entry & 0b11) == 0b11;
    } else {
        return (entry & 0b11) == 0b01;
    }
}


static uint64_t get_address_from_descriptor(uint64_t desc) {
    return desc & 0x0000FFFFFFFFF000ul;
}

//static void tlb_invalidate_el1() {
//	__asm__ __volatile__("tlbi alle1" : : : "memory");
//}

void page_table_init() {
    kernel_page_table = *(uint64_t**)((char*)&_kernel_page_table + VA_OFFSET);
}

/*!
 * Insert a new table descriptor into the page table.
 * @param table Pointer to the table we are modifing. Must be a kernel address!
 * @param next_table Pointer to the table we are mapping. Must be a physical address!
 * @param target_address The virtual/block/table address we are inserting
 * @param level The level of the table we are inserting into
 */
static void page_table_insert_table_descriptor(uint64_t* table, uint64_t* next_table, uint64_t target_address, int level) {
    uint16_t index = address_index(target_address, level);
    uint64_t descriptor = (uint64_t)next_table;
    // Valid bit and index to next table
    descriptor |= 0b11;
    table[index] = descriptor;
}


/*!
 * Insert a new block or leaf descriptor into the page table.
 * @param table Pointer to the table we are modifing. Must be a kernel address!
 * @param pa The physical address to map to
 * @param va The virtual address we are mapping
 * @param level The level of the table we are inserting into
 */
static void page_table_insert_descriptor(uint64_t* table, uint64_t pa, uint64_t va, int level) {
    uint16_t index = address_index(va, level);
    uint64_t descriptor = pa;
    // Valid bit
    descriptor |= 0b1;
    // Level 3 tables confusingly require the second bit to be set.
    // This works because table descriptors are not valid at this level
    if(level == 3) {
        descriptor |= 0b1 << 1;
    }
    descriptor |= 0b1 << 10;
    // Allow access from EL0
    descriptor |= 0b1 << 6;
    table[index] = descriptor;
    // MAIR index = 0b000, default permisions: allow all access.
}


/**
 * @brief Traverses the supplied page table to translate the virtual address.
 * 
 * @param table A top level page table. Must be a kernel address!
 * @param a Address
 * @return Translated address 
 */
uint64_t page_table_virtual_to_physical(uint64_t* table, uint64_t a) {
    for(int level = 0; level <= 3; level++) {
        uint16_t pgt_index = address_index(a, level);
        uint64_t entry = table[pgt_index];
        if(is_table_entry(entry)) {
            table = PT_ADDRESS_TO_KERN((uint64_t*)get_address_from_descriptor(entry));
        } else if(is_leaf_entry(entry, level)) {
            uint64_t base_address = get_address_from_descriptor(entry);
            base_address |= ((1ul << ((3 - level)*9 + 12)) - 1ul) & a;
            return base_address;
        } else {
            // This happens when the address is not mapped and the entry 
            // is null, for example.
            return 0;
        }
    }
    return 0;
}


/*!
 * Insert an address mapping for the specified range.
 * @param root_table Pointer to the page table we are modifing
 * @param pa The physical address we are mapping to
 * @param va The virtual address we are inserting
 * @param size The length of the mapping
 */
int page_table_map_address(uint64_t* root_table, uint64_t pa, uint64_t va, uint64_t size) {
    if((pa & (PAGE_SIZE - 1)) || (va & (PAGE_SIZE - 1)) || (size & (PAGE_SIZE - 1))) {
        print("Failed to map page table address, invalid page alignment!\r\n");
        return -1;
    }

    uint64_t current_address = va;
    while(current_address < (va + size)) {
        // The largest block according to the alignment
        // Alignment should never equal -1, since we checked the address alignment at the start of the function.
        int alignment = page_table_alignment(current_address);
        // The largest block according to the remaining space to map
        uint64_t block_size = page_table_block_size(alignment);
        while(block_size > (va + size - current_address)) {
            alignment++;
            block_size = page_table_block_size(alignment);
        }
        // We found the largest block suitable block
        // Generate page table entries
        uint64_t* current_table = root_table;
        for(int i = 0; i < alignment; i++) {
            uint16_t table_index = address_index(current_address, i);
            if(is_leaf_entry(current_table[table_index], i)) {
                // Page table leaf node
                print("Conflicting entry already present in page table! (address: %p)\r\n", current_address);
                return -1;
            } else if(is_table_entry(current_table[table_index])) {
                // Page table index to next table -> follow
                current_table = PT_ADDRESS_TO_KERN((uint64_t*)(current_table[table_index] & ~0b11));
            } else {
                // No entry present. Create a new table and point to it
                uint64_t *new_table = kmalloc(PAGE_SIZE, ALLOC_ZERO_INIT);
                page_table_insert_table_descriptor(current_table, KERN_TO_PHYS(new_table), current_address, i);
                current_table = new_table;
            }
        }
        // Create the leaf node.
        uint64_t offset  = current_address - va;
        page_table_insert_descriptor(current_table, pa + offset, current_address, alignment);

        current_address += block_size;
    }

    return 0;
}


/*!
 * Removes the mapping for the specified virtual address range.
 * @param root_table Pointer to the page table we are modifing
 * @param va The virtual address to remove
 * @param size The length of the mapping
 */
int page_table_unmap_address(uint64_t* root_table, uint64_t va, uint64_t size) {
    if((va & (PAGE_SIZE - 1)) || (size & (PAGE_SIZE - 1))) {
        print("Failed to unmap page table address, invalid page alignment!\r\n");
        return -1;
    }

    uint64_t current_address = va;
    while(current_address < (va + size)) {
        // Search for the current address in the page table
        // Page table level
        int i = 0;
        uint64_t* current_table = root_table;
        while(i <= PAGE_TABLE_LEVELS) {
            uint16_t table_index = address_index(current_address, i);
            if(is_leaf_entry(current_table[table_index], i)) {
                // Page table leaf node that we want to remove
                current_table[table_index] = 0;
                current_address += page_table_block_size(i);
                break;
            } else if(is_table_entry(current_table[table_index])) {
                // Page table index to next table -> follow
                current_table = PT_ADDRESS_TO_KERN((uint64_t*)(current_table[table_index] & ~0b11));
            } else {
                print("page_table_unmap_address: warning: attempting to unmap non-mapped regions.\r\n");
                current_address += page_table_block_size(i);
                break;
            }
            i++;
        }
    }
    // TODO: Why does this cause an exception?
    //tlb_invalidate_el1();
    return 0;
}


/*!
 * Returns the addresses page table alignment level.
 * @param address
 * @return Alignment level ({-1, 0, 1, 2, 3}). -1 corresponds to no alignment.
 */
int page_table_alignment(uint64_t address) {
    int level = 3;
    while(level >= 0) {
        // First compute the mask for the current level
        // ==1 for all bits that may not contain information
        uint64_t mask = page_table_block_size(level) - 1ul;
        if(address & mask) {
            // Return previous table
            if(level == 3) { 
                return -1;
            }
            return level + 1;
        }
        level--;
    }
    return level + 1;
}


uint64_t page_table_block_size(int level) {
    return 1ul << ((3 - level) * 9 + 12);
}


static void page_table_print_section(uint64_t* table, int depth) {
    int continuous = 0;
    for(int i = 0; i < 512; i++) {
        if(table[i] & 0b11) {
            // The entry is present
            for(int d = 0; d < depth; d++) print("  ");
            if(continuous >= 3) {
                print("...\r\n");
                i = 511;
                for(int d = 0; d < depth; d++) print("  ");
            }

            print("{d}", i);
                        if(i < 10) {
                print("  ");
            } else if(i < 100) {
                print(" ");
            }
            print(": {xl}\r\n",table[i]);

            // Current page table level == depth - 1
            if(depth - 1 < 3 && is_table_entry(table[i])) {
                // Points to another table
                uint64_t* next_table = PT_ADDRESS_TO_KERN((uint64_t*)get_address_from_descriptor(table[i]));
                page_table_print_section(next_table, depth + 1);
            }
            if(continuous >= 3) {
                return;
            }
        } else {
            continuous = -1;
        }
        if(continuous >= 0) {
            continuous++;
        }
    }
}

/**
 * @brief Prints the page table.
 * 
 * @param table Kernel address of table
 */
void page_table_print(uint64_t* table) {
    print("Page table @ {p}\r\n", table);
    page_table_print_section(table, 1);
}