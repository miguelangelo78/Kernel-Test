#include <system.h>
#include <stdint.h>

namespace Kernel {
namespace Memory {
namespace Man {
	#define KERNEL_HEAP_INIT 0x00800000
	#define KERNEL_HEAP_END  0x20000000
	#define KERNEL_HEAP_SIZE KERNEL_HEAP_END - KERNEL_HEAP_INIT
	
	uintptr_t frame_ptr = (uintptr_t)&end; /* Placement pointer to allocate frames */
	uintptr_t heap_top = 0; /* Top of the heap's address. Only used after paging is enabled */

	void kmalloc_starts(uintptr_t start_addr) {
		frame_ptr = start_addr;
	}

	/* 'Real' kmalloc */
	uintptr_t kmalloc(size_t size, char align, uintptr_t * phys) {
		if (heap_top) {
			/* Use malloc */
		}
		/* else: use dumb kmalloc without paging */

		if (align) {

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
}
}
}