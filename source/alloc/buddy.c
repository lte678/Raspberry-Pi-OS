#include <kernel/print.h>
#include <kernel/term.h>
#include <kernel/types.h>
#include <kernel/page.h>
#include <kernel/panic.h>
#include <kernel/alloc.h>
#include <kernel/mem.h>
#include "pool.h"


// TODO: Hashing based on max-sized block address, to reduce free-lookups to O(1) complexity

#define MAX_MEM_BLK_SIZE 13
// Allocate at least 512 bytes at once
#define MEM_BLK_ATOM_SIZE 9
#define MIN_BLK_BYTES (1 << MEM_BLK_ATOM_SIZE)
#define MEM_BLK_BYTES(size) (1 << (MEM_BLK_ATOM_SIZE + size))

#define INITIAL_POOL_SIZE 8

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

// Pool allocator for mem_blk objects
static struct memory_pool buddy_pool;

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


static void remove_from_free_list(struct mem_blk* b) {
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
}


static void insert_into_free_list(struct mem_blk* b) {
    // Inserts block into free linked-list.
    unsigned char blk_idx = BLK_HEAD_IDX(b->blk_size);
    struct mem_blk *blk_head = blk_heads[blk_idx];
    if(blk_head) {
        blk_head->prev_free = b;
    }
    b->next_free = blk_head;
    b->prev_free = 0;
    blk_heads[blk_idx] = b;
}


static void remove_from_all_list(struct mem_blk* b) {
    // Remove from list containing all blocks. 
    // Must be called when deallocating the struct
    struct mem_blk *next, *prev;
    next = b->next;
    prev = b->prev;
    if(next) {
        next->prev = prev;
    }
    if(prev) {
        prev->next = next;
    } else {
        // Start of list
        blk_all_head = next;
    }
}


static void insert_into_all_list(struct mem_blk* b) {
    // Add to list containing all blocks. 
    // Must be called when allocating the struct
    if(blk_all_head) {
        blk_all_head->prev = b;
        b->next = blk_all_head;
    }
    blk_all_head = b;
}


static struct mem_blk *alloc_new_block(char size, void* addr) {
    struct mem_blk *b = pool_alloc(&buddy_pool);
    if(!b) {
        print("Buddy: Failed to allocate block struct!\r\n");
        return 0;
    }
    // Initialize struct
    b->blk_size = size;
    b->allocated = 0;
    b->child1 = 0;
    b->child2 = 0;
    b->next_free = 0;
    b->prev_free = 0;
    b->prev = 0;
    b->next = 0;
    b->start_addr = addr;
    // Add block to the beginning of list of all blocks
    insert_into_all_list(b);
    // Put block into free list
    insert_into_free_list(b);
    return b;
}


static int split_block(struct mem_blk *b) {
    if(b->blk_size > 0) {
        unsigned long child_addr_mask = 1 << (b->blk_size + MEM_BLK_ATOM_SIZE - 1);
        b->child1 = alloc_new_block(b->blk_size - 1, b->start_addr);
        b->child2 = alloc_new_block(b->blk_size - 1, (void*)((unsigned long)b->start_addr ^ child_addr_mask));
        // Undo allocation if one failed.
        if(!b->child1 || !b->child2) {
            if(b->child1) {
                remove_from_free_list(b->child1);
                remove_from_all_list(b->child1);
                pool_free(&buddy_pool, b->child1);
                b->child1 = 0;
            } else if(b->child2) {
                remove_from_free_list(b->child2);
                remove_from_all_list(b->child2);
                pool_free(&buddy_pool, b->child2);
                b->child2 = 0;
            }
            return 1;
        }

        // Mark block as used. Its children are now available
        remove_from_free_list(b);
        return 0;
    } else {
        print("Allocator attempted to split block of size 0.\r\n");
        panic();
    }
}


/*!
 * Traverses the block binary tree starting at a leaf and preforms all possible merges.
 * \param b     A leaf block
 */
