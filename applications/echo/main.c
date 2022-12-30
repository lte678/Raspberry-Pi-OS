#include <stdio.h>
#include <stdlib.h>


int main(int argc, char **argv) {
    if(argc < 2) {
        printf("Usage: echo [...]\n");
        return EXIT_FAILURE;
    }
    for(int i = 1; i < argc; i++) {
        printf("%s", argv[i]);
    }
    printf("\n");
    return EXIT_SUCCESS;
}