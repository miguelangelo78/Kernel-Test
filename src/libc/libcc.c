#include <stdint.h>

void * memset(void * dest, int c, size_t n) {
	uint8_t *ptr = (uint8_t*)dest;
	while(n--) *ptr++ = c;
	return dest;
}
