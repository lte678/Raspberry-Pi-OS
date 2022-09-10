#include <kernel/pagetable.h>

#include <kernel/page.h>
#include <kernel/print.h>
#include <kernel/alloc.h>

// There are 4 page table levels. 0 is the uppermost level. 3 is the most detailled level


static uint16_t address_index(uint64_t addr, uint8_t level) {
    uint64_t index = addr >> ((3 - level) * 9 + 12);
    return index & (PAGE_TABLE_ENTRIES - 1);
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
            uint16_t table_index = address_index(current_address, alignment);
            if((current_table[table_index] & 0b11) == 0b01) {
                // Page table leaf node
                print("Conflicting entry already present in page table! (address: %p)\r\n", current_address);
                return -1;
            } else if((current_table[table_index] & 0b11) == 0b11) {
                // Page table index to next table -> follow
                current_table = current_table[table_index] & ~0b11;
            } else {
                // No entry present. Create a new table and point to it.
                uint64_t *new_table = kmalloc(PAGE_SIZE, ALLOC_ZERO_INIT);
                page_table_insert_table_descriptor(current_table, new_table, current_address, i);
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
 * Insert a new table descriptor into the page table.
 * @param table Pointer to the table we are modifing
 * @param next_table Pointer to the table we are mapping
 * @param target_address The virtual/block/table address we are inserting
 * @param level The level of the table we are inserting into
 */
void page_table_insert_table_descriptor(uint64_t* table, uint64_t* next_table, void* target_address, int level) {
    uint16_t index = address_index((uint64_t)target_address, level);
    uint64_t descriptor = (uint64_t)next_table;
    // Valid bit and index to next table
    descriptor |= 0b11;
    table[index] = descriptor;
}


/*!
 * Insert a new block or leaf descriptor into the page table.
 * @param table Pointer to the table we are modifing
 * @param pa The physical address to map to
 * @param va The virtual address we are mapping
 * @param level The level of the table we are inserting into
 */
void page_table_insert_descriptor(uint64_t* table, void* pa, void* va, int level) {
    uint16_t index = address_index((uint64_t)va, level);
    uint64_t descriptor = (uint64_t)pa;
    // Valid bit
    descriptor |= 0b1;
    descriptor |= 0b1 << 10;
    table[index] = descriptor;
    // MAIR index = 0b000, default permisions: allow all access.
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
    print("Perfect alignment\r\n");
    return level + 1;
}


uint64_t page_table_block_size(int level) {
    return 1ul << ((3 - level) * 9 + 12);
}