/* 
	The kernel will live at 3GB + 1MB in the virtual
    address space, which will be mapped to 1MB in the
    physical address space. 
	Reference: http://wiki.osdev.org/Higher_Half_bare_bones 
*/

ENTRY(_Kernel_Start)
OUTPUT_FORMAT(elf32-i386)

SECTIONS {
	. = 0xC0100000; /* the code should be relocated to 3GB + 1MB */
	
	start = .;

    /* load at 1 MB */
    .text : AT(ADDR(.text)-0xC0000000)
    {
        *(.text)            /* all text sections from all files */
    }

    /* load at 1 MB + . */
    .rodata : AT(ADDR(.rodata)-0xC0000000)
    {
        *(.rodata*)         /* all read-only data sections from all files */
    }

    /* load at 1 MB + . */
    .data : AT(ADDR(.data)-0xC0000000)
    {
        *(.data)            /* all data sections from all files */
    }

	/* align at 4 KB and load at 1 MB + . */
    .paging_sect ALIGN (0x1000) : AT(ADDR(.paging_sect)-0xC0000000)
    {
        *(.paging_sect)  /* all paging_sect sections from all files */
    }

	/* load at 1 MB + . */
    .bss : AT(ADDR(.bss)-0xC0000000)
    {
        *(COMMON)           /* all COMMON sections from all files */
        *(.bss)             /* all bss sections from all files */
    }

	end = .;
}