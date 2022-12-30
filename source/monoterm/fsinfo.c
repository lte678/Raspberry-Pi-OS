#include <kernel/print.h>
#include "../fs/fat32.h"
#include "bindings.h"


int monoterm_fsinfo(int argc, char *argv[]) {
    if(argc == 1) {
        print("Partition Size    {ul}kiB\n", (g_root_fs->sectors * g_root_fs->bpb->bytes_per_sector) / 1024);
        print_bpb(g_root_fs->bpb);
        return 0;
    } else if(argc == 2) {
        if(!strcmp(argv[1], "bpb")) {
            print_bpb(g_root_fs->bpb);
            return 0;
        } else {
            print("Unknown option \"{s}\"\n", argv[1]);
            return 1;
        }
    }
    print("Usage: fsinfo [bpb]\n");
    return 1;
}