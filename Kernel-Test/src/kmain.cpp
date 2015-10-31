#include "console.h"

void kmain() {
	Console term;
	term.clear();
	
	for(int i=0;i<300;i++)
	term.puts(" Test " , COLOR(0x1, 0xF));

	for(;;);
}