#include <kernel/mem.h>
#include <kernel/version.h>

#include "uart.h"
#include "fs/fat32.h"

void kernel_entry_point(void) {
    struct bios_parameter_block bpb;
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
    memset(0, &bpb, sizeof(bpb));
    if(read_fat32_bpb(&bpb)) {
        uart_print("FAT32 read BPB failed!\n");
        return;
    }
}

