#include <kernel/print.h>


#define MAX_MEM_BLK_SIZE 31
// Allocate at least 512 bytes at once
#define MIN_BLK_BYTES 512

#define BLK_HEAD_IDX(x) (MAX_MEM_BLK_SIZE - x)

struct mem_blk {
    struct mem_blk *child1;
    struct mem_blk *child2;
    struct mem_blk *prev;
    struct mem_blk *next;
    struct mem_blk *prev_alloc;
    struct mem_blk *next_alloc;
    char blk_size;
    char allocated;
};

// Allow for early memory block initialization
static struct mem_blk pre_blks[128];
static int pre_blk_count = 0;
// Heads of mem_blk lists with specific sizes
// size 2^32 is a index 0
static struct mem_blk *blk_heads[MAX_MEM_BLK_SIZE + 1];
// Start of linked list for allocated blocks. Contains all sizes.
static struct mem_blk *allocated;


static unsigned long size_to_bytes(unsigned char size) {
    // 2^size * min_blk_size
    return (1 << size) * MIN_BLK_BYTES;
}

static void unfree_block(struct mem_blk *b) {
    // Remove from free list
    struct mem_blk *next, *prev;
    next = b->next;
    prev = b->prev;
    if(next) {
        next->prev = prev;
    }
    if(prev) {
        prev->next = next;
    }

    /*
    #ifdef DEBUG_BUDDY
    uart_print("Allocated block of size ");
    print_int(b->blk_size);
    uart_print("\r\n");
    #endif
    */
}

static void use_block(struct mem_blk *b) {
    // Put into allocated list
    if(allocated) {
        allocated->prev_alloc = b;
        b->next_alloc = allocated;
    }
    allocated = b;
}

static void free_block(struct mem_blk *b) {
    // Inserts block into free linked-list.
    unsigned char blk_idx = BLK_HEAD_IDX(b->blk_size);
    struct mem_blk *blk_head = blk_heads[blk_idx];
    if(blk_head) {
        blk_head->prev = b;
        b->next = blk_head;
    }
    blk_heads[blk_idx] = b;

    // Remove from allocated list
    struct mem_blk *next, *prev;
    next = b->next_alloc;
    prev = b->prev_alloc;
    if(next) {
        next->prev_alloc = prev;
    }
    if(prev) {
        prev->next_alloc = next;
    }
    /*
    #ifdef DEBUG_BUDDY
    uart_print("Put free block into queue at index ");
    print_int(blk_idx);
    uart_print("\r\n");
    #endif
    */
    
}

static struct mem_blk *alloc_new_block(char size) {
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
    b->next = 0;
    b->prev = 0;
    // Put block into free list
    free_block(b);
    return b;
}

static void split_block(struct mem_blk *b) {
    if(b->blk_size > 0) {
        b->child1 = alloc_new_block(b->blk_size - 1);
        b->child2 = alloc_new_block(b->blk_size - 1);
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
            // Get the next free block of greater size
            freeb = next_free_block(size + 1);
            // Split block into two smaller blocks
            split_block(freeb);
            // Arbitrarily use the first child
            return freeb->child1;
        }
    }
}

void kmalloc(unsigned long size) {
    int i = 0;
    while(size_to_bytes(i) < size) {
        i++;
    }
    struct mem_blk *b = next_free_block(i);
    use_block(b);
    unfree_block(b);
    //if(b == 0) {
    //
    //}
    #ifdef DEBUG_BUDDY
    uart_print("Found free ");
    print_ulong(size_to_bytes(i));
    uart_print(" byte block (level: ");
    print_int(i);
    uart_print(")\r\n");
    #endif
}

void init_buddy_allocator() {
    if(!alloc_new_block(MAX_MEM_BLK_SIZE)) {
        uart_print("Failed to create root block!\r\n");
    }
    #ifdef DEBUG_BUDDY
    uart_print("Allocating block...\r\n");
    kmalloc(511);
    kmalloc(1500);
    kmalloc(512);
    #endif
}

unsigned long memory_allocated() {
    unsigned long sum = 0;
    struct mem_blk *b = allocated;
    while(b) {
        sum += size_to_bytes(b->blk_size);
        b = b->next_alloc;
    }
    return sum;
}
