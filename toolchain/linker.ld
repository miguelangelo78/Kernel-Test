OUTPUT_FORMAT(elf32-i386) 
ENTRY(start)

SECTIONS
{
	/* Begin putting sections at 1 MiB, a conventional place for kernels to be
	   loaded at by the bootloader. */
	. = 1M;
	kstart = .;
	phys = .;

	/* Kernel code: */
	.text BLOCK(4K) : ALIGN(4K)
	{
		PROVIDE(kernel_symbols_start = .);
		*(.symbols);
		. += 0x2000 - .; /* Allocate 2 pages for the symbol table */
		PROVIDE(kernel_symbols_end = .);
	 	code = .;
		*(.multiboot)
		*(.text)
	}

	/* Read only data */
	.rodata BLOCK(4K) : ALIGN(4K)
	{
		rodata = .;
		*(.rodata)
	}

	/* Read-write data (initialized) */
	.data BLOCK(4K) : ALIGN(4K)
	{
		data = .;
		*(.data)
	}

	/* Read-write data (uninitialized) plus stack */
	.bss BLOCK(4K) : ALIGN(4K)
	{
		bss = .;
		*(COMMON)
		*(.bss)
		*(.stack)
	}

	end = .;

	/DISCARD/ :
	{
		*(.comment)
		*(.eh_frame)
		*(.note.gnu.build-id)
	}
}