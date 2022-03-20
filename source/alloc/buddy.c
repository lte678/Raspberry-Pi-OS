#include <kernel/print.h>
#include <kernel/types.h>


#define MAX_MEM_BLK_SIZE 11
// Allocate at least 512 bytes at once
#define MEM_BLK_ATOM_SIZE 9
#define MIN_BLK_BYTES (1 << MEM_BLK_ATOM_SIZE)
#define MEM_BLK_BYTES(size) (1 << (MEM_BLK_ATOM_SIZE + size))

#define BLK_HEAD_IDX(x) (MAX_MEM_BLK_SIZE - x)

extern uint32_t __static_memory_end[];

/*
 * A mem_blk can be in three states at any one time:
 * - Free
 * - Split (children pointers are set)
 * - Allocated (allocated flag is set)
 */
struct mem_blk {
    struct mem_blk *child1;
    struct mem_blk *child2;
    struct mem_blk *prev_free;
    struct mem_blk *next_free;
    struct mem_blk *prev;
    struct mem_blk *next;
    char blk_size;
    char allocated;
    void *start_addr;
};

// Allow for early memory block initialization
static struct mem_blk pre_blks[128];
// Number of allocated blocks from pre_blks region
static int pre_blk_count = 0;
// Heads of mem_blk lists with specific sizes
// size 2^32 is a index 0
static struct mem_blk *blk_heads[MAX_MEM_BLK_SIZE + 1];
// Start of linked list for all blocks. Contains all sizes.
static struct mem_blk *blk_all_head;

// Start of heap
static void* heap_start;
// End of heap
static void* heap_end;


/*!
 * Converts the buddy block size to the number of bytes it occupies.
 * @param size  The size property of the block
 * @return      The size of the block in bytes
 */
static unsigned long size_to_bytes(unsigned char size) {
    // 2^size * min_blk_size
    return (1 << size) * MIN_BLK_BYTES;
}


/*!
 * Removes the block from the 'free' linked list.
 * Should not be called outside of mark_block_allocated() and split_block().
 * @param b     The block to remove from the free collection
 */
static void unfree_block(struct mem_blk *b) {
    // Remove from free list
    struct mem_blk *next, *prev;
    next = b->next_free;
    prev = b->prev_free;
    if(next) {
        next->prev_free = prev;
    }
    if(prev) {
        prev->next_free = next;
    } else {
        // Start of list
        blk_heads[BLK_HEAD_IDX(b->blk_size)] = next;
    }

    /*
    #ifdef DEBUG_BUDDY
    uart_print("Allocated block of size ");
    print_int(b->blk_size);
    uart_print("\r\n");
    #endif
    */
}


/*!
 * Puts a block into the free collection. Clears the allocated flag.
 * @param b     The block to free.
 */
static void free_block(struct mem_blk *b) {
    // Inserts block into free linked-list.
    unsigned char blk_idx = BLK_HEAD_IDX(b->blk_size);
    struct mem_blk *blk_head = blk_heads[blk_idx];
    if(blk_head) {
        blk_head->prev_free = b;
        b->next_free = blk_head;
    }
    blk_heads[blk_idx] = b;
    b->allocated = 0;
    /*
    #ifdef DEBUG_BUDDY
    uart_print("Put free block into queue at index ");
    print_int(blk_idx);
    uart_print("\r\n");
    #endif
    */
    
}


/*!
 * Mark the block as allocated and no longer available
 * @param b     The block to allocate
 */
static void block_mark_allocated(struct mem_blk *b) {
    unfree_block(b);
    // Mark as allocated
    b->allocated = 1;
}


static struct mem_blk *alloc_new_block(char size, void* addr) {
    // Get block from stack memory
    if(pre_blk_count > 127) {
        uart_print("Error: alloc_new_block: Reached mem_blk limit!\r\n");
        return 0;
    }
    struct mem_blk *b = &pre_blks[pre_blk_count];
    pre_blk_count++;
    // Initialize struct
    b->blk_size = size;
    b->allocated = 0;
    b->child1 = 0;
    b->child2 = 0;
    b->next_free = 0;
    b->prev_free = 0;
    b->prev = 0;
    b->next = blk_all_head;
    b->start_addr = addr;
    // Add block to the beginning of list of all blocks
    blk_all_head->prev = b;
    blk_all_head = b;
    // Put block into free list
    free_block(b);
    return b;
}


static void split_block(struct mem_blk *b) {
    if(b->blk_size > 0) {
        unsigned long child_addr_mask = 1 << (b->blk_size + MEM_BLK_ATOM_SIZE - 1);
        b->child1 = alloc_new_block(b->blk_size - 1, b->start_addr);
        b->child2 = alloc_new_block(b->blk_size - 1, (void*)((unsigned long)b->start_addr ^ child_addr_mask));
    }
    // Mark block as used. Its children are now available
    unfree_block(b);
}


