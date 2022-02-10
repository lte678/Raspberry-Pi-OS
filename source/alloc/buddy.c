#include <kernel/print.h>


#define MAX_MEM_BLK_SIZE 31

#define BLK_HEAD_IDX(x) (31 - x)

struct mem_blk {
    struct mem_blk *child1;
    struct mem_blk *child2;
    struct mem_blk *prev;
    struct mem_blk *next;
    char blk_size;
    char allocated;
};

// Allow for early memory block initialization
static struct mem_blk pre_blks[128];
static int pre_blk_count = 0;
// Heads of mem_blk lists with specific sizes
// size 2^32 is a index 0
static struct mem_blk *blk_heads[32];

static void use_block(struct mem_blk *b) {
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
}

static void free_block(struct mem_blk *b) {
    // Inserts block into free linked-list.
    unsigned char blk_idx = BLK_HEAD_IDX(b->blk_size);
    struct mem_blk *blk_head = blk_heads[blk_idx];
    if(blk_head) {
        blk_head->prev = b;
    }
    blk_heads[blk_idx] = b;
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
    use_block(b);
}

// Block size is 2^size
static struct mem_blk *next_free_block(int size) {
    struct mem_blk *freeb = 0;
    int i = size;
    // Look for free blocks of the specified size
    while(i >= 0) {
        if(blk_heads[BLK_HEAD_IDX(i)]) {
            freeb = blk_heads[BLK_HEAD_IDX(size)];
        }
        i--;
    }
    if(freeb && i == size) {
        // A free block was found
        // Make sure to mark the block as used now
        use_block(freeb);
        #ifdef DEBUG_BUDDY
            uart_print("Found free block of size ");
            print_int(size);
            uart_print("\r\n");
        #endif
        return freeb;
    } else {
        // A free block was not found. Search for larger blocks
        if(size >= MAX_MEM_BLK_SIZE) {
            // Uh oh, there are no larger free blocks to split!
            uart_print("Error: next_free_block: Out of memory!\r\n");
            return 0;
        } else {
            // Get the next free block of greater size
            freeb = next_free_block(size + 1);
            if(!freeb) {
                return 0;
            }
            #ifdef DEBUG_BUDDY
            uart_print("Splitting block of size ");
            print_int(size);
            uart_print("\r\n");
            #endif
            // Split block into two smaller blocks
            split_block(freeb);
            // Arbitrarily use the first child
            use_block(freeb->child1);
            return freeb->child1;
        }
    }
}

void init_buddy_allocator() {
    if(!alloc_new_block(MAX_MEM_BLK_SIZE)) {
        uart_print("Failed to create root block!\r\n");
    }
    next_free_block(20);
}