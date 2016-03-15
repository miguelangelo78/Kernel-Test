#pragma once

namespace Log {
	#define kputs(msg) if(Log::logging) Kernel::term.puts((char*)msg, COLOR_DEFAULT);
	#define kprintf(msg, ...) if(Log::logging)  Kernel::term.printf(msg, ##__VA_ARGS__);
	#define kprintfc(color, msg, ...) if(Log::logging)  Kernel::term.printf(color, msg, ##__VA_ARGS__);
	#define kputsc(msg, color) if(Log::logging)  Kernel::term.puts((char*)msg, color);
	#define DEBUGOK() kputsc(" OK \n", COLOR_GOOD);
	#define DEBUGVALID() kputsc(" VALID \n", COLOR_GOOD);
	#define DEBUGBAD() kputsc(" BAD \n", COLOR_BAD);
	#define DEBUGNO() kputsc(" NO \n", COLOR_BAD);

	#define LOG_ENABLED 1
	static char logging = LOG_ENABLED;
}
