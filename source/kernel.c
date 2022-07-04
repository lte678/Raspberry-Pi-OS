#include <kernel/version.h>
#include <kernel/delay.h>
#include <kernel/exception.h>
#include <kernel/timer.h>
#include <kernel/panic.h>
#include <kernel/block.h>
#include <kernel/alloc.h>
#include <kernel/inode.h>


#include "alloc/buddy.h"
#include "uart.h"
#include "monoterm/term.h"
#include "fs/fat32.h"
#include "disk/sd.h"


void print_execution_level() {
    uint32_t execution_level = read_system_reg(CurrentEL) >> 2;
    uart_print("Executing in EL");
    print_uint(execution_level);
    uart_print("\r\n");
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

    // Uart
    uart_init();
    uart_print("Booting LXE...\r\n");
    uart_print("Developed by Leon Teichroeb :)\r\n");

    // Print version
    if(version_str(ver_str, sizeof(ver_str)) == 0) {
        uart_print("Version: v");
        uart_print(ver_str);
        uart_print("\r\n");
    }
    print_execution_level();

    // FAT32 shit
    // bpb = (const struct bios_parameter_block){0};
    // if(read_fat32_bpb(&bpb)) {
    //   bpb.bytes_per_sector = 512;
    //   print_bpb(&bpb);
    //
    //    uart_print("FAT32 read BPB failed!\r\n");
    //}

    if(init_buddy_allocator()) {
        panic();
    }

    struct block_dev *primary_sd = alloc_block_dev();
    if(sd_initialize(primary_sd)) {
        uart_print("SD device is required to mount root directory!\r\n");
        panic();
    }
    struct fat32_disk *root_part = init_fat32_disk(primary_sd);
    if(!root_part) {
        uart_print("Failed to load root partition!\r\n");
        panic();
    }
    struct inode *root_node = root_part->root_node;
    inode_print(root_node);

    if(inode_read_data(root_node)) {
        uart_print("Failed to read root inode.\r\n");
        panic();
    }

    struct inode *children = root_node->child_nodes;
    while(children) {
        inode_print(children);
        children = children->peer_nodes;
    }

    // Start monolithic kernel console
    monoterm_start();
}

