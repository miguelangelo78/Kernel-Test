#include "system.h"

#define BSOD_COLOR 0xDE1B1B

namespace Kernel {
	extern Terminal term; /* Used only for debugging to the screen */
	
	namespace Error {

		char kernel_panic_msg[] = "!! KERNEL PANIC !!";
		char bsod_buff[256];

		void bsod_print(char * buff) {
			serial.puts(buff);
			if(gfx->vid_mode) {
				term.fill(BSOD_COLOR);
				term.puts(buff, 0xFFFFFF , BSOD_COLOR);
			} else {
				term.fill(VIDRed);
				term.puts(buff);
			}
		}

		/* Many ways to panic: */

		void panic(const char * msg, const int line, const char * file, int intno) {
			sprintf(bsod_buff, "%s\n\n - %s (At line %d @ %s - int: %d)", kernel_panic_msg, msg, line, file, intno);
			bsod_print(bsod_buff);
			KERNEL_FULL_STOP();
		}

		void panic(const char * msg, int intno) {
			sprintf(bsod_buff, "%s\n\n - %s (int: %d)", kernel_panic_msg, msg, intno);
			bsod_print(bsod_buff);
			KERNEL_FULL_STOP();
		}

		void panic(const char * msg) {
			sprintf(bsod_buff, "%s\n\n - %s", kernel_panic_msg, msg);
			bsod_print(bsod_buff);
			KERNEL_FULL_STOP();
		}

		void panic(void) {
			bsod_print(kernel_panic_msg);
			KERNEL_FULL_STOP();
		}

		void infinite_idle(const char * msg) {
			bsod_print((char*)msg);
			KERNEL_FULL_STOP(); /* IRQs don't work anymore */
		}
	}
}
