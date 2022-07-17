#include <kernel/print.h>
#include <kernel/inode.h>
#include <kernel/alloc.h>
#include <kernel/elf.h>


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

    if(elf_file->state & INODE_TYPE_DIR) {
        print("Target is a directory.\r\n");
        return 1;
    }
    inode_read_data(elf_file);

    if(!(elf_file->state & INODE_STATE_VALID)) {
        print("Failed to read file.\r\n");
        return 1;
    }

    if(elf_file->data_size < 0x40) {
        print("File must be 64 bytes or larger to contain an ELF header.\r\n");
        return 1;
    }

    if(!is_elf(elf_file->data)) {
        print("File does not contain an ELF header!\r\n");
        return 1;
    }

    struct elf_header *hdr = kmalloc(sizeof(struct elf_header), 0);
    if(load_elf(elf_file->data, hdr)) {
        print("Failed to fully parse ELF header\r\n");
    }

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

    return 0;
}