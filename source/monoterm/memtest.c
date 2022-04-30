#include <kernel/alloc.h>
#include <kernel/string.h>
#include <kernel/print.h>
#include <kernel/types.h>


int static test_create_and_free(int argc, char *argv[]) {
    if(argc != 4) {
        uart_print("Usage: memtest create_and_free [size of blocks] [number of blocks]\r\n");
        return 1;
    }
    int block_size;
    if(atoi(argv[2], &block_size)) {
        uart_print("Invalid argument.\r\n");
        return 1;
    }
    int number_blocks;
    if(atoi(argv[3], &number_blocks)) {
        uart_print("Invalid argument.\r\n");
        return 1;
    }

    uart_print("Allocating pointers...\r\n");
    void** pointers = (void**)kmalloc(sizeof(void*) * number_blocks);
    if(!pointers) {
        uart_print("Failed to allocate memory for pointers.\r\n");
        return 1;
    }

    uart_print("Allocating memory blocks...\r\n");
    for(int i = 0; i < number_blocks; i++) {
        pointers[i] = kmalloc(block_size);
        if(pointers[i] == 0) {
            uart_print("Alloc. failed for block ");
            print_int(i);
            uart_print("\r\n");
        } else {
            print_address(pointers[i]);
            uart_print("\r\n");
        }
    }
    uart_print("Releasing...\r\n");
    for(int i = 0; i < number_blocks; i++) {
        if(pointers[i]) {
            free(pointers[i]);
        }
    }
    free(pointers);
    uart_print("Done!\r\n");

    return 0;
}

int monoterm_memtest(int argc, char *argv[]) {
    if(argc >= 2) {
        // Two arguments
        if(!strcmp(argv[1], "create_and_free")) {
            return test_create_and_free(argc, argv);
        }
    }
    
    uart_print("Usage: memtest [create_and_free]\r\n");
    return 1;
}