

SECTIONS {
	__static_memory_start = .;

	/* The kernel entry point:  */

	__entry_point = 0x80000;

    __stack_start = .;
	.stack : {
		. = __entry_point;
		__sp = .;
		__stack_end = .;
	}

	/* Address = __entry_point */
	.init : {
	    __text_start = .;
		*(.init)
	}
	
	/* 
	* Next we put the rest of the code.
	*/
	.text : {
		*(.text)
	}
	__text_end = .;

	/* 
	* Next we put the data.
	*/
	.data : {
	    __data_start = .;
		*(.data)
	}
	.rodata : {
		*(.rodata .rodata.*)
	}
	__data_end = .;
	
	. = ALIGN(4);
	.bss : {
		__bss_start = .;
		*(.bss)
		/* Allows for word based clearing of BSS */
		. = ALIGN(4);
		__bss_end = .;
	}
	__static_memory_end = .;

	/*
	* Finally comes everything else. A fun trick here is to put all other 
	* sections into this section, which will be discarded by default.
	*/
	
	/DISCARD/ : {
		*(*)
	}

	/* Include debug symbols */
}
