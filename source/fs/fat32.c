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
    char buf[16];
    
    uart_print("BIOS Parameter Block\n");

    uart_print("\n  Bytes per Sector:      ");
    itos(bpb->bytes_per_sector, buf, sizeof(buf));
    uart_print(buf);

    uart_print("\n  Sectors per Cluster:   ");
    itos(bpb->sectors_per_cluster, buf, sizeof(buf));
    uart_print(buf);

    uart_print("\n  Reserved Sectors:      ");
    itos(bpb->nr_reserved_sectors, buf, sizeof(buf));
    uart_print(buf);

    uart_print("\n  Number of FATs:        ");
    itos(bpb->nr_file_allocation_tables, buf, sizeof(buf));
    uart_print(buf);

    uart_print("\n  Nr. Root Dir Entrys:   ");
    itos(bpb->nr_root_dir_entry, buf, sizeof(buf));
    uart_print(buf);

    uart_print("\n  Number of Sectors:     ");
    itos(bpb->nr_sectors, buf, sizeof(buf));
    uart_print(buf);

    uart_print("\n  Root Cluster:          ");
    itos(bpb->root_cluster, buf, sizeof(buf));
    uart_print(buf);

    uart_print("\n");
}