#pragma once

namespace Log {

enum LOG_CHANNELS {
	LOG_DISABLED, LOG_VGA, LOG_SERIAL, LOG_VGA_SERIAL
};

#ifndef MODULE

	#define kputs(msg) do { \
		if(Log::logging == Log::LOG_VGA) \
			Kernel::term.puts((char*)msg, COLOR_DEFAULT); \
		else if(Log::logging == Log::LOG_SERIAL) \
			Kernel::serial.puts((char*)msg); \
		else if(Log::logging == Log::LOG_VGA_SERIAL) { \
			Kernel::term.puts((char*)msg, COLOR_DEFAULT); \
			Kernel::serial.puts((char*)msg); \
		} } while(0);

	#define kprintf(msg, ...) do { \
		if(Log::logging == Log::LOG_VGA) \
			Kernel::term.printf((char*)msg, ##__VA_ARGS__); \
		else if(Log::logging == Log::LOG_SERIAL) \
			Kernel::serial.printf((char*)msg, ##__VA_ARGS__); \
		else if(Log::logging == Log::LOG_VGA_SERIAL) { \
			Kernel::term.printf(msg, ##__VA_ARGS__); \
			Kernel::serial.printf((char*)msg, ##__VA_ARGS__); \
		} } while(0);

	#define kprintfc(color, msg, ...) do { \
		if(Log::logging == Log::LOG_VGA)  \
			Kernel::term.printf(color, msg, ##__VA_ARGS__); \
		else if(Log::logging == Log::LOG_SERIAL) \
			Kernel::serial.printf((char*)msg, ##__VA_ARGS__); \
		else if(Log::logging == Log::LOG_VGA_SERIAL) { \
			Kernel::term.printf(color, msg, ##__VA_ARGS__); \
			Kernel::serial.printf((char*)msg, ##__VA_ARGS__); \
		} } while(0);

	#define kputsc(msg, color) do { \
		if(Log::logging == Log::LOG_VGA) \
			Kernel::term.puts((char*)msg, color); \
		else if(Log::logging == Log::LOG_SERIAL) \
			Kernel::serial.puts((char*)msg); \
		else if(Log::logging == Log::LOG_VGA_SERIAL) { \
			Kernel::term.puts((char*)msg, color); \
			Kernel::serial.puts((char*)msg); \
		} } while(0);

	#define DEBUGOK() kputsc(" OK \n", COLOR_GOOD);
	#define DEBUGVALID() kputsc(" VALID \n", COLOR_GOOD);
	#define DEBUGBAD() kputsc(" BAD \n", COLOR_BAD);
	#define DEBUGNO() kputsc(" NO \n", COLOR_BAD);
#endif

#define debug_buffer(buff, size) \
	do { kprintf("\n @ 0x%x (%d) >>\n", buff, size); \
		 for(int i = 0; i < size; i++) kprintf("%d ", ((uint8_t*)(buff))[i]); \
		 kprintf("\n <<");\
	} while(0);

extern char logging;
inline void redirect_log(char logchannel) {
	logging = logchannel & 0x3;
}
}
