#include <kernel/string.h>
#include <kernel/print.h>
#include <kernel/mem.h>
#include <kernel/elf.h>
#include <kernel/process.h>
#include <kernel/page.h>
#include <kernel/pagetable.h>
#include <kernel/util.h>
#include <kernel/address_space.h>


struct elf_data *alloc_elf_data() {
    return kmalloc(sizeof(struct elf_data), ALLOC_ZERO_INIT);
}


/**
 * @brief Checks if the elf magic is legit.
 * 
 * @param data Pointer to first 4 bytes of data.
 * @return 1 if valid ELF file
 */
int is_elf(void* data) {
    char* header = data;
    char magic[] = {0x7F, 'E', 'L', 'F'};
    if(strncmp(header, magic, 4) == 0) {
        return 1;
    }
    return 0;
}


/**
 * @brief Loads the ELF header from disk.
 * 
 * @param elf Allocated elf_data struct. Node must point to a valid inode.
 * @return 0 for success 
 */
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


/**
 * @brief Loads the program headers contained in the ELF file.
 * 
 * @param elf Must contain the parsed ELF header.
 * @return 0 for success
 */
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


/**
 * @brief Loads the complete ELF file into an elf_data struct
 * 
 * @param elf_file Inode pointing to ELF file.
 * @param elf Target allocated empty struct.
 * @return 0 for success 
 */
int load_elf(struct inode* elf_file, struct elf_data *elf) {
    if(elf_file->state & INODE_TYPE_DIR) {
        print("Target is a directory.\r\n");
        return 1;
    }
    inode_fetch_data(elf_file);

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

    if(!elf) {
        return 1;
    }
    elf->node = elf_file;
    if(load_elf_header(elf)) {
        print("Failed to fully parse ELF header\r\n");
        return 1;
    }
    if(load_elf_program_header(elf)) {
        print("Failed to load ELF program header\r\n");
        return 1;
    }
    return 0;
}


static int elf_create_process_map_address(struct process* p, uint64_t map_from, void *kernel_address, uint64_t section_size) {
    // Translate to physical address
    uint64_t map_to = page_table_virtual_to_physical(kernel_page_table, (uint64_t)kernel_address);
    if(!map_to) {
        return 1;
    }

    // print("Mapping {x} bytes from virtual address {p} to physical address {p}\r\n", (uint32_t)section_size, map_from, map_to);

    // Attemp to perform mapping
    if(map_memory_region(p->addr_space, map_from, map_to, section_size)) {
        print("Failed to map memory region for ELF file\r\n");
        return 1;
    }

    return 0;
}


static void *elf_create_add_memory_region(struct process *p, uint64_t size) {
    // Allocate memory. This returns a kernel virtual address.
    void* header_memory = kmalloc(size, ALLOC_ZERO_INIT | ALLOC_PAGE_ALIGN);
    if(!header_memory) {
        print("Failed to allocate process memory!\r\n");
        return 0;
    }

    struct process_memory_handle *al = kmalloc(sizeof(struct process_memory_handle), 0);
    if(!al) {
        free(header_memory);
        free_process_memory(p);
        return 0;
    }
    struct process_memory_handle *old_al = p->allocated_list;
    al->next = old_al;
    al->memory = header_memory;
    p->allocated_list = al;
    return header_memory;
}


/**
 * @brief Creates a new process from an elf file.
 * 
 * @param elf Populated elf_data struct containing program headers.
 * @param p Allocated empty process struct.
 * @return 0 for success 
 */
int elf_create_process(struct elf_data *elf, struct process *p) {
    // Set the processes entry point
    p->user_thread->link_reg = elf->header.entry_point;
    p->user_thread->stack_size = USERSPACE_PREALLOCATED_STACK_SIZE;
    p->kern_thread->stack_size = KERNEL_STACK_SIZE;

    // Create memory mappings and allocate RAM
    for(int i = 0; i < elf->header.pheader_num; i++) {
        struct elf_program_header *hdr = &(elf->pheader[i]);
        // Map this header to memory

        // The offset to the previous page boundary within the file
        uint64_t file_page_offset = hdr->file_address % PAGE_SIZE;
        // We will start mapping from the previous page boundary,
        // this means that the target virtual address should also be
        // offset by file_page_offset

        uint64_t map_from = hdr->proc_address - file_page_offset;
        if(map_from % PAGE_SIZE) {
            // ELF segments not page aligned!
            // TODO: Free all of the process memory allocated up to now.
            return 1;
        }

        // Append memory region to the processes allocation lists
        void* header_memory = elf_create_add_memory_region(p, hdr->proc_size);
        if(!header_memory) {
            return 1;
        }

        reterr(elf_create_process_map_address(p, map_from, header_memory, round_up_to_page(hdr->proc_size)));

        // Copy data
        memcpy((char*)header_memory + file_page_offset, elf->node->data + hdr->file_address, hdr->file_size);
    }

    // Allocate ram for the stack
    // Allocate memory. This returns a kernel virtual address.
    void* stack_memory = elf_create_add_memory_region(p, p->user_thread->stack_size);
    if(!stack_memory) {
        print("Failed to allocate process user stack!\r\n");
        return 1;
    }
    uint64_t stack_begin = (uint64_t)p->user_thread->sp - p->user_thread->stack_size;
    reterr(elf_create_process_map_address(p, stack_begin, stack_memory, p->user_thread->stack_size));


    // Create the necessary kernel context
    void* kernel_stack = elf_create_add_memory_region(p, p->kern_thread->stack_size);
    if(!kernel_stack) {
        print("Failed to allocate process kernel stack!\r\n");
        return 1;
    }
    p->kern_thread->sp = (uint64_t)kernel_stack;
    p->kern_thread->link_reg = (uint64_t)(switch_to_user_thread);

    return 0;
}
