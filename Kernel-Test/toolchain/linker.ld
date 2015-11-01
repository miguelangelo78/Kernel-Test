/* 
	The kernel will live at 3GB + 1MB in the virtual
    address space, which will be mapped to 1MB in the
    physical address space. 
	Reference: http://wiki.osdev.org/Higher_Half_bare_bones 
*/

ENTRY(_Kernel_Start)
OUTPUT_FORMAT(elf32-i386)

SECTIONS {
	. = 0xC0100000;

	start = .;

	/* align at 4 KB and load at 1 MB + . */
    .text : AT(ADDR(.text) - 0xC0000000) {
		*(.text)
	}

	/* align at 4 KB and load at 1 MB + . */
    .rodata ALIGN (0x1000) : AT(ADDR(.rodata)-0xC0000000)
    {
        *(.rodata*) /* all read-only data sections from all files */
    }
	
   /* align at 4 KB and load at 1 MB + . */
   .data ALIGN (0x1000) : AT(ADDR(.data) - 0xC0000000) {
	   *(.data)
   }

   /* align at 4 KB and load at 1 MB + . */
   .bss ALIGN (0x1000) : AT(ADDR(.bss) - 0xC0000000) {
       *(.bss)
    }

	end = .;
}