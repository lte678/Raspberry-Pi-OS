#pragma once

#include <kernel/types.h>
#include <kernel/inode.h>


#define ELF_CLASS_32BIT 1
#define ELF_CLASS_64BIT 2
#define ELF_LITTLE_ENDIAN 1
#define ELF_BIG_ENDIAN 1
// Instruction sets
#define ELF_INSTR_ARM64 0xB7
// Object types
#define ELF_OBJ_EXECUTABLE 0x02


struct elf_header {
    // 32-bit or 64-bit?
    uint8_t elf_class;
    uint8_t endianness;
    uint8_t version;
    uint8_t abi;
    uint8_t abi_version;
    uint16_t object_type;
    uint16_t instruction_set;
    uint64_t entry_point;
    uint64_t pheader_addr;
    uint64_t sheader_addr;
    uint16_t pheader_num;
    uint16_t sheader_num;
    uint16_t pheader_size;
    uint16_t sheader_size;
    uint16_t sheader_name_index;
    uint32_t flags;
};


struct elf_program_header {
    struct elf_program_header *next;
    uint32_t segment_type;
    uint32_t segment_flags;
    uint64_t file_address;
    uint64_t proc_address;
    uint64_t file_size;
    uint64_t proc_size;
    uint64_t alignment;
};


struct elf_data {
    struct inode *node;
    struct elf_header header;
    struct elf_program_header *pheader;
};


int is_elf(void* data);
int load_elf_header(struct elf_data *elf);
int load_elf_program_header(struct elf_data *elf);