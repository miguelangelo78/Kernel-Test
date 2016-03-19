#pragma once

namespace Log {
	#define kputs(msg) \
		if(Log::logging == 1) \
			Kernel::term.puts((char*)msg, COLOR_DEFAULT); \
		else if(Log::logging == 2) \
			Kernel::serial.puts((char*)msg);

	#define kprintf(msg, ...) \
		if(Log::logging == 1) \
			Kernel::term.printf(msg, ##__VA_ARGS__); \
		else if(Log::logging == 2) \
			Kernel::serial.printf((char*)msg, ##__VA_ARGS__);

	#define kprintfc(color, msg, ...) if(Log::logging == 1)  Kernel::term.printf(color, msg, ##__VA_ARGS__); else if(Log::logging == 2) Kernel::serial.printf((char*)msg, ##__VA_ARGS__);
	#define kputsc(msg, color) if(Log::logging == 1)  Kernel::term.puts((char*)msg, color); else if(Log::logging == 2) Kernel::serial.puts((char*)msg);
	#define DEBUGOK() kputsc(" OK \n", COLOR_GOOD);
	#define DEBUGVALID() kputsc(" VALID \n", COLOR_GOOD);
	#define DEBUGBAD() kputsc(" BAD \n", COLOR_BAD);
	#define DEBUGNO() kputsc(" NO \n", COLOR_BAD);

	#define LOG_DISABLED 0
	#define LOG_VGA 1
	#define LOG_SERIAL 2

	extern char logging;
	inline void redirect_log(char logchannel) {
		logging = logchannel & 0x3;
	}
}
