#pragma once

#include <stdint.h>

/* NOTE: x86 Virtual Address format -
AAAAAAAAAA         BBBBBBBBBB        CCCCCCCCCCCC
directory index    page table index  offset into page
(http://www.brokenthorn.com/Resources/OSDev18.html) */

#define USER_STACK_BOTTOM 0xAFF00000
#define USER_STACK_TOP    0xB0000000
#define SHM_START         0xB0000000

#define KERNEL_HEAP_INIT 0x00800000
#define KERNEL_HEAP_END  0x20000000
#define KERNEL_HEAP_SIZE KERNEL_HEAP_END - KERNEL_HEAP_INIT

#define PAGES_PER_TABLE 1024
#define TABLES_PER_DIR 1024
#define PAGE_SIZE 0x1000

/* Page definition: */
typedef struct page {
	unsigned int present:1; /* 0: NOT PRESENT 1: PRESENT */
	unsigned int rw:1; /* 0: READ ONLY 1: WRITABLE */
	unsigned int user:1; /* 0: KERNEL MODE 1: USER MODE */
	unsigned int writethrough:1;
	unsigned int cachedisabled:1;
	unsigned int accessed:1; /* 0: NOT ACCESSED 1: ACCESSED */
	unsigned int dirty:1; /* 0: NOT BEEN WRITTEN TO 1: WRITTEN TO */
	unsigned int unused1:1;
	unsigned int global:1;
	unsigned int unused2:3;
	unsigned int phys_addr:20; /* FRAME ADDRESS */
} __packed page_t;

/* Table entry definition: */
typedef struct page_directory_table {
	unsigned int present:1; /* 0: table not present 1: table present */
	unsigned int rw:1; /* 0: table read only 1: table writable */
	unsigned int user:1; /* 0: kernel mode 1: user mode */
	unsigned int writethrough:1; /* 0: write back caching enabled 1: ... disabled */
	unsigned int cachedisabled:1; /* 0: table won't be cached 1: table will be cached */
	unsigned int accessed:1; /* 0: not accessed 1: accessed */
	unsigned int unused1:1;
	unsigned int page_size:1; /* 0: 4kb page sizes 1: 4mb page sizes */
	unsigned int available:4; /* available for use */
	unsigned int table_address:20; /* address of the page directory table */
} __packed page_table_entry;

/* Many pages definition (as a table) */
typedef struct page_table {
	page_t pages[PAGES_PER_TABLE]; /* 4MB per table and 4KB per page */
} page_table_t;

/* Directory definition: */
typedef struct page_directory {
	page_table_entry table_entries[TABLES_PER_DIR];
	page_table_t tables[TABLES_PER_DIR]; /* Array of page tables, covers entire memory space */
} paging_directory_t;

namespace Kernel {
	namespace Memory {
		namespace Man {
			extern bool check_paging(void);
			extern paging_directory_t * clone_directory(paging_directory_t*);
			extern void switch_directory(paging_directory_t * dir);
			extern void enable_paging(void);
			extern void disable_paging(void);
		}
	}
}
