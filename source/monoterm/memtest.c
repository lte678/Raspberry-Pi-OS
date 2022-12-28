#include <kernel/alloc.h>
#include <kernel/string.h>
#include <kernel/print.h>
#include <kernel/types.h>


int static test_create_and_free(int argc, char *argv[]) {
    if(argc != 4) {
        print("Usage: memtest create_and_free [size of blocks] [number of blocks]\n");
        return 1;
    }
    int block_size;
    if(atoi(argv[2], &block_size)) {
        print("Invalid argument.\n");
        return 1;
    }
    int number_blocks;
    if(atoi(argv[3], &number_blocks)) {
        print("Invalid argument.\n");
        return 1;
    }

    print("Allocating pointers...\n");
    void** pointers = (void**)kmalloc(sizeof(void*) * number_blocks, 0);
    if(!pointers) {
        print("Failed to allocate memory for pointers.\n");
        return 1;
    }

    print("Allocating memory blocks...\n");
    for(int i = 0; i < number_blocks; i++) {
        pointers[i] = kmalloc(block_size, 0);
        if(pointers[i] == 0) {
            print("Alloc. failed for block {i}\n", i);
        } else {
            print("{p}\n", pointers[i]);
        }
    }
    print("Releasing...\n");
    for(int i = 0; i < number_blocks; i++) {
        if(pointers[i]) {
            free(pointers[i]);
        }
    }
    free(pointers);
    print("Done!\n");

    return 0;
}

int monoterm_memtest(int argc, char *argv[]) {
    if(argc >= 2) {
        // Two arguments
        if(!strcmp(argv[1], "create_and_free")) {
            return test_create_and_free(argc, argv);
        }
    }
    
    print("Usage: memtest [create_and_free]\n");
    return 1;
}