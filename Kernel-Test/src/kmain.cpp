#include "console.h"

void kmain() {
	Console terminal;
	terminal.clear();
	for(int i=0;i<100;i++)
		terminal.writestr("Hi Bro  ", COLOR(0x0, 0xF));
	
	for(;;);
}