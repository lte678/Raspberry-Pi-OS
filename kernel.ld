

SECTIONS {
	__static_memory_start = .;

	/* The kernel entry point:  */

	__entry_point = 0x80000;

	.stack : {
		. = __entry_point;
		__sp = .;
	}

	/* Address = __entry_point */
	.init : {
		*(.init)
	}
	
	/* 
	* Next we put the rest of the code.
	*/
	.text : {
		*(.text)
	}
	
	/* 
	* Next we put the data.
	*/
	.data : {
		*(.data)
	}
	.rodata : {
		*(.rodata .rodata.*)
	}
	
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
