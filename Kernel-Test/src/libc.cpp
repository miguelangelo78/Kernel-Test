#include "libc.h"

void * memset(void * dest, int c, size_t n) {
	uint8_t *ptr = (uint8_t*)dest;
	for (size_t i = 0; i < n; i++)
		ptr[i] = (uint8_t)c;
	return dest;
}

int strcmp(const char * l, const char * r) {
	for (; *l == *r && *l; l++, r++);
	return *(uint8_t *)l - *(uint8_t *)r;
}

size_t strlen(const char * s) {
	const char * a = s;
	const size_t * w;
	for (; (uintptr_t)s % ALIGN; s++) {
		if (!*s) {
			return s - a;
		}
	}
	for (w = (const size_t *)s; !HASZERO(*w); w++);
	for (s = (const char *)w; *s; s++);
	return s - a;
}
