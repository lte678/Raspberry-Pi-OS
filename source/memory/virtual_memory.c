// This file is compiled with different linker settings, and may not be linked with other parts of the kernel!
// It operates in physical address space, and prepares the switch to kernel space.
// Many of these functions are therefore duplicated here and in pagetable.c for example.


#include <kernel/types.h>
#include <kernel/register.h>
#include <kernel/page.h>
#include <init_sysregs.h>


#define TCR_REG_DS_OFFSET    59
#define TCR_REG_IPS_OFFSET   32
#define TCR_REG_EPD1_OFFSET  23
#define TCR_REG_TG0_OFFSET   14
#define TCR_REG_EPD0_OFFSET  7
#define TCR_REG_T0SZ_OFFSET  0

// The _id tables are used for execution in the current pre-virtual context and are remapped after transferring execution.
static uint64_t page_table0[512] __attribute__((aligned(4096))) = {};
static uint64_t page_table1_id[512] __attribute__((aligned(4096))) = {};
static uint64_t page_table1[512] __attribute__((aligned(4096))) = {};
static uint64_t page_table2_id[512] __attribute__((aligned(4096))) = {};
static uint64_t page_table2[512] __attribute__((aligned(4096))) = {};

/*!
 * Configure the MAIR_EL1 register, which is indexed into by the processor
 * to find memory region permissions.
 * @param None
 */
static void page_table_configure_memory_attr() {
    uint64_t mair_reg = 0xFF;
    write_system_reg(MAIR_EL1, mair_reg);
}

/*!
 * Setup the TCR_EL1 register
 * @param None
 */
static void page_table_configure_tcr_reg() {
    uint64_t tcr_reg = 0;
    // 52-bit addressing disabled
    // tcr &= ~(1 << TCR_REG_DS_OFFSET);
    // 48-bit physical address space
    tcr_reg |= 0b101ul << TCR_REG_IPS_OFFSET;
    // 48-bit virtual address space.
    tcr_reg |= 0b010000ul << TCR_REG_T0SZ_OFFSET;
    // The translation granule size (4kB)
    tcr_reg |= 0b00ul << TCR_REG_TG0_OFFSET;
    // The second translation table is not valid
    tcr_reg |= 1ul << TCR_REG_EPD1_OFFSET;
    // Perhaps set HD and HA bits to let HW check pages as dirty
    
    write_system_reg(TCR_EL1, tcr_reg);
}


/*!
 * Insert a new table descriptor into the page table.
 * @param table Pointer to the table we are modifing
 * @param next_table Pointer to the table we are mapping
 * @param target_address The virtual/block/table address we are inserting
 * @param level The level of the table we are inserting into
 */
static void page_table_insert_table_descriptor(uint64_t* table, uint64_t* next_table, void* target_address, int level) {
    unsigned long index = (uint64_t)target_address >> ((3 - level) * 9 + 12);
    index = index & (PAGE_TABLE_ENTRIES - 1);
    uint64_t descriptor = (uint64_t)next_table;
    // Valid bit and index to next table
    descriptor |= 0b11;
    table[index] = descriptor;
}


/*!
 * Insert a new block or leaf descriptor into the page table.
 * @param table Pointer to the table we are modifing
 * @param next_table Pointer to the table we are mapping
 * @param target_address The virtual/block/table address we are inserting
 * @param level The level of the table we are inserting into
 */
static void page_table_insert_descriptor(uint64_t* table, void* pa, void* va, int level) {
    unsigned long index = (uint64_t)va >> ((3 - level) * 9 + 12);
    index = index & (PAGE_TABLE_ENTRIES - 1);
    uint64_t descriptor = (uint64_t)pa;
    // Valid bit
    descriptor |= 0b1;
    descriptor |= 0b1 << 10;
    table[index] = descriptor;
    // MAIR index = 0b000, default permisions: allow all access.
}


static void zero_page_table(uint64_t* table) {
    for(int i = 0; i < PAGE_TABLE_ENTRIES; i++) {
        table[i] = 0ul;
    }
}

static void page_table_initialize() {
    zero_page_table(page_table0);
    zero_page_table(page_table1);
    zero_page_table(page_table1_id);

    // Map base address
    // Point to level 1 table.
    page_table_insert_table_descriptor(page_table0, page_table1, (void*)VA_OFFSET, 0);
    // Point to level 1 identity mapping table.
    page_table_insert_table_descriptor(page_table0, page_table1_id, 0, 0);
    // Point to level 2 table.
    page_table_insert_table_descriptor(page_table1, page_table2, (void*)VA_OFFSET, 1);
    // Point to level 2 identity mapping table.
    page_table_insert_table_descriptor(page_table1_id, page_table2_id, 0, 1);
    // Set block entries
    // TODO: Allocate entire physical RAM
    for(int i = 0; i < PAGE_TABLE_ENTRIES; i++) {
        uint64_t phys_address = i * PAGE_SIZE * PAGE_TABLE_ENTRIES;
        // Proper kernel mapping
        page_table_insert_descriptor(page_table2, (void*)phys_address, (void*)(VA_OFFSET + phys_address), 2);
        // Identity mapping
        page_table_insert_descriptor(page_table2_id, (void*)phys_address, (void*)phys_address, 2);
    }
}



/*!
 * Perform a context switch to enable paging.
 * @param None
 */
void mmu_enable(void) {
    page_table_initialize();
    page_table_configure_memory_attr();
    page_table_configure_tcr_reg();

    // Setup page table address
    write_system_reg(TTBR0_EL1, (uint64_t)page_table0);

    // System control register
    uint64_t sctlr_reg = 0;
    sctlr_reg = (SCTLR_RESERVED | SCTLR_EE_LITTLE_ENDIAN | SCTLR_I_CACHE_DISABLED | SCTLR_D_CACHE_DISABLED | SCTLR_MMU_ENABLED);
    write_system_reg(SCTLR_EL1, sctlr_reg);

    // Jump to the return address 
    // write_system_reg(ELR_EL1, (uint64_t)__builtin_return_address(0));
}
