#include <kernel/print.h>
#include <kernel/inode.h>
#include <kernel/alloc.h>
#include <kernel/elf.h>
#include <kernel/process.h>


int monoterm_elfdump(int argc, char *argv[]) {
    if(argc != 2) {
        print("Invalid number of arguments.\r\nUsage: elfdump [ELF file]\r\n");
        return 1;
    }

    struct inode* elf_file = inode_from_path(g_root_inode, argv[1]);
    if(!elf_file) {
        print("Invalid file\r\n");
        return 1;
    }

    struct elf_data *elf = alloc_elf_data();
    if(!elf) {
        return 1;
    }
    if(load_elf(elf_file, elf)) {
        free(elf);
        return 1;
    }

    struct elf_header *hdr = &elf->header;
    print("ELF Header:\r\n");
    print("    Class:           {i}\r\n", (int)hdr->elf_class);
    print("    Endianness:      {i}\r\n", (int)hdr->endianness);
    print("    ABI:             {i}\r\n", (int)hdr->abi);
    print("    ABI Version:     {i}\r\n", (int)hdr->abi_version);
    print("    Object Type:     0x{x}", (unsigned int)hdr->object_type);
    switch(hdr->object_type) {
    case ELF_OBJ_EXECUTABLE:
        print(" (Executable)");
        break;
    default:
        print(" (Unknown)");
    }
    print("\r\n");
    print("    Instr. Set:      0x{x}", (unsigned int)hdr->instruction_set);
    switch(hdr->instruction_set) {
    case ELF_INSTR_ARM64:
        print(" (ARM 64-bit)");
        break;
    default:
        print(" (Unknown)");
    }
    print("\r\n");
    print("    Flags:           0x{x}\r\n", hdr->flags);
    print("    Program Headers: {i}\r\n", (int)hdr->pheader_num);
    print("    Section Headers: {i}\r\n", (int)hdr->sheader_num);

    if(load_elf_program_header(elf)) {
        print("Failed to load ELF program header\r\n");
        return 1;
    }

    struct elf_program_header *phdr = elf->pheader;
    while(phdr) {
        print("\r\nProgram Header:\r\n");
        print("    Segment Type:   0x{x}\r\n", phdr->segment_type);
        print("    Segment Flags:  0x{x}\r\n", phdr->segment_flags);
        print("    File Offset:    {ul}\r\n", phdr->file_address);
        print("    Size On-Disk:   {ul}\r\n", phdr->file_size);
        print("    Memory Address: 0x{xl}\r\n", phdr->proc_address);
        print("    Memory Size:    {ul}\r\n", phdr->proc_size);
        print("    Alignment:      {ul}\r\n", phdr->alignment);
        phdr = phdr->next;
    }

    return 0;
}


int monoterm_run(int argc, char *argv[]) {
    if(argc != 2) {
        print("Invalid number of arguments.\r\nUsage: elfrun [ELF file]\r\n");
        return 1;
    }

    // Load the ELF file
    struct inode* elf_file = inode_from_path(g_root_inode, argv[1]);
    if(!elf_file) {
        print("Invalid file\r\n");
        return 1;
    }

    struct elf_data *elf = alloc_elf_data();
    if(!elf) {
        return 1;
    }
    if(load_elf(elf_file, elf)) {
        free(elf);
        return 1;
    }

    // Create the process
    struct process *p = allocate_process();
    if(!p) {
        free(elf);
        print("Failed to allocate process struct\r\n");
        return 1;
    }

    if(elf_create_process(elf, p)) {
        free(elf);
        free(p);
        print("Failed to create process!\r\n");
        return 1;
    }

    return 0;
}