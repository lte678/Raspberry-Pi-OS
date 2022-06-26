#include <kernel/block.h>
#include <kernel/mem.h>
#include <kernel/alloc.h>
#include "fat32.h"


static int read_fat32_bpb(struct bios_parameter_block *bpb, struct block_dev *dev) {
    unsigned char *blk_buff = kmalloc(dev->block_size, 0);
    if(read_blk(dev, blk_buff) != 1) {
        uart_print("Failed to read block device.\r\n");
        return -1;
    }

    if(*(uint16_t*)(blk_buff + FAT32_MAGIC_NUMBER_OFFSET) != FAT32_MAGIC_NUMBER) {
        uart_print("Block device does not contain FAT signature.\r\n");
        return -1;
    }

    bpb->bytes_per_sector = *((uint16_t*)(blk_buff + BPB_OFF_BYTES_PER_SECTOR));
    bpb->sectors_per_cluster = *((uint8_t*)(blk_buff + BPB_OFF_SECTORS_PER_CLUSTER));
    bpb->nr_reserved_sectors = *((uint16_t*)(blk_buff + BPB_OFF_RESERVED_SECTORS));
    bpb->nr_file_allocation_tables = *((uint8_t*)(blk_buff + BPB_OFF_NR_FILE_ALLOC_TABLES));
    bpb->nr_root_dir_entry = *((uint16_t*)(blk_buff + BPB_OFF_NR_ROOT_DIR_ENTRIES));
    bpb->sectors_per_fat = *((uint16_t*)(blk_buff + BPB_OFF_SECTORS_PER_FAT));
    bpb->nr_sectors = *((uint32_t*)(blk_buff + BPB_OFF_TOTAL_SECTORS));
    bpb->root_cluster = *((uint32_t*)(blk_buff + BPB_OFF_ROOT_DIR_CLUSTER));
    strncpy(bpb->volume_label, (char*)(blk_buff + BPB_OFF_VOLUME_LABEL), 11);
    bpb->volume_label[10] = 0;

    return 0;
}

static int fat32_check_valid(struct bios_parameter_block *bpb) {
    if(bpb->bytes_per_sector != FAT32_BYTES_PER_SECTOR) {
        uart_print("Unexpected number of bytes per sector!\r\n");
        return -1;
    }

    return 0;
}

static int fat32_load_fat(struct fat32_disk *part) {
    unsigned int fat_bytes = part->fat_entries * 4;
    
    unsigned int fat_start = part->bpb->nr_reserved_sectors * part->bpb->bytes_per_sector;
    if(seek_blk(part->dev, fat_start / part->dev->block_size)) {
        return -1;
    }

    part->fat = kmalloc(fat_bytes, 0);
    unsigned int read_entries = 0;
    while(1) {
        unsigned char *blk_buff = kmalloc(part->dev->block_size, 0);
        if(read_blk(part->dev, blk_buff) != 1) {
            uart_print("Failed to read block device.\r\n");
            free(blk_buff);
            free(part->fat);
            return -1;
        }
        
        int i = 0;
        while(i < (part->dev->block_size / 4)) {
            part->fat[read_entries] = *(uint32_t*)(blk_buff + (i * 4));
            read_entries++;
            i++;

            if(read_entries >= part->fat_entries) {
                free(blk_buff);
                return 0;
            }
        }
        free(blk_buff);
    }
}

void print_bpb(struct bios_parameter_block *bpb) {
    uart_print("BIOS Parameter Block");

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

    uart_print("\r\n  Sectors per FAT:       ");
    print_int(bpb->sectors_per_fat);

    uart_print("\r\n  Number of Sectors:     ");
    print_int(bpb->nr_sectors);

    uart_print("\r\n  Root Cluster:          ");
    print_int(bpb->root_cluster);

    uart_print("\r\n  Volume Label:          ");
    uart_print(bpb->volume_label);

    uart_print("\r\n");
}

/*
 * Parses the 32 byte entry_row and appends the data it contains to the result.
 * Returns 1 when it has completed reading the entry.
 */
int parse_directory_entry(struct directory_entry *result, uint8_t *entry_row) {
    if(entry_row[11] & 0x0F == 0x0F) {
            // Long file name
            
    }
    
    // It is a short entry. The entry is complete.
    // Mark directories as such.
    if(entry_row[11] & 0x10) {
        result->is_directory = 1;
    }
    return 1;
}

struct fat32_disk* init_fat32_disk(struct block_dev *dev) {
    struct fat32_disk *partition = kmalloc(sizeof(struct fat32_disk), ALLOC_ZERO_INIT);
    partition->bpb = kmalloc(sizeof(struct bios_parameter_block), ALLOC_ZERO_INIT);
    partition->dev = dev;
    uart_print("Loading FAT32 BIOS Parameter Block...\r\n");
    if(read_fat32_bpb(partition->bpb, dev)) {
        goto init_failure;
    }
    partition->fat_entries = partition->bpb->sectors_per_fat * partition->bpb->bytes_per_sector / 4;
    partition->data_sector = partition->bpb->nr_reserved_sectors + partition->bpb->nr_file_allocation_tables * partition->bpb->sectors_per_fat;
    if(fat32_check_valid(partition->bpb)) {
        goto init_failure;
    }
    uart_print("Loading FAT32 FAT table...\r\n");
    if(fat32_load_fat(partition)) {
        goto init_failure;
    }

    // We are ready to read the file system tree
    uart_print("Loading filesystem tree...\r\n");
    unsigned int root_blk = partition->data_sector + (partition->bpb->root_cluster - 2) * partition->bpb->sectors_per_cluster;
    seek_blk(partition->dev, (root_blk*partition->bpb->bytes_per_sector) / partition->dev->block_size);

    unsigned char *blk_buff = kmalloc(partition->dev->block_size, 0);
    if(read_blk(partition->dev, blk_buff) != 1) {
        uart_print("Failed to read block device.\r\n");
        free(blk_buff);
        return 0;
    }
    for(int i = 0; i < 16; i++) {
        print_hex(blk_buff, 32);
        
        blk_buff += 32;
        uart_print("\r\n");
    }
    //uart_print(blk_buff);

    return partition;

init_failure:
    free(partition);
    free(partition->bpb);
    return 0;
}