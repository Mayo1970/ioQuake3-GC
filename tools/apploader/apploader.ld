OUTPUT_ARCH(powerpc)
ENTRY(entry)

SECTIONS
{
	. = 0x81200000;

	.text : {
		*(.text.entry)
		*(.text)
		*(.text.*)
	}

	.rodata : {
		*(.rodata)
		*(.rodata.*)
	}

	.data : {
		*(.data)
		*(.data.*)
		*(.sdata)
		*(.sdata.*)
	}

	.bss : {
		__bss_start = .;
		*(.bss)
		*(.bss.*)
		*(COMMON)
		*(.sbss)
		*(.sbss.*)
		. = ALIGN(32);
		__bss_end = .;
	}

	/DISCARD/ : {
		*(.comment)
		*(.note*)
		*(.eh_frame*)
	}
}
