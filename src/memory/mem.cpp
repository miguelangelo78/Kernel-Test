#include <system.h>

namespace Kernel {
namespace Memory {
namespace Man {

/************ USEFUL MACROS FOR ACESSING THE PAGING DIRECTORY: ***********/
#define ALIGNP(page) (page) * PAGE_SIZE
#define DEALIGNP(page) (page) / PAGE_SIZE

/* Returns a page from a table from the directory */
#define PAGE(dir, address) (&dir->tables[INDEX_FROM_BIT((address)/PAGE_SIZE, PAGES_PER_TABLE)]->pages[OFFSET_FROM_BIT((address)/PAGE_SIZE, PAGES_PER_TABLE)])
/* Returns a table entry from the directory */
#define TABLE_ENTRY(dir, address) (&dir->table_entries[INDEX_FROM_BIT((address)/PAGE_SIZE, PAGES_PER_TABLE)])

#define IPAGE(dir, table_index, page_index) (&dir->tables[table_index].pages[page_index])
#define ITABLE_ENTRY(dir, table_index) (&dir->table_entries[table_index])

#define DIR_PAGE_IT() for (uintptr_t table_ctr = 0; table_ctr < table_count; table_ctr++) { \
						for (int page_ctr = 0; page_ctr < PAGES_PER_TABLE; page_ctr++)
#define DIR_PAGE_ITST(startaddr) for (uintptr_t table_ctr = startaddr; table_ctr < table_count; table_ctr++) { \
								for (int page_ctr = 0; page_ctr < PAGES_PER_TABLE; page_ctr++)
#define DIR_PAGE_ITADDR() for(uintptr_t page_ctr = 0; page_ctr < PAGES_PER_TABLE * frame_count; page_ctr += PAGE_SIZE)
#define DIR_PAGE_ITADDRST(startaddr) for(uint32_t page_ctr = startaddr; page_ctr < ALIGNP(PAGES_PER_TABLE * page_count); page_ctr += PAGE_SIZE)
/*************************************************************************/

paging_directory_t * kernel_directory; /* Declare kernel_directory's pointer */
paging_directory_t * curr_dir;
uintptr_t page_count; /* How many pages in TOTAL */
uintptr_t table_count; /* How many tables in TOTAL */

uintptr_t frame_ptr = (uintptr_t)&end; /* Placement pointer to allocate data on the heap */
uintptr_t heap_tail = 0; /* Start of the heap. Only used after paging is enabled */
uintptr_t heap_head = KERNEL_HEAP_INIT; /* End of heap space */
uintptr_t last_known_newpage = 0;
bool is_paging_enabled = 0;

static spin_lock_t frame_alloc_lock = { 0 };

uintptr_t find_new_page(void); /* Function Prototype */
void page_fault(struct regs *r); /* Function Prototype */
uintptr_t map_to_physical(uintptr_t virtual_addr); /* Function Prototype */

void kheap_starts(uintptr_t start_addr) {
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

void create_page(page_t * page, char is_kernel, char is_writeable) {

}

paging_directory_t * clone_directory(paging_directory_t * src) {
	paging_directory_t * new_dir = (paging_directory_t*)kvmalloc(sizeof(paging_directory_t));
	memset(new_dir, 0, sizeof(paging_directory_t));

	for(uint32_t i = 0; i < TABLES_PER_DIR; i++) {
		if((uintptr_t)&kernel_directory->tables[i] == (uintptr_t)&src->tables[i]) {
			new_dir->tables[i] = src->tables[i];
			new_dir->table_entries[i] = src->table_entries[i];
		} else if (i * PAGE_SIZE * PAGES_PER_TABLE < SHM_START) {
			/* User tables must be cloned: */
			uintptr_t table_phys_addr;
			new_dir->tables[i] = clone_table(src->tables[i], &table_phys_addr);
			new_dir->table_entries[i].present = 1;
			new_dir->table_entries[i].rw      = 1;
			new_dir->table_entries[i].user    = 1;
			new_dir->table_entries[i].table_address = table_phys_addr >> 12;
		}
	}
	return new_dir;
}

page_table_t * clone_table(page_table_t * src, uintptr_t * physAddr) {
	page_table_t * new_table = (page_table_t*)kvmalloc_p(sizeof(page_table_t), physAddr);
	memset(new_table, 0, sizeof(page_table_t));
	for (uint32_t i = 0; i < PAGES_PER_TABLE; i++) {
		/* PHYSICALLY copy ALL pages. This is VERY difficult for the CPU: */
		if(!(uintptr_t)(*((uintptr_t*)&src->pages[i])))
			continue; /* If page is zeroed, just continue */
		/* Allocate new page: */
		alloc_page(&new_table->pages[i], 0, 0, (last_known_newpage = find_new_page()));
		last_known_newpage += PAGE_SIZE;

		if (src->pages[i].present)	new_table->pages[i].present  = 1;
		if (src->pages[i].rw)		new_table->pages[i].rw       = 1;
		if (src->pages[i].user)		new_table->pages[i].user     = 1;
		if (src->pages[i].accessed)	new_table->pages[i].accessed = 1;
		if (src->pages[i].dirty)	new_table->pages[i].dirty    = 1;

		/* Finally, physically copy the pages (tough...): */
		copy_page_physical(ALIGNP(src->pages[i].phys_addr), ALIGNP(new_table->pages[i].phys_addr));
	}
	return new_table;
}

void release_directory(paging_directory_t * dir) {

}

void release_directory_for_exec(paging_directory_t * dir) {

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
	is_paging_enabled = 0;
	/* Disable paging: */
	asm volatile (
		"mov %cr0, %eax\n"
		"xorl $0x80000000, %eax\n"
		"mov %eax, %cr0\n"
	);
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
	return PAGE(curr_dir, virtual_addr)->phys_addr << 12;
}

/* Returns the address of the next available (non-present) page: */
uintptr_t find_new_page(void) {
	DIR_PAGE_ITADDRST(last_known_newpage) /* Iterate through all pages starting at a given location, in order to find a non-present page */
		if(!PAGE(curr_dir, page_ctr)->present) return page_ctr; /* Check if it's present. Note: page_ctr is already ALIGNED */
	return 0;
}

void alloc_table(int is_kernel, int is_writeable, uintptr_t physical_address) {
	page_table_entry * table = TABLE_ENTRY(curr_dir, physical_address);
	uintptr_t phys_addr_table;
	uint32_t table_index = INDEX_FROM_BIT((physical_address)/PAGE_SIZE, PAGES_PER_TABLE);
	curr_dir->tables[table_index] = (page_table_t*)kvmalloc_p(sizeof(page_table_t), &phys_addr_table);
	memset(curr_dir->tables[table_index], 0, sizeof(page_table_t));
	table->table_address = phys_addr_table >> 12; /* THIS IS IMPORTANT!! */
	table->rw = is_writeable ? 1 : 0; /* RW */
	table->user = is_kernel ? 0 : 1; /* User */
	table->present = 1; /* Table is present */
	table->page_size = 0; /* 4KB page size */
}

void dealloc_table(uintptr_t virtual_address) {
	page_table_entry * table = TABLE_ENTRY(curr_dir, virtual_address);
	table->present = 0;
	table->rw = 0;
	table->user = 0;
}

void alloc_page(page_t * page, char is_kernel, char is_writeable, uintptr_t map_to_virtual) {
	spin_lock(frame_alloc_lock);
	page->phys_addr = map_to_virtual >> 12;
	page->rw = is_writeable ? 1 : 0;
	page->user = is_kernel ? 0 : 1;
	page->present = 1;
	spin_unlock(frame_alloc_lock);
}

/* Allocate page by providing the physical address AND the virtual address, to which the physical address will map to: */
void alloc_page(char is_kernel, char is_writeable, uintptr_t physical_address, uintptr_t map_to_virtual) {
	/* Alloc table in case it wasn't already: */
	if(!TABLE_ENTRY(curr_dir, physical_address)->present)
		alloc_table(is_kernel, is_writeable, physical_address);

	alloc_page(PAGE(curr_dir, physical_address), is_kernel, is_writeable, map_to_virtual);
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

void alloc_pages(char is_kernel, char is_writeable, uintptr_t physical_address_start, uintptr_t physical_address_end) { /* Identity */
	for(uintptr_t page_ctr = physical_address_start; page_ctr <= physical_address_end; page_ctr += PAGE_SIZE)
		alloc_page(is_kernel, is_writeable, page_ctr);
}

char alloc_pages(char is_kernel, char is_writeable, uintptr_t physical_address_start, uintptr_t physical_address_end, uintptr_t virtual_addr_start,  uintptr_t virtual_addr_end) {
	if((physical_address_end - physical_address_start) != (virtual_addr_end - virtual_addr_start)) return 0; /* The ranges need to match */

	for(uintptr_t page_ctr = physical_address_start, virt_page_ctr = virtual_addr_start; page_ctr <= physical_address_end; page_ctr += PAGE_SIZE, virt_page_ctr += PAGE_SIZE)
		alloc_page(is_kernel, is_writeable, page_ctr, virt_page_ctr);
	return 1;
}

void dealloc_page(uintptr_t physical_address) {
	PAGE(curr_dir, physical_address)->present = 0;
	last_known_newpage = ALIGNP(physical_address);
}

/* Simply remaps physical address to a virtual one: */
void map_page(uintptr_t physical_address, uintptr_t virtual_address) {
	PAGE(curr_dir, physical_address)->phys_addr = virtual_address >> 12;
}

/* Simply remaps physical address to a virtual one, and "updates"/invalidates the table */
void map_page_update(uintptr_t physical_address, uintptr_t virtual_address) {
	map_page(physical_address, virtual_address);
	invalidate_tables_at(physical_address);
}

void mem_test(char before_paging) {
	/* All memory testing will go here */
	#define mem(addr) (*(char*)addr)

	if(before_paging) {
		/* Test memory before paging: */
	} else {
		/* Test memory after paging: */
	}
}

void paging_install(uint32_t memsize) {
	/* Install page fault handler: */
	Kernel::CPU::ISR::isr_install_handler(Kernel::CPU::IDT::IDT_IVT::ISR_PAGEFAULT, (Kernel::CPU::ISR::isr_handler_t)page_fault);

	mem_test(1);

	/* Initialize paging directory: */
	kernel_directory = (paging_directory_t*)kvmalloc(sizeof(paging_directory_t));
	memset(&curr_dir->table_entries, 0, sizeof(page_table_entry) * table_count);

	/* Point current directory to kernel_directory: */
	curr_dir = kernel_directory;

	page_count = memsize / 4;
	table_count = page_count / TABLES_PER_DIR + 1;

	kprintf("(Pages: %d Tables: %d) ", page_count, table_count);

	/* Allocate the kernel itself (from address 0 to frame_ptr): */
	for (uintptr_t i = 0; i <= frame_ptr; i += PAGE_SIZE)
		alloc_page(1, 1);
	
	/* VGA Text mode (user mode): */
	for (uintptr_t i = 0xB8000; i <= 0xBF000; i += PAGE_SIZE)
		alloc_page(0, 1, i);

	/* Allocate space for the kernel stack: */
	for(uintptr_t i = KInit::init_esp; i > CPU::read_reg(CPU::ebp) - (PAGE_SIZE * STACK_SIZE); i -= PAGE_SIZE)
		alloc_page(1, 1, i);

	/* Finally, allocate the kernel heap: */
	for (uintptr_t i = 0; i <= heap_head; i += PAGE_SIZE)
		alloc_page(1, 1, i);

	switch_directory(curr_dir);
	enable_paging();

	/* Finally, set up heap pointer to the start of the heap:  */
	heap_install();
	mem_test(0);
}

void heap_install(void) {
	heap_tail = (frame_ptr + PAGE_SIZE) & ~0xFFF;
}

void * sbrk(uintptr_t increment) {
	uintptr_t * address = (uintptr_t*)heap_tail;
	if(heap_tail + increment > heap_head) {
		for (uintptr_t i = heap_tail; i < heap_tail + increment; i += PAGE_SIZE)
			alloc_page(1, 1);
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
