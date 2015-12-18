#include <system.h>
#include <memory.h>

namespace Kernel {
namespace Memory {
namespace Man {
	
	#define MEM_FRAMES_PER_BYTE 8
	#define MEM_FRAME_SIZE 0x1000
	#define MEM_PAGE_SIZE MEM_FRAME_SIZE
	#define MEM_FRAME_ALIGN MEM_FRAME_SIZE

	#define ALIGN(frame) (frame) * MEM_FRAME_ALIGN

	static spin_lock_t frame_alloc_lock = { 0 };

	static uint32_t mem_size = 0;
	static uint32_t nframes = 0;
	static uint32_t * frames; /* Memory map bit array */

	uintptr_t frame_ptr = (uintptr_t)&end; /* Placement pointer to allocate raw frames */
	uintptr_t heap_top = 0; /* Top of the heap's address. Only used after paging is enabled */
	uintptr_t kernel_heap_alloc_point = KERNEL_HEAP_INIT;

	page_directory_t * kernel_directory;
	page_directory * current_directory;

	void page_fault(struct regs *r);
	void switch_page_directory(page_directory_t * dir);

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
			frame_ptr += MEM_FRAME_SIZE;
		}

		if(phys)
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

	inline uint32_t get_memsize(void) {
		return mem_size;
	}

	inline void set_frame(uintptr_t frame_addr) {
		if(frame_addr < get_memsize() * 1024) {
			frame_addr /= MEM_FRAME_ALIGN;
			BIT_SET(frames[INDEX_FROM_BIT_SZ32(frame_addr)], OFFSET_FROM_BIT_SZ32(frame_addr));
		}
	}

	inline void clear_frame(uintptr_t frame_addr) {
		frame_addr /= MEM_FRAME_ALIGN;
		BIT_CLEAR(frames[INDEX_FROM_BIT_SZ32(frame_addr)], OFFSET_FROM_BIT_SZ32(frame_addr));
	}

	inline uint32_t test_frame(uintptr_t frame_addr) {
		frame_addr /= MEM_FRAME_ALIGN;
		return IS_BIT_SET(frames[INDEX_FROM_BIT_SZ32(frame_addr)], OFFSET_FROM_BIT_SZ32(frame_addr));
	}

	uint32_t first_frame(void) {
		for (uint32_t i = 0; i < INDEX_FROM_BIT_SZ32(nframes); i++) {
			if (frames[i] != 0xFFFFFFFF) {
				for (uint32_t j = 0; j < 32; j++)
					if(!IS_BIT_SET(frames[i], j))
						return i * 32 + j;
			}
		}
		Kernel::Error::panic("Couldn't find a new frame. Out of memory!");
		return -1;
	}

	uint32_t first_n_frames(int frame_count) {
		for (uint32_t i = 0; i < nframes * MEM_FRAME_SIZE; i += MEM_FRAME_SIZE) {
			int bad = 0;
			for (uint32_t j = 0; j < frame_count; j++)
				if(test_frame(i + MEM_FRAME_SIZE * j))
					bad = j + 1;
			if(!bad)
				return i / MEM_FRAME_SIZE;
		}
		return (uint32_t)-1;
	}

	uintptr_t memory_use(void) {
		uintptr_t ret = 0;
		uint32_t i, j;
		for (i = 0; i < INDEX_FROM_BIT_SZ32(nframes); i++)
			for(j = 0; j < 32; j++)
				if(IS_BIT_SET(frames[i], j))
					ret++;
		return ret * 4;
	}

	void free_frame(page_t * page) {
		uint32_t frame;
		if (!(frame = page->frame)) {
			ASSERT(0, "Couldn't free frame with address 0");
			return;
		} else {
			clear_frame(ALIGN(frame));
			page->frame = 0;
		}
	}

	void alloc_frame(page_t * page, int is_kernel, int is_writeable) {
		if (page->frame) { /* Already allocated */
			page->present = 1;
			page->rw = is_writeable;
			page->user = !is_kernel;
		} else {
			spin_lock(frame_alloc_lock);
			uint32_t index = first_frame();
			ASSERT(index != (uint32_t)-1, "Out of frames");
			set_frame(ALIGN(index));
			page->frame = index;
			spin_unlock(frame_alloc_lock);
			page->present = 1;
			page->rw = is_writeable;
			page->user = !is_kernel;
		}
	}

	void dma_frame(page_t * page, int is_kernel, int is_writeable, uintptr_t physical_address) {
		/* Set physical address to a certain page */
		page->present = 1;
		page->rw = is_writeable;
		page->user = !is_kernel;
		page->frame = physical_address / MEM_PAGE_SIZE;
		set_frame(physical_address);
	}

