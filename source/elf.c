#include <kernel/string.h>
#include <kernel/print.h>
#include <kernel/mem.h>
#include <kernel/elf.h>



int is_elf(void* data) {
    char* header = data;
    char magic[] = {0x7F, 'E', 'L', 'F'};
    if(strncmp(header, magic, 4) == 0) {
        return 1;
    }
    return 0;
}


int load_elf_header(struct elf_data *elf) {
    void* data = elf->node->data;
    struct elf_header *header = &elf->header;
    if(!(elf->node->state & INODE_STATE_VALID)) {
        return 1;
    }

    memset(0, header, sizeof(*header));

    header->elf_class   = unpack_from(data, uint8_t, 0x04);
    header->endianness  = unpack_from(data, uint8_t, 0x05);
    header->version     = unpack_from(data, uint8_t, 0x06);
    header->abi         = unpack_from(data, uint8_t, 0x07);
    header->abi_version = unpack_from(data, uint8_t, 0x08);
    if(header->endianness != ELF_LITTLE_ENDIAN) {
        print("ELF endianness is invalid.\r\n");
        return 1;
    }
    if(header->elf_class != ELF_CLASS_64BIT) {
        print("ELF is not 64-bit.\r\n");
        return 1;
    }

    // This section is endianness and word-width dependent
    header->object_type = unpack_from(data, uint16_t, 0x10);
    header->instruction_set = unpack_from(data, uint16_t, 0x12);
    header->entry_point = unpack_from(data, uint64_t, 0x18);
    header->pheader_addr = unpack_from(data, uint64_t, 0x20);
    header->sheader_addr = unpack_from(data, uint64_t, 0x28);
    header->flags = unpack_from(data, uint32_t, 0x30);
    header->pheader_size = unpack_from(data, uint16_t, 0x36);
    header->pheader_num = unpack_from(data, uint16_t, 0x38);
    header->sheader_size = unpack_from(data, uint16_t, 0x3A);
    header->sheader_num = unpack_from(data, uint16_t, 0x3C);
    header->sheader_name_index = unpack_from(data, uint16_t, 0x3E);
    return 0;
}

int load_elf_program_header(struct elf_data *elf) {
    if(!(elf->node->state & INODE_STATE_VALID)) {
        return 1;
    }
    
    int nr_headers = elf->header.pheader_num;
    if(!elf->pheader) {
        elf->pheader = kmalloc(elf->header.pheader_size * nr_headers, 0);
    }

    struct elf_program_header *hdr = elf->pheader;
    void *data = elf->node->data;
    for(int i = 0; i < nr_headers; i++) {
        uint64_t offset = (uint64_t)data + elf->header.pheader_addr + elf->header.pheader_size*i;
        hdr[i].segment_type = unpack_from(offset, uint32_t, 0x00);
        hdr[i].segment_flags = unpack_from(offset, uint32_t, 0x04);
        hdr[i].file_address = unpack_from(offset, uint64_t, 0x08);
        hdr[i].proc_address = unpack_from(offset, uint64_t, 0x10);
        hdr[i].file_size = unpack_from(offset, uint64_t, 0x20);
        hdr[i].proc_size = unpack_from(offset, uint64_t, 0x28);
        hdr[i].alignment = unpack_from(offset, uint64_t, 0x30);
        if(hdr > 0) {
            hdr[i - 1].next = &hdr[i];
        }
    }
    hdr[nr_headers - 1].next = 0;

    return 0;
}