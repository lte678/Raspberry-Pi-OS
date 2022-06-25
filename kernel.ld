MEMORY {
	PRE_MMU_MEMORY :  ORIGIN = 0x0000000000000000, LENGTH = 0xFFFFFFFF
	/* Kernel address space starts at the upper half of the 48-bit virtual address space.*/
	POST_MMU_MEMORY : ORIGIN = 0x0000800000000000, LENGTH = 0xFFFFFFFF
}


SECTIONS {
	.stack : {
		__static_memory_start = .;
		__stack_start = .;
		. = 0x80000;
		__sp = .;
		__stack_end = .;
	} > POST_MMU_MEMORY AT > PRE_MMU_MEMORY
	.stack_phony : {
		. += SIZEOF(.stack);
	} > PRE_MMU_MEMORY


	/* The kernel entry point:  */
	/* Address = __entry_point */
	.init : {
		__entry_point = .;
	    __text_start = .;
		*(.init)
		build/memory/virtual_memory.o (.text)
		build/memory/virtual_memory.o (.data .data.*)
		build/memory/virtual_memory.o (.bss)
	} > PRE_MMU_MEMORY
	.init_phony : {
		. += SIZEOF(.init);
	} > POST_MMU_MEMORY AT > PRE_MMU_MEMORY

	/* 
	* Next we put the rest of the code.
	*/
	.text : {
		*(.text)
		__text_end = .;
	} > POST_MMU_MEMORY AT > PRE_MMU_MEMORY

	/* 
	* Next we put the data.
	*/
	.data : {
	    __data_start = .;
		*(.data .data.*)
	} > POST_MMU_MEMORY AT > PRE_MMU_MEMORY
	.rodata : {
		*(.rodata .rodata.*)
		__data_end = .;
	} > POST_MMU_MEMORY AT > PRE_MMU_MEMORY
	


	.bss : ALIGN(4) {
		__bss_start = .;
		*(.bss)
		/* Allows for word based clearing of BSS */
		. = ALIGN(4);
		__bss_end = .;
		__static_memory_end = .;
	} > POST_MMU_MEMORY AT > PRE_MMU_MEMORY

	/*
	* Finally comes everything else. A fun trick here is to put all other 
	* sections into this section, which will be discarded by default.
	*/

	/* Include debug symbols */
}