	page_t * get_page(uintptr_t address, int make_table, page_directory_t * dir) {
		address /= MEM_PAGE_SIZE;
		uint32_t table_index = INDEX_FROM_BIT(address, 1024);
		if (dir->tables[table_index]) {
			return &dir->tables[table_index]->pages[OFFSET_FROM_BIT(address, PAG_TABLES_PER_DIR)];
		}
		else if (make_table) {
			uint32_t tmp;
			dir->tables[table_index] = (page_table_t*)kvmalloc_p(sizeof(page_table_t), (uintptr_t *)(&tmp));
			memset(dir->tables[table_index], 0, sizeof(page_table_t));
			dir->physical_tables[table_index].table_address = tmp >> 12;
			dir->physical_tables[table_index].present = 1;
			dir->physical_tables[table_index].rw = 1;
			dir->physical_tables[table_index].user = 1;
			return &dir->tables[table_index]->pages[OFFSET_FROM_BIT(address, PAG_TABLES_PER_DIR)];
		}
		else {
			return 0;
		}
	}

	inline page_t * create_table(uintptr_t address, page_directory_t * dir) {
		get_page(address, 1, dir);
	}

	uintptr_t map_to_physical(uintptr_t virtual_addr) {
		uintptr_t remaining = OFFSET_FROM_BIT(virtual_addr, MEM_PAGE_SIZE);
		uintptr_t frame = INDEX_FROM_BIT(virtual_addr, MEM_PAGE_SIZE);
		uintptr_t table = INDEX_FROM_BIT(frame, PAG_TABLES_PER_DIR);
		uintptr_t subframe = OFFSET_FROM_BIT(frame, PAG_TABLES_PER_DIR);
	
		if (current_directory->tables[table]) {
			page_t * page = &current_directory->tables[table]->pages[subframe];
			return ALIGN(page->frame) + remaining;
		}
		else {
			return 0;
		}
	}

	uintptr_t map_to_virtual(uintptr_t physical_addr) {
		/* TODO */
	}

	/*
	* Clone a page directory and its contents.
	* (If you do not intend to clone the contents, do it yourself!)
	*
	* @param  src Pointer to source directory to clone from.
	* @return A pointer to a new directory.
	*/
	page_directory_t * clone_directory(page_directory_t * src) {
		/* Allocate a new page directory */
		uintptr_t phys;
		page_directory_t * dir = (page_directory_t *)kvmalloc_p(sizeof(page_directory_t), &phys);
		memset(dir, 0, sizeof(page_directory_t));
		dir->ref_count = 1;

		/* And store it... */
		dir->physical_address = phys;
		uint32_t i;
		for (i = 0; i < 1024; ++i) {
			/* Copy each table */
			if (!src->tables[i] || (uintptr_t)src->tables[i] == (uintptr_t)0xFFFFFFFF) {
				continue;
			}
			if (kernel_directory->tables[i] == src->tables[i]) {
				/* Kernel tables are simply linked together */
				dir->tables[i] = src->tables[i];
				dir->physical_tables[i] = src->physical_tables[i];
			}
			else {
				if (i * 0x1000 * 1024 < SHM_START) {
					/* User tables must be cloned */
					uintptr_t phys;
					dir->tables[i] = clone_table(src->tables[i], &phys);
					dir->physical_tables[i].table_address = phys;
					dir->physical_tables[i].present = 1;
					dir->physical_tables[i].rw = 1;
					dir->physical_tables[i].user = 1;
				}
			}
		}
		return dir;
	}

	/*
	* Clone a page table
	*
	* @param src      Pointer to a page table to clone.
	* @param physAddr [out] Pointer to the physical address of the new page table
	* @return         A pointer to a new page table.
	*/
	page_table_t * clone_table(page_table_t * src, uintptr_t * physAddr) {
		/* Allocate a new page table */
		page_table_t * table = (page_table_t *)kvmalloc_p(sizeof(page_table_t), physAddr);
		memset(table, 0, sizeof(page_table_t));
		for (uint32_t i = 0; i < 1024; ++i) {
			/* For each frame in the table... */
			if (!src->pages[i].frame)
				continue;
			/* Allocate a new frame */
			alloc_frame(&table->pages[i], 0, 0);
			/* Set the correct access bit */
			if (src->pages[i].present)	table->pages[i].present = 1;
			if (src->pages[i].rw)		table->pages[i].rw = 1;
			if (src->pages[i].user)		table->pages[i].user = 1;
			if (src->pages[i].accessed)	table->pages[i].accessed = 1;
			if (src->pages[i].dirty)	table->pages[i].dirty = 1;
			/* Copy the contents of the page from the old table to the new one */
			copy_page_physical(src->pages[i].frame * 0x1000, table->pages[i].frame * 0x1000);
		}
		return table;
	}

