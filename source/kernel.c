#include <kernel/version.h>
#include <kernel/delay.h>
#include <kernel/exception.h>
#include <kernel/timer.h>
#include <kernel/panic.h>
#include <kernel/block.h>
#include <kernel/alloc.h>
#include <kernel/inode.h>
#include <kernel/pagetable.h>
#include <kernel/address_space.h>
#include <kernel/mmap.h>


#include "alloc/buddy.h"
#include "uart.h"
#include "monoterm/monoterm.h"
#include "fs/fat32.h"
#include "disk/sd.h"


void print_execution_level() {
    uint32_t execution_level = read_system_reg(CurrentEL) >> 2;
    print("Executing in EL{u}\r\n", execution_level);
}

void kernel_entry_point(void) {
    //struct bios_parameter_block bpb;
    char ver_str[8];
    //uint64_t time;
    
    wait_usec(3000000);

    // Exceptions
    init_exceptions();
    enable_system_timer_interrupt();
    //time = read_system_timer();
    //set_system_timer_interrupt((time + 1000000) & 0xFFFFFFFF);

    // Populate kernel_page_table pointer
    page_table_init();

    // Uart
    uart_pre_init();
    print("Stack address: {xl}\r\n", read_stack_pointer());
    print("Booting LXE...\r\n");
    print("Developed by Leon Teichroeb :)\r\n");

    // Print version
    if(version_str(ver_str, sizeof(ver_str)) == 0) {
        print("Version: v{s}\r\n", ver_str);
    }
    print_execution_level();

    // FAT32 shit
    // bpb = (const struct bios_parameter_block){0};
    // if(read_fat32_bpb(&bpb)) {
    //   bpb.bytes_per_sector = 512;
    //   print_bpb(&bpb);
    //
    //    print("FAT32 read BPB failed!\r\n");
    //}

    if(init_buddy_allocator()) {
        panic();
    }
    
    struct address_mapping *id_mapping;
    if(!init_kernel_address_space_struct(&id_mapping)) {
        print("Failed to allocate initial address space.");
        panic();
    }
    // We can now preform memory mappings. Migrate to new UART mappings before we remove the identity maps.
    if(uart_init()) {
        panic();
    }
    // After this point, physical addresses no longer work!
    if(unmap_memory_region(kernel_address_space, id_mapping)) {
        print("Failed to unmap identity mapping!\r\n");
        panic();
    }
    print("Unmapped identity mapping.\r\n");
    
    struct block_dev *primary_sd = alloc_block_dev();
    if(sd_initialize(primary_sd)) {
        print("SD device is required to mount root directory!\r\n");
        panic();
    }
    struct fat32_disk *root_part = init_fat32_disk(primary_sd);
    if(!root_part) {
        print("Failed to load root partition!\r\n");
        panic();
    }
    g_root_inode = root_part->root_node;

    // Start monolithic kernel console
    monoterm_start();
}