// Block size is 2^size * min_size
static struct mem_blk *next_free_block(int size) {
    struct mem_blk *freeb = 0;
    int i = size;
    // Look for free blocks of the specified size
    while(i <= MAX_MEM_BLK_SIZE) {
        if(blk_heads[BLK_HEAD_IDX(i)]) {
            freeb = blk_heads[BLK_HEAD_IDX(i)];
            break;
        }
        i++;
    }

    if(freeb && i == size) {
        // A free block was found
        // Make sure to mark the block as used now
        return freeb;
    } else {
        // A free block was not found. Search for larger blocks
        if(!freeb) {
            // Uh oh, there are not even free blocks to split!
            uart_print("Error: next_free_block: Out of memory!\r\n");
            return 0;
        } else {
            // Get the next_free free block of greater size
            freeb = next_free_block(size + 1);
            // Split block into two smaller blocks
            split_block(freeb);
            // Arbitrarily use the first child
            return freeb->child1;
        }
    }
}


void* kmalloc(unsigned long size) {
    int i = 0;
    while(size_to_bytes(i) < size) {
        i++;
    }
    struct mem_blk *b = next_free_block(i);
    block_mark_allocated(b);
    
    #ifdef DEBUG_BUDDY
    uart_print("Found free ");
    print_ulong(size_to_bytes(i));
    uart_print(" byte block (level: ");
    print_int(i);
    uart_print(") at addr ");
    print_ulong((unsigned long)b->start_addr);
    uart_print("\r\n");
    #endif

    return b->start_addr;
}


int init_buddy_allocator() {
    // Creates superblock starting at address 0
    uart_print("Initializing buddy allocator with block size ");
    print_int(MIN_BLK_BYTES);
    uart_print("\r\n");

    // Figure out heap limits
    unsigned int blk_mask = MAX_MEM_BLK_SIZE + MEM_BLK_ATOM_SIZE;
    heap_end = (void*)((0x00400000ul >> blk_mask) << blk_mask);
    void* heap_min_limit = __static_memory_end;
    if(heap_end < heap_min_limit + size_to_bytes(MAX_MEM_BLK_SIZE)) {
        uart_print("Failed to allocate buddy blocks: Insufficient memory!\r\n");
        print_ulong(heap_end - heap_min_limit);
        uart_print("B available\r\n");
        return 1;
    }

    // Iterate over memory region and fill with blocks.
    void* block_p = heap_end - size_to_bytes(MAX_MEM_BLK_SIZE);
    while(block_p >= heap_min_limit) {
        alloc_new_block(MAX_MEM_BLK_SIZE, block_p);
        block_p -= size_to_bytes(MAX_MEM_BLK_SIZE);
    }
    heap_start = block_p + size_to_bytes(MAX_MEM_BLK_SIZE);

    #ifdef DEBUG_BUDDY
    uart_print("Allocating block...\r\n");
    kmalloc(511);
    kmalloc(1500);
    kmalloc(512);
    kmalloc(1);
    kmalloc(1);
    kmalloc(1);
    kmalloc(3000);
    kmalloc(1040);
    #endif

    return 0;
}


unsigned long memory_allocated() {
    unsigned long sum = 0;
    struct mem_blk *b = blk_all_head;
    while(b) {
        if(b->allocated) {
            sum += size_to_bytes(b->blk_size);
        }
        b = b->next;
    }
    return sum;
}


void* buddy_heap_start() {
    return heap_start;
}


void* buddy_heap_end() {
    return heap_end;
}


static void print_block(struct mem_blk *b, char start_depth) {
    term_set_cursor_column((start_depth - b->blk_size) * 2 + 1);
    print_hex_uint32((uint64_t)b->start_addr);
    uart_print(" - ");
    print_hex_uint32((uint64_t)b->start_addr + MEM_BLK_BYTES(b->blk_size));
    if(b->allocated) {
        term_set_cursor_column(start_depth * 2 + 1 + 20);
        uart_print(" [allocated]");
    }
    uart_print("\r\n");
    if(b->child1 || b->child2) {
        // Block is split
        print_block(b->child1, start_depth);
        print_block(b->child2, start_depth);
    } else {
        // Block is not split
    }
}


int monoterm_buddy_print_map(int argc, char* argv[]) {
    struct mem_blk *b = blk_all_head;
    while(b) {
        if(b->blk_size == MAX_MEM_BLK_SIZE) {
            print_block(b, b->blk_size);
        }
        b = b->next;
    }
    return 0;
}