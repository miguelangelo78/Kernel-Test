/*
 ..\toolchain\Windows\Tools\Cross\i686-elf\bin\i686-elf-g++ foo.c -nostdlib -o runme.o --entry main
*/

static int fork(void) {
	int a;
	int num = 8;
	__asm__ __volatile__("int $0x7F" : "=a" (a) : "0" (num)); \
	return a;
}

enum VIDColor {
	VIDBlack,
	VIDBlue,
	VIDGreen,
	VIDCyan,
	VIDRed,
	VIDMagenta,
	VIDBrown,
	VIDLightGray,
	VIDDarkGray,
	VIDLightBlue,
	VIDLightGreen,
	VIDLightCyan,
	VIDLightRed,
	VIDLightMagenta,
	VIDYellow,
	VIDWhite
};

#define COLOR(bg, fg) ((bg<<4) | fg)
#define COLOR_DEFAULT COLOR(VIDBlack, VIDLightGray)

int main(int argc, char ** argv) {
	if(!fork()) {
		if(!fork()) {
			for(;;) {
				for(int i=0;i<10;i++) {
					char * c = (char*)0xb8000;
					c[(0+1*80)*2] = i + '0';
					c[(0+1*80 + 1)*2] = COLOR_DEFAULT;
				}
			}
		} else {
			for(;;) {
				for(int i=0;i<10;i++) {
					char * c = (char*)0xb8000;
					c[(0+2*80)*2] = i + '0';
					c[(0+2*80 + 1)*2] = COLOR_DEFAULT;
				}
			}
		}
	} else {
		for(;;) {
			for(int i=0;i<10;i++) {
				char * c = (char*)0xb8000;
				c[(0+3*80)*2] = i + '0';
				c[(0+3*80 + 1)*2] = COLOR_DEFAULT;
			}
		}
	}
	for(;;);
	return 0;
}