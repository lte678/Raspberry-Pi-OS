#include <kernel/block.h>
#include <kernel/mem.h>
#include <kernel/alloc.h>
#include <kernel/inode.h>
#include "fat32.h"


struct fat32_disk *g_root_fs = 0;

static struct inode_ops fat32_inode_ops;

static int read_fat32_bpb(struct bios_parameter_block *bpb, struct block_dev *dev) {
    unsigned char *blk_buff = kmalloc(dev->block_size, 0);
    if(read_blk(dev, blk_buff) != 1) {
        print("Failed to read block device.\n");
        return -1;
    }

    if(*(uint16_t*)(blk_buff + FAT32_MAGIC_NUMBER_OFFSET) != FAT32_MAGIC_NUMBER) {
        print("Block device does not contain FAT signature.\n");
        return -1;
    }

    bpb->bytes_per_sector = *((uint16_t*)(blk_buff + BPB_OFF_BYTES_PER_SECTOR));
    bpb->sectors_per_cluster = *((uint8_t*)(blk_buff + BPB_OFF_SECTORS_PER_CLUSTER));
    bpb->nr_reserved_sectors = *((uint16_t*)(blk_buff + BPB_OFF_RESERVED_SECTORS));
    bpb->nr_file_allocation_tables = *((uint8_t*)(blk_buff + BPB_OFF_NR_FILE_ALLOC_TABLES));
    bpb->nr_root_dir_entry = *((uint16_t*)(blk_buff + BPB_OFF_NR_ROOT_DIR_ENTRIES));
    bpb->fat16_sectors_per_fat = *((uint16_t*)(blk_buff + BPB_OFF_FAT16_SECTORS_PER_FAT));
    bpb->sectors_per_fat = *((uint16_t*)(blk_buff + BPB_OFF_SECTORS_PER_FAT));
    bpb->nr_sectors = *((uint32_t*)(blk_buff + BPB_OFF_TOTAL_SECTORS));
    bpb->nr_fat16_sectors = *((uint16_t*)(blk_buff + BPB_OFF_FAT16_TOTAL_SECTORS));
    bpb->root_cluster = *((uint32_t*)(blk_buff + BPB_OFF_ROOT_DIR_CLUSTER));
    strncpy(bpb->volume_label, (char*)(blk_buff + BPB_OFF_VOLUME_LABEL), 11);
    bpb->volume_label[10] = 0;

    return 0;
}

