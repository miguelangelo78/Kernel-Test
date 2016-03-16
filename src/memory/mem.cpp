#include <system.h>

namespace Kernel {
namespace Memory {
namespace Man {

/************ USEFUL MACROS FOR ACESSING THE PAGING DIRECTORY: ***********/
#define ALIGNP(page) (page) * PAGE_SIZE
#define DEALIGNP(page) (page) / PAGE_SIZE

/* Returns a page from a table from the directory */
#define PAGE(address) (&curr_dir->tables[INDEX_FROM_BIT((address)/PAGE_SIZE, PAGES_PER_TABLE)].pages[OFFSET_FROM_BIT((address)/PAGE_SIZE, PAGES_PER_TABLE)])
/* Returns a table entry from the directory */
#define TABLE_ENTRY(address) (&curr_dir->table_entries[INDEX_FROM_BIT((address)/PAGE_SIZE, PAGES_PER_TABLE)])

#define IPAGE(table_index, page_index) (&curr_dir->tables[table_index].pages[page_index])
#define ITABLE_ENTRY(table_index) (&curr_dir->table_entries[table_index])

#define DIR_PAGE_IT() for (uintptr_t table_ctr = 0; table_ctr < table_count; table_ctr++) { \
						for (int page_ctr = 0; page_ctr < PAGES_PER_TABLE; page_ctr++)
#define DIR_PAGE_ITST(startaddr) for (uintptr_t table_ctr = startaddr; table_ctr < table_count; table_ctr++) { \
								for (int page_ctr = 0; page_ctr < PAGES_PER_TABLE; page_ctr++)
#define DIR_PAGE_ITADDR() for(uintptr_t page_ctr = 0; page_ctr < PAGES_PER_TABLE * frame_count; page_ctr += PAGE_SIZE)
#define DIR_PAGE_ITADDRST(startaddr) for(uint32_t page_ctr = startaddr; page_ctr < ALIGNP(PAGES_PER_TABLE * page_count); page_ctr += PAGE_SIZE)
/*************************************************************************/

paging_directory_t kernel_directory; /* Declare kernel_directory "statically" */
paging_directory_t * curr_dir;
uintptr_t page_count; /* How many pages in TOTAL */
uintptr_t table_count; /* How many tables in TOTAL */

uintptr_t frame_ptr = (uintptr_t)&end; /* Placement pointer to allocate data on the heap */
uintptr_t heap_tail = 0; /* Start of the heap. Only used after paging is enabled */
uintptr_t heap_head = KERNEL_HEAP_INIT; /* End of heap space */
uintptr_t last_known_newpage = 0;
bool is_paging_enabled = 0;

void page_fault(struct regs *r); /* Function Prototype */
uintptr_t map_to_physical(uintptr_t virtual_addr); /* Function Prototype */

void kmalloc_starts(uintptr_t start_addr) {
	frame_ptr = start_addr;
}

/* 'Real' kmalloc */
uintptr_t kmalloc(size_t size, char align, uintptr_t * phys) {
	if (heap_tail) {
		/* Use proper malloc */
		void * address = align ? valloc(size) : malloc(size);
		if(phys)
			*phys = map_to_physical((uintptr_t)address);
		return (uintptr_t)address;
	}

	/* Else: use dumb kmalloc without paging */

	if (align && (frame_ptr & 0xFFFFF000)) {
		frame_ptr &= 0xFFFFF000;
		frame_ptr += PAGE_SIZE;
	}

	if (phys)
		*phys = frame_ptr;

	uintptr_t addr = frame_ptr;
	frame_ptr += size;
	return addr;
}

/* Normal kmalloc */
uintptr_t kmalloc(size_t size) {
	return kmalloc(size, 0, 0);
}

/* Aligned */
uintptr_t kvmalloc(size_t size) {
	return kmalloc(size, 1, 0);
}

/* With a physical address */
uintptr_t kmalloc_p(size_t size, uintptr_t *phys) {
	return kmalloc(size, 0, phys);
}

/* Aligned, with a physical address */
uintptr_t kvmalloc_p(size_t size, uintptr_t *phys) {
	return kmalloc(size, 1, phys);
}

paging_directory_t * clone_directory(paging_directory_t * src) {
	paging_directory_t * new_dir = (paging_directory_t*)kvmalloc(sizeof(paging_directory_t));
	memcpy(new_dir, src, sizeof(paging_directory_t));
	return new_dir;
}

bool check_paging(void) {
	return is_paging_enabled;
}

void enable_paging(void) {
	/* Enable paging: */
	asm volatile (
		"mov %cr0, %eax\n"
		"orl $0x80000000, %eax\n"
		"mov %eax, %cr0\n"
	);
	is_paging_enabled = 1;
}

void disable_paging(void) {
	/* Disable paging: */
	asm volatile (
		"mov %cr0, %eax\n"
		"xorl $0x80000000, %eax\n"
		"mov %eax, %cr0\n"
	);
	is_paging_enabled = 0;
}

void switch_directory(paging_directory_t * dir) {
	curr_dir = dir;
	/* Install directory pointer to cr3: */
	asm volatile (
		"mov %0, %%cr3\n"
		:: "r"(dir->table_entries)
		: "%eax"
	);
}

void invalidate_page_tables(void) {
	asm volatile (
		"movl %%cr3, %%eax\n"
		"movl %%eax, %%cr3\n"
		::: "%eax");
}

void invalidate_tables_at(uintptr_t addr) {
	asm volatile (
		"movl %0,%%eax\n"
		"invlpg (%%eax)\n"
		:: "r"(addr) : "%eax");
}

uintptr_t map_to_physical(uintptr_t virtual_addr) {
	return PAGE(virtual_addr)->phys_addr << 12;
}

/* Returns the address of the next available (non-present) page: */
uintptr_t find_new_page(void) {
	DIR_PAGE_ITADDRST(last_known_newpage) /* Iterate through all pages starting at a given location, in order to find a non-present page */
		if(!PAGE(page_ctr)->present) return page_ctr; /* Check if it's present. Note: page_ctr is already ALIGNED */
	return 0;
}

void alloc_table(int is_kernel, int is_writeable, uintptr_t physical_address) {
	page_table_entry * table = TABLE_ENTRY(physical_address);
	table->table_address = (uintptr_t)&curr_dir->tables[INDEX_FROM_BIT((physical_address)/PAGE_SIZE, PAGES_PER_TABLE)] >> 12; /* THIS IS IMPORTANT */
	table->rw = is_writeable ? 1 : 0; /* RW */
	table->user = is_kernel ? 0 : 1; /* User */
	table->present = 1; /* Table is present */
	table->page_size = 0; /* 4KB page size */
}

void dealloc_table(uintptr_t virtual_address) {
	page_table_entry * table = TABLE_ENTRY(virtual_address);
	table->present = 0;
	table->rw = 0;
	table->user = 0;
}

/* Allocate page by providing the physical address AND the virtual address, to which the physical address will map to: */
void alloc_page(char is_kernel, char is_writeable, uintptr_t physical_address, uintptr_t map_to_virtual) {
	page_t * page = PAGE(physical_address);
	page->phys_addr = map_to_virtual >> 12;
	page->rw = is_writeable ? 1 : 0;
	page->user = is_kernel ? 0 : 1;
	page->present = 1;

	/* Alloc table in case it wasn't already: */
	if(!TABLE_ENTRY(physical_address)->present)
		alloc_table(is_kernel, is_writeable, physical_address);
}

/* Allocate page by providing the physical address (uses identity mapping): */
void alloc_page(char is_kernel, char is_writeable, uintptr_t physical_address) {
	alloc_page(is_kernel, is_writeable, physical_address, physical_address); /* Identity mapping */
}

/* Allocate page without providing physical address. The function should guess the next page */
void alloc_page(char is_kernel, char is_writeable) {
	alloc_page(is_kernel, is_writeable, (last_known_newpage = find_new_page()));
	last_known_newpage += PAGE_SIZE;
}

void dealloc_page(uintptr_t physical_address) {
	PAGE(physical_address)->present = 0;
	last_known_newpage = ALIGNP(physical_address);
}

/* Simply remaps physical address to a virtual one: */
void map_page(uintptr_t physical_address, uintptr_t virtual_address) {
	PAGE(physical_address)->phys_addr = virtual_address >> 12;
}

/* Simply remaps physical address to a virtual one, and "updates"/invalidates the table */
void map_page_update(uintptr_t physical_address, uintptr_t virtual_address) {
	map_page(physical_address, virtual_address);
	invalidate_tables_at(physical_address);
}

/****** PAGING EXAMPLE  ************/
unsigned int page_directory[1024] __attribute__((aligned(4096)));
unsigned int page_tables[1024][1024] __attribute__((aligned(4096)));
void paging_install_example() {
	uintptr_t phys_addr = 0;
	for (int i = 0; i < 1024; i++) { /* for every table ...*/
		for (int j = 0; j < 1024; j++) { /* and every page ...  */
			page_tables[i][j] = (phys_addr & 0xFFFFF000) | 3; // attributes: supervisor level, read/write, present.
			phys_addr += 0x1000;
		}
		/* for every table pointer ... */
		page_directory[i] = ((unsigned int)page_tables[i] & 0xFFFFF000) | 0x00000003;
	}

	/* Enable paging: */
	asm volatile (
		"mov %0, %%cr3\n"
		"mov %%cr0, %%eax\n"
		"orl $0x80000000, %%eax\n"
		"mov %%eax, %%cr0\n"
		:: "r"(page_directory)
		: "%eax");
	/* Test it: */
	uintptr_t * c = (uintptr_t*)-10;
	kprintf("%c", *c);
}
/****** PAGING EXAMPLE - END ******/

void mem_test(void) {
	/* All memory testing will go here */
	#define mem(addr) (*(char*)addr)

}

void paging_install(uint32_t memsize) {
#if 0
	paging_install_example(); return;
#endif
	/* Point current directory to kernel_directory: */
	curr_dir = &kernel_directory;

	page_count = memsize / 4;
	table_count = page_count / TABLES_PER_DIR + 1;

	kprintf("(Pages: %d Tables: %d) ", page_count, table_count);

	/* Install page fault handler: */
	Kernel::CPU::ISR::isr_install_handler(Kernel::CPU::IDT::IDT_IVT::ISR_PAGEFAULT, (Kernel::CPU::ISR::isr_handler_t)page_fault);

	/* Initialize paging directory: */
	for (uintptr_t table_ctr = 0; table_ctr < table_count; table_ctr++) {
		memset(&curr_dir->table_entries[table_ctr], 0, sizeof(page_table_entry));
		for (int page_ctr = 0; page_ctr < PAGES_PER_TABLE; page_ctr++)
			memset(&curr_dir->tables[table_ctr].pages[page_ctr], 0, sizeof(page_t));
	}

	/* Main allocations: */
	for (uintptr_t i = 0; i <= frame_ptr; i += PAGE_SIZE)
		alloc_page(1, 0);
	
	/* VGA Text mode (user mode): */
	for (uintptr_t i = 0xB8000; i <= 0xBF000; i += PAGE_SIZE)
		alloc_page(0, 1, i);

	/* Preallocate some extra heap: */
	if (KERNEL_HEAP_INIT <= frame_ptr)
		heap_head = frame_ptr + (memsize*1024); /* Allocate the rest of the memory (TODO: FIX THIS, we don't want to allocate for the entire memory...) */
	for (uintptr_t i = frame_ptr; i < heap_head; i += PAGE_SIZE)
		alloc_page(1, 0);

	switch_directory(curr_dir);
	enable_paging();
	/* Finally, set up heap pointer to the start of the heap:  */
	heap_install();
	mem_test();
}

void heap_install(void) {
	heap_tail = (frame_ptr + PAGE_SIZE) & ~0xFFF;
}

void * sbrk(uintptr_t increment) { 
	uintptr_t * address = (uintptr_t*)heap_tail;
	if(heap_tail + increment > heap_head) {
		for (uintptr_t i = heap_tail; i < heap_tail + increment; i += PAGE_SIZE)
			alloc_page(1, 0);
		invalidate_page_tables();
	}
	heap_tail += increment;
	memset(address, 0, increment);
	return address;
}

void page_fault(struct regs *r) {
	Kernel::Error::panic("Page fault");
}

}
}
}
