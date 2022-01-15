#include "fat32.h"

int read_fat32_bpb(struct bios_parameter_block *bpb) {
    uart_print("Loading FAT32 BIOS Parameter Block...\n");

    return -1;
}

int fat32_check_valid(struct bios_parameter_block *bpb) {
    if(bpb->bytes_per_sector != FAT32_BYTES_PER_SECTOR) {
        uart_print("Unexpected number of bytes per sector!\n");
        return -1;
    }

    return 0;
}

void print_bpb(struct bios_parameter_block *bpb) {
    uart_print("BIOS Parameter Block\n");

    uart_print("\n  Bytes per Sector:      ");
    print_int(bpb->bytes_per_sector);

    uart_print("\n  Sectors per Cluster:   ");
    print_int(bpb->sectors_per_cluster);

    uart_print("\n  Reserved Sectors:      ");
    print_int(bpb->nr_reserved_sectors);

    uart_print("\n  Number of FATs:        ");
    print_int(bpb->nr_file_allocation_tables);

    uart_print("\n  Nr. Root Dir Entrys:   ");
    print_int(bpb->nr_root_dir_entry);

    uart_print("\n  Number of Sectors:     ");
    print_int(bpb->nr_sectors);

    uart_print("\n  Root Cluster:          ");
    print_int(bpb->root_cluster);

    uart_print("\n");
}