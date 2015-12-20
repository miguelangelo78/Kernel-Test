#include <system.h>
#include <memory.h>

namespace Kernel {
namespace Memory {
namespace Man {

#define ALIGN(frame) (frame) * PAGE_SIZE
#define DEALIGN(frame) (frame) / PAGE_SIZE

/* Returns a table from the directory */
#define TABLE(dir, address) (dir).page_tables[INDEX_FROM_BIT((address/PAGE_SIZE), PAGES_PER_TABLE)]
/* Returns a page from a table from the directory */
#define PAGE(dir, address) TABLE((dir), address)[OFFSET_FROM_BIT((address/PAGE_SIZE), PAGES_PER_TABLE)]

#define DIR_PAGE_IT() for (uintptr_t table_ctr = 0; table_ctr < table_count; table_ctr++) { \
						for (int page_ctr = 0; page_ctr < PAGES_PER_TABLE; page_ctr++) \

page_directory_t kerneldir;
page_directory_t * curr_dir = &kerneldir;
uintptr_t frame_count;
uintptr_t table_count;

uintptr_t frame_ptr = (uintptr_t)&end; /* Placement pointer to allocate data on the heap */
uintptr_t heap_top = 0; /* Top of the heap's address. Only used after paging is enabled */
uintptr_t kernel_heap_alloc_point = KERNEL_HEAP_INIT;

void page_fault(struct regs *r);

void kmalloc_starts(uintptr_t start_addr) {
	frame_ptr = start_addr;
}

/* 'Real' kmalloc */
uintptr_t kmalloc(size_t size, char align, uintptr_t * phys) {
	if (heap_top) {
		/* Use proper malloc */
		void * address = 0;
		/* TODO */
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

uintptr_t clone_directory(page_directory_t * src) {

}

uintptr_t clone_table(page_directory_t * src, uintptr_t physical_address) {
	
}

void switch_directory(page_directory_t * dir) {
	curr_dir = dir;
	/* Enable paging: */
	asm volatile (
		"mov %0, %%cr3\n"
		"mov %%cr0, %%eax\n"
		"orl $0x80000000, %%eax\n"
		"mov %%eax, %%cr0\n"
		:: "r"(dir->page_directory)
		: "%eax");
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
	return PAGE(*curr_dir, virtual_addr) & 0xFFFFF000;
}

void alloc_table(page_directory_t * dir, int is_kernel, int is_writeable, uintptr_t physical_address) {
	unsigned int * page_dir_ptr = &dir->page_directory[INDEX_FROM_BIT((physical_address - 0x1000) / 0x1000, 1024)];
	*page_dir_ptr = ((unsigned int)dir->page_tables[INDEX_FROM_BIT((physical_address - 0x1000) / 0x1000, 1024)] & 0xFFFFF000) | 1;
	BIT_WRITE(is_writeable, *page_dir_ptr, 1); /* RW */
	BIT_WRITE(is_kernel, *page_dir_ptr, 2); /* User */
}

void dealloc_table(page_directory_t * dir, uintptr_t virtual_address) {
	unsigned int * page_dir_ptr = &dir->page_directory[INDEX_FROM_BIT((virtual_address - 0x1000) / 0x1000, 1024)];
	BIT_CLEAR(*page_dir_ptr, 0); /* Not present */
}

void alloc_page(page_directory_t * dir, int is_kernel, int is_writeable, uintptr_t physical_address) {
	unsigned int * page = &PAGE(*dir, physical_address);
	*page = (physical_address & 0xFFFFF000) | 1;
	BIT_WRITE(is_writeable, *page, 1); /* RW */
	BIT_WRITE(is_kernel, *page, 2); /* User */

	/* Alloc table in case it wasn't already: */
	if(!OFFSET_FROM_BIT((physical_address - 0x1000) / 0x1000, PAGES_PER_TABLE))
		alloc_table(curr_dir, is_kernel, is_writeable, physical_address);
}

void dealloc_page(page_directory_t * dir, uintptr_t page_index) {
	unsigned int * page = &PAGE(*dir, ALIGN(page_index));
	BIT_CLEAR(*page, 0); /* Not present */
}

unsigned int page_directory[1024] __attribute__((aligned(4096)));
unsigned int page_tables[1024][1024] __attribute__((aligned(4096)));
void paging_install_example() {
	uint32_t phys_addr = 0;
	uint32_t table_count = 512;
	for (int i = 0; i < table_count; i++) { /* for every table ...*/
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
	Kernel::term.printf("%c", *c);
}

void paging_enable(uint32_t memsize) {
#if 0
	paging_install_example();
	return;
#endif
	frame_count = memsize / 4;
	table_count = frame_count / TABLES_PER_DIR + 1;
	
	/* Install page fault handler: */
	Kernel::CPU::ISR::isr_install_handler(Kernel::CPU::IDT::IDT_IVT::ISR_PAGEFAULT, (Kernel::CPU::ISR::isr_handler_t)page_fault);

	uint32_t phys_addr = 0;
	for (int i = 0; i < table_count; i++) { /* for every table ...*/
		for (int j = 0; j < PAGES_PER_TABLE; j++, phys_addr += PAGE_SIZE) /* every page ...  */
			alloc_page(curr_dir, 1, 0, phys_addr);
	}

	switch_directory(curr_dir);
}

void heap_install(void) {
	heap_top = (frame_ptr + PAGE_SIZE) & ~0xFFF;
}

void page_fault(struct regs *r) {
	Kernel::Error::panic("Page fault");
}

}
}
}