	void release_directory(page_directory_t * dir) {
		/* TODO */
	}

	void paging_install(uint32_t memsize) {
		mem_size = memsize;
		
		/* Allocate frame bitmap: */
		nframes = (mem_size * 1024) / MEM_FRAME_SIZE;
		uint32_t frame_bitmap_count_alloc = INDEX_FROM_BIT_SZ32(nframes * 8);
		frames = (uint32_t*)kmalloc(frame_bitmap_count_alloc);
		memset(frames, 0, frame_bitmap_count_alloc);
	
		/* Initialize kernel page directory: */
		kernel_directory = (page_directory_t*)kvmalloc(sizeof(page_directory_t));
		memset(kernel_directory, 0, sizeof(page_directory_t));

		/* Parse memory map: */
		if (IS_BIT_SET(mboot_ptr->flags, 6)) {
			mboot_memmap_t * mmap = (mboot_memmap_t*)mboot_ptr->mmap_addr;
			while ((uintptr_t)mmap < mboot_ptr->mmap_addr + mboot_ptr->mmap_length) {
				if (mmap->type == 2) {
					for(unsigned long long int i = 0; i < mmap->length; i += MEM_FRAME_SIZE) {
						if(mmap->base_addr + i > 0xFFFFFFFF) break; /* Reached the end */
						set_frame((uint64_t)((mmap->base_addr + i) & 0xFFFFF000)); /* Set frame as present for this memory segment */
					}
				}
				mmap = (mboot_memmap_t*) ((uintptr_t)mmap + mmap->size + sizeof(uintptr_t));
			}
		}

		/* Map memory (identity) on kernel directory: */
		create_table(0, kernel_directory)->present = 0;
		set_frame(0);

		for(uintptr_t i = 0x1000; i < 0x80000; i += MEM_PAGE_SIZE)
			dma_frame(create_table(i, kernel_directory), 1, 0, i);
		for (uintptr_t i = 0x80000; i < 0x100000; i += MEM_PAGE_SIZE)
			dma_frame(create_table(i, kernel_directory), 1, 0, i);
		for (uintptr_t i = 0x100000; i < frame_ptr + 0x3000; i += MEM_PAGE_SIZE)
			dma_frame(create_table(i, kernel_directory), 1, 0, i);
		/* VGA Text mode (in user mode): */
		for (uintptr_t i = 0xb8000; i < 0xc0000; i += MEM_PAGE_SIZE)
			dma_frame(get_page(i, 0, kernel_directory), 0, 1, i);
		
		/* Install page fault handler: */
		CPU::ISR::isr_install_handler(CPU::IDT::IDT_IVT::ISR_PAGEFAULT, (CPU::ISR::isr_handler_t)page_fault);

		/* MMU needs this to enable paging: */
		kernel_directory->physical_address = (uintptr_t)kernel_directory->physical_tables;

		uintptr_t tmp_heap_start = KERNEL_HEAP_INIT;
		if (tmp_heap_start <= frame_ptr + 0x3000) {
			tmp_heap_start = frame_ptr + 0x100000;
			kernel_heap_alloc_point = tmp_heap_start;
		}

		/* Kernel Heap Space: */
		for(uintptr_t i = frame_ptr + 0x3000; i < tmp_heap_start; i += MEM_PAGE_SIZE)
			alloc_frame(create_table(i, kernel_directory), 1, 0);
		/* And preallocate the page entries for all the rest of the kernel heap as well */
		for (uintptr_t i = tmp_heap_start; i < KERNEL_HEAP_END; i += MEM_PAGE_SIZE)
			create_table(i, kernel_directory);
		
		/* Switch page directory and enable paging: */
		current_directory = clone_directory(kernel_directory);
		switch_page_directory(kernel_directory);
	}

	void heap_install(void) {
		heap_top = (frame_ptr + MEM_PAGE_SIZE) & ~0xFFF; 
	}

	void switch_page_directory(page_directory_t * dir) {
		current_directory = dir;
		asm volatile (
			"mov %0, %%cr3\n"
			"mov %%cr0, %%eax\n"
			"orl $0x80000000, %%eax\n"
			"mov %%eax, %%cr0\n"
			:: "r"(dir->physical_address)
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

	void page_fault(struct regs *r) {
		Kernel::Error::panic("Page fault");
	}
}
}
}