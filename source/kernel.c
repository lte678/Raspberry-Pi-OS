#include <kernel/version.h>
#include <kernel/delay.h>
#include <kernel/exception.h>
#include <kernel/timer.h>
#include <kernel/panic.h>


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
    uint64_t time;
    
    wait_usec(3000000);

    irq_init();
    enable_system_timer_interrupt();
    time = read_system_timer();
    set_system_timer_interrupt((time + 1000000) & 0xFFFFFFFF);
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
    sd_initialize();
    // Start monolithic kernel console
    monoterm_start();
}

