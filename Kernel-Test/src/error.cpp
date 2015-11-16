#include "system.h"

namespace Kernel {
	extern Terminal term; /* Used only for debugging to the screen */
	
	namespace Error {

		char kernel_panic_msg[] = "!! KERNEL PANIC !!";
		
		/* Many ways to panic: */

		void panic(const char * msg, const int line, const char * file, int intno) {
			char buff[256];
			sprintf(buff, "%s\n\n - %s (At line %d @ %s - int: %d)", kernel_panic_msg, msg, line, file, intno);

			term.fill(VIDRed);
			term.puts(buff, COLOR_BAD);

			KERNEL_FULL_PAUSE(); /* IRQs still work */
		}

		void panic(const char * msg, int intno) {
			char buff[256];
			sprintf(buff, "%s\n\n - %s (int: %d)", kernel_panic_msg, msg, intno);

			term.fill(VIDRed);
			term.puts(buff, COLOR_BAD);

			KERNEL_FULL_PAUSE(); /* IRQs still work */
		}

		void panic(const char * msg) {
			term.fill(VIDRed);
			term.puts(kernel_panic_msg, COLOR_BAD);
			term.puts("\n\n - ", COLOR_BAD);
			term.puts(msg, COLOR_BAD);

			KERNEL_FULL_PAUSE(); /* IRQs still work */
		}

		void panic(void) {
			term.fill(VIDRed);
			term.puts(kernel_panic_msg, COLOR_BAD);
			KERNEL_FULL_PAUSE(); /* IRQs still work */
		}

		void infinite_idle(const char * msg) {
			term.fill(VIDBlue);
			term.puts(msg, COLOR_INFO);
			
			KERNEL_FULL_STOP(); /* IRQs don't work anymore */
		}
	}
}