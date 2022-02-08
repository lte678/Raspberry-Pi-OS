#include <kernel/mem.h>
#include <kernel/version.h>

#include "uart.h"
#include "monoterm/term.h"
#include "fs/fat32.h"
#include "disk/sd.h"

void kernel_entry_point(void) {
    //struct bios_parameter_block bpb;
    char ver_str[8];
    
    uart_init();
    uart_print("Booting LXE...\n");
    uart_print("Developed by Leon Teichroeb :)\n");

    // Print version
    if(version_str(ver_str, sizeof(ver_str)) == 0) {
        uart_print("Version: v");
        uart_print(ver_str);
        uart_print("\n");
    }

    // FAT32 shit
    // bpb = (const struct bios_parameter_block){0};
    // if(read_fat32_bpb(&bpb)) {
    //   bpb.bytes_per_sector = 512;
    //   print_bpb(&bpb);
    //
    //    uart_print("FAT32 read BPB failed!\n");
    //}

    // sd_initialize();

    // Start monolithic kernel console
    monoterm_start();
}