static int fat32_check_valid(struct bios_parameter_block *bpb) {
    if(bpb->bytes_per_sector != FAT32_BYTES_PER_SECTOR) {
        print("Unexpected number of bytes per sector!\n");
        return -1;
    }

    if(bpb->nr_fat16_sectors == 0 && bpb->nr_sectors == 0) {
        print("Partitions number sectors == 0\n");
        print_bpb(bpb);
        return -1;
    }

    if(bpb->fat16_sectors_per_fat != 0) {
        print("FAT16 filesystems are not supported.\n");
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
            print("Failed to read block device.\n");
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


static int fat32_load_cluster(struct fat32_disk *partition, uint8_t *buffer, unsigned int n, uint32_t cluster_index) {
    unsigned int blk = partition->data_sector + (cluster_index - 2) * partition->bpb->sectors_per_cluster;
    if(seek_blk(partition->dev, (blk*partition->bpb->bytes_per_sector) / partition->dev->block_size)) {
        print("FAT32: Failed to load cluster (seek)\n");
        return 1;
    }

    if(n == 0) {
        if(read_blk(partition->dev, buffer) != 1) {
            print("FAT32: Failed to load cluster (read)\n");
            return 1;
        }
    } else {
        if(read_nblk(partition->dev, buffer, n) != 1) {
            print("FAT32: Failed to load cluster (readn)\n");
            return 1;
        }
    }
    return 0;
}

static unsigned int fat32_load_cluster_chain(struct fat32_disk *partition, uint8_t *buffer, unsigned int buffer_size, uint32_t cluster_index) {
    unsigned int bytes_read = 0;
    uint32_t current_cluster = cluster_index;
    while(bytes_read < buffer_size) {
        if(fat32_load_cluster(partition, buffer + bytes_read, buffer_size - bytes_read, current_cluster)) {
            return 0;
        }
        bytes_read += partition->bytes_per_cluster;
        // Find next cluster index
        current_cluster = partition->fat[current_cluster];
        if((current_cluster & 0x0FFFFFF8) == 0x0FFFFFF8) {
            // End of file/directory
            break;
        }
    }
    return bytes_read;
}

static char ucs2_to_ascii(uint16_t ucs_byte) {
    if(ucs_byte < 127) {
        return ucs_byte;
    }
    return '?';
}


/*
 * Parses the 32 byte entry_row and appends the data it contains to the result.
 * Returns 0 when it has completed reading the entry (we count down to the starting index 0).
 * Return 0xFE when a zero entry has been reached -> end of directory table.
 */
static uint8_t parse_directory_entry(struct inode *result, uint8_t *entry_row, uint8_t prev_index) {
    if(entry_row[0] == 0x00 || entry_row[0] == 0xE5) {
        return 0xFE;
    }

    if((entry_row[11] & 0x0F) == 0x0F) {
        // Long file name

        // Check index
        // An index of over 18 is invalid.
        uint8_t index = entry_row[0] & 0x3F;
        if(index > 18 || index == 0) {
            return 0xFF;
        }
        // Bit 0x40 indicates it is the first entry.
        if(entry_row[0] & 0x40) {
            // Expected termination
            result->filename[(index)*13] = 0;
        } else if(index != prev_index - 1) {
            print("parse_directory_entry: Unexpected LFN order.\n");
            return 0xFF;
        }
        
        // Extract characters
        char lfn_chars[13];
        lfn_chars[0] = ucs2_to_ascii(*(uint16_t*)(entry_row + 1));
        lfn_chars[1] = ucs2_to_ascii(*(uint16_t*)(entry_row + 3));
        lfn_chars[2] = ucs2_to_ascii(*(uint16_t*)(entry_row + 5));
        lfn_chars[3] = ucs2_to_ascii(*(uint16_t*)(entry_row + 7));
        lfn_chars[4] = ucs2_to_ascii(*(uint16_t*)(entry_row + 9));
        lfn_chars[5] = ucs2_to_ascii(*(uint16_t*)(entry_row + 14));
        lfn_chars[6] = ucs2_to_ascii(*(uint16_t*)(entry_row + 16));
        lfn_chars[7] = ucs2_to_ascii(*(uint16_t*)(entry_row + 18));
        lfn_chars[8] = ucs2_to_ascii(*(uint16_t*)(entry_row + 20));
        lfn_chars[9] = ucs2_to_ascii(*(uint16_t*)(entry_row + 22));
        lfn_chars[10] = ucs2_to_ascii(*(uint16_t*)(entry_row + 24));
        lfn_chars[11] = ucs2_to_ascii(*(uint16_t*)(entry_row + 28));
        lfn_chars[12] = ucs2_to_ascii(*(uint16_t*)(entry_row + 30));

        // Append characters to directory name.
        strncpy(result->filename + (index-1)*13, lfn_chars, 13);

        // Return the index
        return index;
    }
    
    // It is a short entry. The entry is complete.
    // Mark directories as such.
    if(entry_row[11] & 0x10) {
        result->state |= INODE_TYPE_DIR;
    } else {
        result->state |= INODE_TYPE_FILE;
    }
    // Extract the cluster
    struct fat32_inode_data *fat_data = result->fs_data;
    fat_data->cluster = *(uint16_t*)(entry_row + 26);
    fat_data->cluster |= (uint32_t)(*(uint16_t*)(entry_row + 20)) << 16;
    result->data_size = (uint32_t)(*(uint32_t*)(entry_row + 28));
    // If the prev index was 0, this is the only entry, and not part of a LFN!
    if(prev_index == 0) {
        int i = 0;
        while(i < 8 && entry_row[i] != 0x20) {
            result->filename[i] = entry_row[i];
            i++;
        }
        if(entry_row[8] != 0x20) {
            result->filename[i] = '.';
            i++;
        }
        for(int j = 0; j < 3; j++) {
            if(entry_row[8 + j] == 0x20) {
                break;
            }
            result->filename[i] = entry_row[8 + j];
            i++;
        }
        result->filename[i] = 0;
    }

    return 0;
}


// This is not a real inode operation. It is called by the op "inode_fetch_data" when the inode is a directory.
static int fat32_inode_read_directory(struct inode *n) {
    struct fat32_inode_data *n_data = (struct fat32_inode_data*)n->fs_data;
    uint32_t n_cluster = n_data->cluster;
    struct fat32_disk *partition = n_data->partition;

    unsigned char *blk_buff = kmalloc(partition->bytes_per_cluster, 0);
    uint8_t prev_index = 0;
    struct inode *new_child = alloc_inode();
    new_child->parent_node = n;
    new_child->ops = fat32_inode_ops;
    new_child->fs_data = kmalloc(sizeof(struct fat32_inode_data), ALLOC_ZERO_INIT);
    ((struct fat32_inode_data*)new_child->fs_data)->partition = partition;
    while(1) {
        fat32_load_cluster(partition, blk_buff, 0, n_cluster);
        for(int i = 0; i < 16; i++) {
            prev_index = parse_directory_entry(new_child, blk_buff, prev_index);
            if(prev_index == 0) {
                // Valid child inode
                inode_insert_child(n, new_child);

                new_child = alloc_inode();
                new_child->parent_node = n;
                new_child->ops = fat32_inode_ops;
                new_child->fs_data = kmalloc(sizeof(struct fat32_inode_data), ALLOC_ZERO_INIT);
                ((struct fat32_inode_data*)new_child->fs_data)->partition = partition;
            } else if(prev_index == 0xFF) {
                print("FAT32 directory entry parse error.\n");
                free(new_child);
                return 1;
            } else if(prev_index == 0xFE) {
                free(new_child);
                return 0;
            }
            blk_buff += 32;
        }
        // Find next cluster index
        n_cluster = partition->fat[n_cluster];
        if((n_cluster & 0x0FFFFFF8) == 0x0FFFFFF8) {
            // End of file/directory
            free(new_child);
            return 0;
        }
    }
}


/*
 * Allocates memory for inode (if unallocated and file), and reads the file/folder contents
 */
static int fat32_inode_fetch_data(struct inode *n) {
    struct fat32_inode_data *n_data = (struct fat32_inode_data*)n->fs_data;
    uint32_t n_cluster = n_data->cluster;
    struct fat32_disk *partition = n_data->partition;

    if(n->state & INODE_TYPE_DIR) {
        // Load directory
        if(fat32_inode_read_directory(n)) {
            print("Failed to load FAT32 directory entry.\n");
            return 1;
        }
        n->state = (n->state & ~INODE_STATE_MASK) | INODE_STATE_VALID;
        return 0;
    } else if(n->state & INODE_TYPE_FILE) {
        // Load file
        if(!n->data) {
            // Data size is populated in new FAT32 inodes, so we may use it here.
            // Don't zero the memory.
            n->data = vmalloc(n->data_size, 0);
            if(!n->data) {
                print("fat32_inode_fetch_data: Failed to allocate space for inode\n");
                return 1;
            }
        }
        unsigned int bytes_loaded = fat32_load_cluster_chain(partition, n->data, n->data_size, n_cluster);
        if(bytes_loaded < n->data_size) {
            // We did not load the expected number of bytes.
            if(bytes_loaded != 0) {
                print("Unexpected number of bytes in file.   {u} != {u}\n", bytes_loaded, n->data_size);
            }
            free(n->data);
            n->data = 0;
            n->state = (n->state & ~INODE_STATE_MASK) | INODE_STATE_NEW;
            return 1;
        }
        // Success!
        n->state = (n->state & ~INODE_STATE_MASK) | INODE_STATE_VALID;
        return 0;
    } else {
        print("Unknown inode type.\n");
        return 1;
    }
}


void print_bpb(struct bios_parameter_block *bpb) {
    print("BIOS Parameter Block\n");

    print("  Bytes per Sector:      {i}\n", (int)bpb->bytes_per_sector);
    print("  Sectors per Cluster:   {i}\n", (int)bpb->sectors_per_cluster);
    print("  Reserved Sectors:      {i}\n", (int)bpb->nr_reserved_sectors);
    print("  Number of FATs:        {i}\n", (int)bpb->nr_file_allocation_tables);
    print("  Nr. Root Dir Entrys:   {i}\n", (int)bpb->nr_root_dir_entry);
    print("  Sectors per FAT:       {i}\n", (int)bpb->sectors_per_fat);
    print("  Number of Sectors:     {u}\n", bpb->nr_sectors);
    print("  Nr of Sectors (FAT16): {i}\n", (int)bpb->nr_fat16_sectors);
    print("  Root Cluster:          {u}\n", bpb->root_cluster);
    print("  Volume Label:          {s}\n", bpb->volume_label);
}


static struct inode_ops fat32_inode_ops = {
    .fetch_data = fat32_inode_fetch_data,
    .push_data = 0
};


struct fat32_disk* init_fat32_disk(struct block_dev *dev) {
    struct fat32_disk *partition = kmalloc(sizeof(struct fat32_disk), ALLOC_ZERO_INIT);
    partition->bpb = kmalloc(sizeof(struct bios_parameter_block), ALLOC_ZERO_INIT);
    partition->dev = dev;
    print("Loading FAT32 BIOS Parameter Block...\n");
    if(read_fat32_bpb(partition->bpb, dev)) {
        goto init_failure;
    }
    partition->fat_entries = partition->bpb->sectors_per_fat * partition->bpb->bytes_per_sector / 4;
    partition->data_sector = partition->bpb->nr_reserved_sectors + partition->bpb->nr_file_allocation_tables * partition->bpb->sectors_per_fat;
    partition->bytes_per_cluster = partition->bpb->sectors_per_cluster * partition->bpb->bytes_per_sector;

    if(fat32_check_valid(partition->bpb)) {
        goto init_failure;
    }
    if(partition->bpb->nr_sectors) {
        partition->sectors = partition->bpb->nr_sectors;
    } else {
        partition->sectors = partition->bpb->nr_fat16_sectors;
    }

    print("Loading FAT32 FAT table...\n");
    if(fat32_load_fat(partition)) {
        goto init_failure;
    }

    // Create root inode
    partition->root_node = alloc_inode();
    if(!partition->root_node) {
        print("Failed to allocated root node.\n");
        goto init_failure;
    }
    partition->root_node->state |= INODE_TYPE_DIR;
    partition->root_node->parent_node = 0;
    partition->root_node->ops = fat32_inode_ops;
    partition->root_node->fs_data = kmalloc(sizeof(struct fat32_inode_data), ALLOC_ZERO_INIT);
    ((struct fat32_inode_data*)partition->root_node->fs_data)->cluster = 2;
    ((struct fat32_inode_data*)partition->root_node->fs_data)->partition = partition;

    return partition;

init_failure:
    free(partition);
    free(partition->bpb);
    return 0;
}