static void merge_blocks(struct mem_blk *b) {
    if(b->blk_size >= MAX_MEM_BLK_SIZE) {
        return;
    }
    uint64_t parent_addr_mask = 1 << (b->blk_size + MEM_BLK_ATOM_SIZE);
    void *parent_addr = (void*)((uint64_t)b->start_addr & ~parent_addr_mask);
    struct mem_blk *i = blk_all_head;
    while(i) {
        if(i->start_addr == parent_addr && i->blk_size == b->blk_size + 1) {
            /* The other child block is not free. Don't merge. */
            if (i->child1->allocated || i->child2->allocated) {
                return;
            }
            /* The other child block is split. Don't merge. */
            if (i->child1->child1 || i->child2->child1) {
                return;
            }
            // We found our parent block
            remove_from_free_list(i->child1);
            remove_from_all_list(i->child1);
            pool_free(&buddy_pool, i->child1);
            i->child1 = 0;
            remove_from_free_list(i->child2);
            remove_from_all_list(i->child2);
            pool_free(&buddy_pool, i->child2);
            i->child2 = 0;

            insert_into_free_list(i);
            merge_blocks(i);
            return;
        }
        i = i->next;
    }
    print("Buddy: Tried to merge invalid block!\r\n");
}


static int buddy_reserve_block(void* addr, int size) {
    struct mem_blk *b = blk_heads[BLK_HEAD_IDX(MAX_MEM_BLK_SIZE)];
    while(b) {
        if(b->start_addr == addr) {
            break;
        }
        b = b->next_free;
    }
    // No free block
    if(!b) {
        return 1;
    }
    while(b->blk_size > size) {
        if(!b->child1) {
            if(split_block(b)) {
                // Failed to split block
                return 1;
            }
        }

        if(b->child1->start_addr == addr && !b->child1->allocated) {
            b = b->child1;
        } else if(b->child2->start_addr == addr && !b->child2->allocated) {
            b = b->child2;
        } else {
            return 1;
        }
    }
    // Block of right size at right address located
    remove_from_free_list(b);
    b->allocated = 1;
    return 0;
}


// Block size is 2^size * min_size
static struct mem_blk *next_free_block(int size) {
    if(size > MAX_MEM_BLK_SIZE || size < 0) {
        return 0;
    }
    struct mem_blk *freeb = blk_heads[BLK_HEAD_IDX(size)];
    if(freeb) {
        // A free block was found
        // Make sure to mark the block as used now

        // Perform sanity check
        #ifdef DEBUG_BUDDY
        if(size != freeb->blk_size) {
            print("Unexpected block in free head array!\r\n");
            print("{i} != {i}\r\n", freeb->blk_size, size);
            panic();
        }
        #endif

        return freeb;
    } else {
        // A free block was not found. Search for larger blocks
        // Get the next_free free block of greater size
        freeb = next_free_block(size + 1);
        if(!freeb) {
            //print("Error: next_free_block: Out of memory!\r\n");
            return 0;
        }

        // Split block into two smaller blocks
        #ifdef DEBUG_BUDDY
        print("Splitting block with address {p}\r\n", freeb->start_addr);
        #endif

        if(split_block(freeb)) {
            // Failed to split block.
            return 0;
        }
        // Arbitrarily use the first child
        return freeb->child1;
    }
}


void* kmalloc(unsigned long size, uint32_t flags) {
    int i = 0;
    while(size_to_bytes(i) < size) {
        i++;
    }
    struct mem_blk *b = next_free_block(i);
    if(b == 0) {
        // A block was not found
        return 0;
    }
    remove_from_free_list(b);
    b->allocated = 1;
    
    #ifdef DEBUG_BUDDY
    print("Found free {ul} byte block (level: {i}) at addr {p}\r\n", size_to_bytes(i), i, b->start_addr);
    #endif

    // Zero region if flag is set
    if(flags & ALLOC_ZERO_INIT) {
        memset(0, b->start_addr, size);
    }

    return b->start_addr;
}


void free(void* memory) {
    struct mem_blk *b = blk_all_head;
    while(b) {
        if(b->start_addr == memory && b->allocated) {
            b->allocated = 0;
            insert_into_free_list(b);
            merge_blocks(b);
            return;
        }
        b = b->next;
    }
    print("Attempted to free invalid pointer!\r\n");
}


