#include "fat32.h"

int read_fat32_bpb(struct bios_parameter_block *bpb) {
    uart_print("Loading FAT32 BIOS Parameter Block...\r\n");

    return -1;
}

int fat32_check_valid(struct bios_parameter_block *bpb) {
    if(bpb->bytes_per_sector != FAT32_BYTES_PER_SECTOR) {
        uart_print("Unexpected number of bytes per sector!\r\n");
        return -1;
    }

    return 0;
}

void print_bpb(struct bios_parameter_block *bpb) {
    uart_print("BIOS Parameter Block\r\n");

    uart_print("\r\n  Bytes per Sector:      ");
    print_int(bpb->bytes_per_sector);

    uart_print("\r\n  Sectors per Cluster:   ");
    print_int(bpb->sectors_per_cluster);

    uart_print("\r\n  Reserved Sectors:      ");
    print_int(bpb->nr_reserved_sectors);

    uart_print("\r\n  Number of FATs:        ");
    print_int(bpb->nr_file_allocation_tables);

    uart_print("\r\n  Nr. Root Dir Entrys:   ");
    print_int(bpb->nr_root_dir_entry);

    uart_print("\r\n  Number of Sectors:     ");
    print_int(bpb->nr_sectors);

    uart_print("\r\n  Root Cluster:          ");
    print_int(bpb->root_cluster);

    uart_print("\r\n");
}