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

typedef struct {
	unsigned int page_directory[TABLES_PER_DIR] __align(PAGE_SIZE);
	unsigned int page_tables[TABLES_PER_DIR][PAGES_PER_TABLE] __align(PAGE_SIZE);
} page_directory_t;

/* UNUSED PAGING DATA STRUCTURES: */
//typedef struct page {
//	unsigned int present:1; /* 0: NOT PRESENT 1: PRESENT */
//	unsigned int rw:1; /* 0: READ ONLY 1: WRITABLE */
//	unsigned int user:1; /* 0: KERNEL MODE 1: USER MODE */
//	unsigned int accessed:1; /* 0: NOT ACCESSED 1: ACCESSED */
//	unsigned int dirty:1; /* 0: NOT BEEN WRITTEN TO 1: WRITTEN TO */
//	unsigned int unused:7;
//	unsigned int frame:20; /* FRAME ADDRESS */
//} page_t __packed;

//typedef struct page_table {
//	page_t pages[PAG_PAGES_PER_TABLE]; /* 4MB per table and 4KB per page */
//} page_table_t;

//typedef struct page_directory_table {
//	unsigned int present:1; /* 0: table not present 1: table present */
//	unsigned int rw:1; /* 0: table read only 1: table writable */
//	unsigned int user:1; /* 0: kernel mode 1: user mode */
//	unsigned int writethrough:1; /* 0: write back caching enabled 1: ... disabled */
//	unsigned int cache_en:1; /* 0: table won't be cached 1: table will be cached */
//	unsigned int accessed:1; /* 0: not accessed 1: accessed */
//	unsigned int unused:1;
//	unsigned int page_size:1; /* 0: 4kb page sizes 1: 4mb page sizes */
//	unsigned int unused2:1;
//	unsigned int available:3; /* available for use */
//	unsigned int table_address:20; /* address of the page directory table */
//} page_directory_table_t __packed;

//typedef struct page_directory {
//	uintptr_t physical_tables[PAG_TABLES_PER_DIR] __align(4096); /* Page tables' table (a table with flags for each table) */
//	page_table_t * tables[PAG_TABLES_PER_DIR]; /* Array of page tables, covers entire memory space */
//	int32_t ref_count;
//} page_directory_t;

//extern page_directory_t * kernel_directory;
//extern page_directory_t * current_directory;

//namespace Kernel {
//	namespace Memory {
//		namespace Man {
			//extern page_directory_t * clone_directory(page_directory_t * src);
			//extern page_table_t * clone_table(page_table_t * src, uintptr_t * physAddr);
			//extern void release_directory(page_directory_t * dir);
			//extern "C" { void copy_page_physical(uint32_t, uint32_t); }
//		}
//	}
//}
