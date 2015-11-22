#pragma once

namespace Log {
	#define DEBUG(msg) if(Log::logging) Kernel::term.puts((char*)msg, COLOR_DEFAULT);
	#define DEBUGF(msg, ...) if(Log::logging)  Kernel::term.printf(msg, ##__VA_ARGS__);
	#define DEBUGCF(color, msg, ...) if(Log::logging)  Kernel::term.printf(color, msg, ##__VA_ARGS__);
	#define DEBUGC(msg, color) if(Log::logging)  Kernel::term.puts((char*)msg, color);
	#define DEBUGOK() DEBUGC(" OK \n", COLOR_GOOD);
	#define DEBUGVALID() DEBUGC(" VALID \n", COLOR_GOOD);
	#define DEBUGBAD() DEBUGC(" BAD \n", COLOR_BAD);
	#define DEBUGNO() DEBUGC(" NO \n", COLOR_BAD);

	#define LOG_ENABLED 1
	static char logging = LOG_ENABLED;
}