typedef unsigned char uint8_t;
typedef unsigned int int16_t;

void kmain() {
	uint8_t bg = 0b10;
	uint8_t fg = 0x00;
	uint8_t Colour = ((bg << 4) & 0xF0) | (fg & 0x0F);
	unsigned short* DisplayMemoryPtr = (unsigned short*)0xB8000;
	int16_t  DisplaySize = 2000;
	while (DisplaySize--)	
		DisplayMemoryPtr[DisplaySize] = (((unsigned short)Colour) << 8);

	for(;;);
}