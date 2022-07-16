#include <kernel/alloc.h>
#include <kernel/string.h>
#include <kernel/print.h>
#include <kernel/types.h>


int static test_create_and_free(int argc, char *argv[]) {
    if(argc != 4) {
        print("Usage: memtest create_and_free [size of blocks] [number of blocks]\r\n");
        return 1;
    }
    int block_size;
    if(atoi(argv[2], &block_size)) {
        print("Invalid argument.\r\n");
        return 1;
    }
    int number_blocks;
    if(atoi(argv[3], &number_blocks)) {
        print("Invalid argument.\r\n");
        return 1;
    }

    print("Allocating pointers...\r\n");
    void** pointers = (void**)kmalloc(sizeof(void*) * number_blocks, 0);
    if(!pointers) {
        print("Failed to allocate memory for pointers.\r\n");
        return 1;
    }

    print("Allocating memory blocks...\r\n");
    for(int i = 0; i < number_blocks; i++) {
        pointers[i] = kmalloc(block_size, 0);
        if(pointers[i] == 0) {
            print("Alloc. failed for block {i}\r\n", i);
        } else {
            print("{p}\r\n", pointers[i]);
        }
    }
    print("Releasing...\r\n");
    for(int i = 0; i < number_blocks; i++) {
        if(pointers[i]) {
            free(pointers[i]);
        }
    }
    free(pointers);
    print("Done!\r\n");

    return 0;
}

int monoterm_memtest(int argc, char *argv[]) {
    if(argc >= 2) {
        // Two arguments
        if(!strcmp(argv[1], "create_and_free")) {
            return test_create_and_free(argc, argv);
        }
    }
    
    print("Usage: memtest [create_and_free]\r\n");
    return 1;
}