int init_buddy_allocator() {
    // Creates superblock starting at address 0
    print("Initializing buddy allocator with block size {i}\r\n", MIN_BLK_BYTES);

    // Figure out heap limits
    uint64_t max_blk_mask = size_to_bytes(MAX_MEM_BLK_SIZE) - 1;
    // Round down heap end to next increment of MAX_MEM_BLK_SIZE
    uint64_t memory_end = 0x40000000ul + VA_OFFSET;
    heap_end = (void*)(memory_end & ~max_blk_mask);
    // Round up heap start to next increment of MAX_MEM_BLK_SIZE
    void* heap_min_limit = (void*)(((uint64_t)__static_memory_end + max_blk_mask) & ~max_blk_mask);
    if(heap_end < heap_min_limit + size_to_bytes(MAX_MEM_BLK_SIZE)) {
        print("Failed to allocate buddy blocks: Insufficient memory!\r\n");
        print("{ul}B available\r\n", heap_end - heap_min_limit);
        return 1;
    }

    // Early initialize memory pool
    // TODO: Find lowest address.
    buddy_pool = pool_create_pool_using_memory(sizeof(struct mem_blk),
        MEM_BLK_BYTES(INITIAL_POOL_SIZE),
        heap_min_limit);

    // The first block is used to kickstart the pool allocator
    if(!alloc_new_block(MAX_MEM_BLK_SIZE, (void*)heap_min_limit)) {
        return 1;
    }
    // Reserve the block we gave to the memory pool
    if(buddy_reserve_block(heap_min_limit, INITIAL_POOL_SIZE)) {
        print("Failed to reserve buddy block for early pool allocator.\r\n");
        return 1;
    }
    // Mark this first page's address as the beginning of our heap
    heap_start = (void*)(heap_min_limit);
    // Do not reallocate the first page, which we created manually.
    heap_min_limit += size_to_bytes(MAX_MEM_BLK_SIZE);

    // Iterate over memory region and fill with blocks.
    uint64_t block_p = (uint64_t)heap_end;
    while(block_p >= (uint64_t)heap_min_limit + size_to_bytes(MAX_MEM_BLK_SIZE)) {
        block_p -= size_to_bytes(MAX_MEM_BLK_SIZE);
        if(!alloc_new_block(MAX_MEM_BLK_SIZE, (void*)block_p)) {
            return 1;
        }
    }

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


unsigned int buddy_free_block_structs() {
    return buddy_pool.free_objects;
}


unsigned int buddy_used_block_structs() {
    return buddy_pool.objects - buddy_pool.free_objects;
}


static void print_block_tree(struct mem_blk *b, char start_depth) {
    term_set_cursor_column((start_depth - b->blk_size) * 2 + 1);
    print("{x} - {x}", (uint32_t)(uint64_t)b->start_addr, (uint32_t)(uint64_t)b->start_addr + MEM_BLK_BYTES(b->blk_size));

    if(b->allocated) {
        term_set_cursor_column(start_depth * 2 + 1 + 20);
        print(" [allocated]");
    }
    print("\r\n");
    if(b->child1 || b->child2) {
        // Block is split
        print_block_tree(b->child1, start_depth);
        print_block_tree(b->child2, start_depth);
    } else {
        // Block is not split
    }
}

static void print_block(struct mem_blk *b) {
    print("Block(size={i},alloc={i}", (int)b->blk_size, (int)b->allocated);
    if(b->child1 || b->child2) {
        print(",children=1");
    } else {
        print(",children=0");
    }
    print(",addr={p})\r\n", b->start_addr);
}

int monoterm_buddy_print_free_lists(int argc, char* argv[]) {
    if(argc != 3) {
        print("Requires block size argument.\r\n");
        return 1;
    }
    int blk_size;
    if(atoi(argv[2], &blk_size)) {
        print("Invalid block size.\r\n");
        return 1;
    }
    if(blk_size < 0 || blk_size > MAX_MEM_BLK_SIZE) {
        print("Invlid block size.\r\n");
        return 1;
    }

    struct mem_blk *b = blk_heads[BLK_HEAD_IDX(blk_size)];
    while(b) {
        print_block(b);
        b = b->next_free;
    }
    return 0;
}

int monoterm_buddy_print_map(int argc, char* argv[]) {
    struct mem_blk *b = blk_all_head;
    while(b) {
        if(b->blk_size == MAX_MEM_BLK_SIZE) {
            print_block_tree(b, b->blk_size);
        }
        b = b->next;
    }
    return 0;
}