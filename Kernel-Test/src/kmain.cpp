#include "console.h"

typedef unsigned int uint16_t;
typedef unsigned char uint8_t;

#define ENTRY_PER_PAGETABLE 1024 // There is always 1024 pages (4KiB/Page)
#define PAGE_TABLE_COUNT 1024 

/* # of entries/page table * total # of page tables
actual size = 4194304 bytes = 4MiB, represents 4GiB in physical memory (size of unsigned int = 4 bytes)
ie. each 4 byte entry represent 4 KiB in physical memory */
extern uint16_t Page_Table1[ENTRY_PER_PAGETABLE * PAGE_TABLE_COUNT]; 

extern uint16_t Page_Directory[PAGE_TABLE_COUNT]; // # of pages tables * # of directory (4096 bytes = 4 KiB)

#define KERNEL_VIRTUAL_BASE 0xC0000000 // Constant declaring base of Higher-half kernel (from Kernel.asm)		 
#define KERNEL_PAGE_TABLE (KERNEL_VIRTUAL_BASE >> 22) // Constant declaring Page Table index in virtual memory (from Kernel.asm)

namespace Memory {
	namespace Kernel_Memory {
		namespace Paging {
			static void paging_init() {
				uint16_t PhysicalAddressAndFlags = 0b111; // Setting Page Table flags (Present: ON, Read/Write: ON, User/Supervisor: ON)
				uint16_t NoOfPageTables = 4; // 4 is arbitrary to cover 16MiB
				uint16_t StartPageTableEntryIndex = 0;
				uint16_t SizeOfPageTables = NoOfPageTables * ENTRY_PER_PAGETABLE;

				uint16_t * Page_Table1_Physical = (uint16_t*)((uint16_t)Page_Table1 - KERNEL_VIRTUAL_BASE);
				uint16_t* Page_Directory_Physical = (uint16_t*)((uint16_t)Page_Directory - KERNEL_VIRTUAL_BASE);

				/* Setting up identity mapping */
				for(uint16_t i = 0; i < (SizeOfPageTables + StartPageTableEntryIndex); i++) 
				{
					Page_Table1_Physical[i] = PhysicalAddressAndFlags;
					PhysicalAddressAndFlags += 4096;
				}

				PhysicalAddressAndFlags = 0b111;
				StartPageTableEntryIndex = (KERNEL_PAGE_TABLE * 1024);
					
				for(uint16_t i = (KERNEL_PAGE_TABLE*1024); i < (SizeOfPageTables + StartPageTableEntryIndex); i++) 
				{
					Page_Table1_Physical[i] = PhysicalAddressAndFlags;
					PhysicalAddressAndFlags += 4096;
				}

				PhysicalAddressAndFlags = (unsigned int)&Page_Table1_Physical[0];
				PhysicalAddressAndFlags |= 0b111; // Setting Page Table flags (Present: ON, Read/Write: ON, User/Supervisor: ON)

				uint16_t EntriesOfPageDirectory = 1024;
				for(uint16_t i = 0;i<EntriesOfPageDirectory;i++)
				{
					Page_Directory_Physical[i] = PhysicalAddressAndFlags; // Move to next entry in Page Directory (4 bytes down)
					PhysicalAddressAndFlags += 4096; // Update physical address to which to set the next Page Directory entry to (4 KiB down)
				}

				/* Set virtual addressing via control register CR3 
				 high 20 bits is Page directory Base Register i.e physical address of first page directory entry */
				__asm__(
					"lea ECX, [Page_Directory - 0xC0000000]\n" // 0xC0000000 = KERNEL_VIRTUAL_BASE
					"mov CR3, ECX\n"
				);
				
				// Switch on paging via control register CR0
				__asm__(
					"mov ECX, CR0\n"
					"or ECX, 0x80000000\n" // Set PG bit in CR0 to enable paging.
					"mov CR0, ECX\n"
				);
				
				// At only this point we are in physical higher-half mode
			}
		}

		void Memory_Initialize() {
			/* Initializes all memory related stuff */
			Memory::Kernel_Memory::Paging::paging_init(); /* First thing's first, set up Paging */

		}
	}
}

namespace Kernel {
	#define DEBUG(msg) term.puts((char*)msg, DEFAULT_COLOR);
	#define DEBUGC(msg, color) term.puts((char*)msg, color);

	void kinit() {
		Memory::Kernel_Memory::Memory_Initialize();
		Console term;

		DEBUG(">> Initializing Kernel <<\n");
		DEBUG("Setting up paging - "); DEBUGC("DONE\n", COLOR(VIDGreen, VIDYellow));
	}
}

//char buff[] = "AAAA";
//int x;

void kmain() {
	Kernel::kinit();
	
	for(;;);
}
