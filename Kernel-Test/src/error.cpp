#include "system.h"

namespace Kernel {
	extern Console term; /* Used only for debugging to the screen */
	
	namespace Error {

		/* Many ways to panic: */

		void panic(const char * msg, const int line, const char * file, int intno) {
			char buff[256];
			sprintf(buff, "!! KERNEL PANIC !!\n\n - %s (At line %d @ %s - int: %d)", msg, line, file, intno);

			term.fill(VIDRed);
			term.puts(buff, COLOR_BAD);

			KERNEL_FULL_STOP();
		}

		void panic(const char * msg, int intno) {
			char buff[256];
			sprintf(buff, "!! KERNEL PANIC !!\n\n - %s (int: %d)", msg, intno);

			term.fill(VIDRed);
			term.puts(buff, COLOR_BAD);

			KERNEL_FULL_STOP();
		}

		void panic(const char * msg) {
			term.fill(VIDRed);
			term.puts("!! KERNEL PANIC !!\n\n - ", COLOR_BAD);
			term.puts(msg, COLOR_BAD);

			KERNEL_FULL_STOP();
		}

		void panic(void) {
			term.fill(VIDRed);
			term.puts("!! KERNEL PANIC !!", COLOR_BAD);
			KERNEL_FULL_STOP();
		}

		void infinite_idle(const char * msg) {
			term.fill(VIDBlue);
			term.puts(msg, COLOR_INFO);
			KERNEL_FULL_STOP();
		}
	}
}