#ifndef FAT32_H
#define FAT32_H

#include <kernel/types.h>
#include <kernel/print.h>

#define FAT32_BYTES_PER_SECTOR 512
#define FAT32_NR_FAT 2
#define FAT32_MAGIC_NUMBER_OFFSET 0x1FE
#define FAT32_MAGIC_NUMBER 0xAA55

// https://en.wikipedia.org/wiki/BIOS_parameter_block
#define BPB_GLOBAL_OFF = 0x0B

// DOS 2.0 BPB + some fat32 stuff
#define BPB_OFF_BYTES_PER_SECTOR = 0x00
#define BPB_OFF_SECTORS_PER_CLUSTER = 0x02
#define BPB_OFF_RESERVED_SECTORS = 0x03
#define BPB_OFF_NR_FILE_ALLOC_TABLES = 0x05
#define BPB_OFF_NR_ROOT_DIR_ENTRIES = 0x06
// Extended fat32 specific data
#define BPB_OFF_TOTAL_SECTORS = 0x19
#define BPB_OFF_ROOT_DIR_CLUSTER = 0x21

struct bios_parameter_block {
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t nr_reserved_sectors;
    uint8_t nr_file_allocation_tables;
    uint16_t nr_root_dir_entry;
    uint32_t nr_sectors;
    uint32_t root_cluster;
};

int read_fat32_bpb(struct bios_parameter_block *bpb);
void print_bpb(struct bios_parameter_block *bpb);

#endif  // FAT32_H