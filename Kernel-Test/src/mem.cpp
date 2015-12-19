#include <attr.h>
#include <stdint.h>
#include <system.h>

#define PAGES_PER_TABLE 1024
#define TABLES_PER_DIR 1024
#define PAGE_SIZE 0x1000

#define TABLE(dir, address) (dir).page_tables[INDEX_FROM_BIT((address/PAGE_SIZE), PAGES_PER_TABLE)]
#define PAGE(dir, address) TABLE((dir), address)[OFFSET_FROM_BIT((address/PAGE_SIZE), PAGES_PER_TABLE)]

#define DIR_PAGE_IT() for (uintptr_t table_ctr = 0; table_ctr < table_count; table_ctr++) { \
						for (int page_ctr = 0; page_ctr < PAGES_PER_TABLE; page_ctr++) \

typedef struct {
	unsigned int page_directory[1024] __align(0x1000);
	unsigned int page_tables[1024][1024] __align(0x1000);
} page_directory_t;

page_directory_t kerneldir;
page_directory_t * curr_dir = &kerneldir;
uintptr_t frame_count;
uintptr_t table_count;

void page_fault(struct regs *r) {
	Kernel::Error::panic("Page fault");
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

uintptr_t last_known_unnalocated = 0;

uintptr_t find_new_page(page_directory_t * dir) {
	/*TODO*/
	DIR_PAGE_IT() {
		if(!IS_BIT_SET(dir->page_tables[table_ctr][page_ctr], 0)) {

			return (table_ctr * 32 + page_ctr)*0x1000;
		}
		}
	}
}

void alloc_page_p(page_directory_t * dir, int is_kernel, int is_writeable, uintptr_t physical_address) {
	unsigned int * page = &PAGE(*dir, physical_address);
	*page = (physical_address & 0xFFFFF000) | 1;
	BIT_WRITE(is_writeable, *page, 1);
	BIT_WRITE(is_kernel, *page, 2);
}

void alloc_page(page_directory_t * dir, int is_kernel, int is_writeable) {
	uintptr_t page_addr = find_new_page(dir);
	unsigned int * page = &PAGE(*dir, page_addr);
	*page = (page_addr & 0xFFFFF000) | 1;
	BIT_WRITE(is_writeable, *page, 1);
	BIT_WRITE(is_kernel, *page, 2);
}

void dealloc_page(page_directory_t * dir, uintptr_t physical_address) {
	unsigned int * page = &PAGE(*dir, physical_address);
	BIT_CLEAR(*page, 0);
}

void alloc_table(page_directory_t * dir, int is_kernel, int is_writeable, uintptr_t physical_address) {
	unsigned int * page_dir_ptr = &dir->page_directory[INDEX_FROM_BIT((physical_address - 0x1000) / 0x1000, 1024)];
	*page_dir_ptr = ((unsigned int)dir->page_tables[INDEX_FROM_BIT((physical_address - 0x1000) / 0x1000, 1024)] & 0xFFFFF000) | 1;
	BIT_WRITE(is_writeable, *page_dir_ptr, 1);
	BIT_WRITE(is_kernel, *page_dir_ptr, 2);
}

void dealloc_table(page_directory_t * dir, uintptr_t physical_address) {
	unsigned int * page_dir_ptr = &dir->page_directory[INDEX_FROM_BIT((physical_address - 0x1000) / 0x1000, 1024)];
	BIT_CLEAR(*page_dir_ptr, 0);
}

void paging_enable(uintptr_t memsize) {
	frame_count = memsize / 4;

	/* Install page fault handler: */
	Kernel::CPU::ISR::isr_install_handler(Kernel::CPU::IDT::IDT_IVT::ISR_PAGEFAULT, (Kernel::CPU::ISR::isr_handler_t)page_fault);

	uint32_t phys_addr = 0;
	for (int i = 0; i < (table_count = frame_count / TABLES_PER_DIR + 1); i++) { /* for every table ...*/
		for (int j = 0; j < PAGES_PER_TABLE; j++, phys_addr += PAGE_SIZE) /* every page ...  */
			alloc_page_p(curr_dir, 1, 0, phys_addr);
		alloc_table(curr_dir, 1, 0, phys_addr);
	}
	switch_directory(curr_dir);

	uintptr_t * c = (uintptr_t*)-10;
	Kernel::term.printf("%c", *c);
	for (;;);
}