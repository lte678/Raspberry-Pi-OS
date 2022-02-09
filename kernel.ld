SECTIONS {
	.init 0x80000 : {
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
		*(.rodata.*)
	}
	
	. = ALIGN(4);
	.bss : {
		__bss_start = .;
		*(.bss)
		/* Allows for word based clearing of BSS */
		. = ALIGN(4);
		__bss_end = .;
	}

	/*
	* Finally comes everything else. A fun trick here is to put all other 
	* sections into this section, which will be discarded by default.
	*/
	
	/DISCARD/ : {
		*(*)
	}

	/* Include debug symbols */
}
