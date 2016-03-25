#include <system.h>

unsigned short * memsetw(unsigned short * dest, unsigned short val, int count) {
	int i = 0;
	for (; i < count; ++i) dest[i] = val;
	return dest;
}

void * memcpy(void * __restrict__ dest, const void * __restrict__ src, size_t n) {
	int8_t* dst8 = (int8_t*)dest;
	int8_t* src8 = (int8_t*)src;
	while (n--) *dst8++ = *src8++;
	return dest;
}

int memcmp(const void * vl, const void * vr, size_t n) {
	const uint8_t *l = (uint8_t*)vl;
	const uint8_t *r = (uint8_t*)vr;
	for (; n && *l == *r; n--, l++, r++);
	return n ? *l - *r : 0;
}

void * memchr(const void * src, int c, size_t n) {
	const uint8_t * s = (uint8_t*)src;
	c = (unsigned char)c;
	for (; ((uintptr_t)s & (ALIGN - 1)) && n && *s != c; s++, n--);
	if (n && *s != c) {
		const size_t * w;
		size_t k = ONES * c;
		for (w = (const size_t *)s; n >= sizeof(size_t) && !HASZERO(*w^k); w++, n -= sizeof(size_t));
		for (s = (const uint8_t *)w; n && *s != c; s++, n--);
	}
	return n ? (void *)s : 0;
}

void * memrchr(const void * m, int c, size_t n) {
	const uint8_t * s = (uint8_t*)m;
	c = (unsigned char)c;
	while (n--)
		if (s[n] == c)
			return (void*)(s + n);
	return 0;
}

void * memmove(void * dest, const void * src, size_t n) {
	char * d = (char*)dest;
	const char * s = (char*)src;

	if (d == s)
		return d;

	if (s + n <= d || d + n <= s)
		return memcpy(d, s, n);

	if (d<s) {
		if ((uintptr_t)s % sizeof(size_t) == (uintptr_t)d % sizeof(size_t)) {
			while ((uintptr_t)d % sizeof(size_t)) {
				if (!n--)
					return dest;
				*d++ = *s++;
			}
			for (; n >= sizeof(size_t); n -= sizeof(size_t), d += sizeof(size_t), s += sizeof(size_t))
				*(size_t *)d = *(size_t *)s;
		}
		for (; n; n--)
			*d++ = *s++;
	}
	else {
		if ((uintptr_t)s % sizeof(size_t) == (uintptr_t)d % sizeof(size_t)) {
			while ((uintptr_t)(d + n) % sizeof(size_t)) {
				if (!n--) 
					return dest;
				d[n] = s[n];
			}
			while (n >= sizeof(size_t)) {
				n -= sizeof(size_t);
				*(size_t *)(d + n) = *(size_t *)(s + n);
			}
		}
		while (n) {
			n--;
			d[n] = s[n];
		}
	}

	return dest;
}

size_t strlen(const char * s) {
	const char * a = s;
	const size_t * w;
	for (; (uintptr_t)s % ALIGN; s++) 
		if (!*s) 
			return s - a;
	for (w = (const size_t *)s; !HASZERO(*w); w++);
	for (s = (const char *)w; *s; s++);
	return s - a;
}

char * strdup(const char * s) {
	size_t l = strlen(s);
	return (char*)memcpy((void*)malloc((uintptr_t)(l + 1)), s, l + 1);
}

char * stpcpy(char * __restrict__ d, const char * __restrict__ s) {
	size_t * wd;
	const size_t * ws;

	if ((uintptr_t)s % ALIGN == (uintptr_t)d % ALIGN) {
		for (; (uintptr_t)s % ALIGN; s++, d++) 
			if (!(*d = *s)) 
				return d;
		wd = (size_t *)d;
		ws = (const size_t *)s;
		for (; !HASZERO(*ws); *wd++ = *ws++);
		d = (char *)wd;
		s = (const char *)ws;
	}

	for (; (*d = *s); s++, d++);

	return d;
}

char * strcpy(char * __restrict__ dest, const char * __restrict__ src) {
	stpcpy(dest, src);
	return dest;
}

size_t strspn(const char * s, const char * c) {
	const char * a = s;
	size_t byteset[32 / sizeof(size_t)] = { 0 };

	if (!c[0])
		return 0;
	
	if (!c[1]) {
		for (; *s == *c; s++);
		return s - a;
	}

	for (; *c && BITOP(byteset, *(unsigned char *)c, |=); c++);
	for (; *s && BITOP(byteset, *(unsigned char *)s, &); s++);

	return s - a;
}

char * strchrnul(const char * s, int c) {
	size_t * w;
	size_t k;

	c = (unsigned char)c;
	if (!c)
		return (char *)s + strlen(s);

	for (; (uintptr_t)s % ALIGN; s++) 
		if (!*s || *(unsigned char *)s == c) 
			return (char *)s;

	k = ONES * c;
	for (w = (size_t *)s; !HASZERO(*w) && !HASZERO(*w^k); w++);
	for (s = (char*)w; *s && *(unsigned char *)s != c; s++);
	return (char *)s;
}

char * strchr(const char * s, int c) {
	char *r = strchrnul(s, c);
	return *(unsigned char *)r == (unsigned char)c ? r : 0;
}

char * strrchr(const char * s, int c) {
	return (char*)memrchr(s, c, strlen(s) + 1);
}

int strcmp(const char * l, const char * r) {
	for (; *l == *r && *l; l++, r++);
	return *(uint8_t *)l - *(uint8_t *)r;
}

size_t strcspn(const char * s, const char * c) {
	const char *a = s;
	if (c[0] && c[1]) {
		size_t byteset[32 / sizeof(size_t)] = { 0 };
		for (; *c && BITOP(byteset, *(unsigned char *)c, |=); c++);
		for (; *s && !BITOP(byteset, *(unsigned char *)s, &); s++);
		return s - a;
	}
	return strchrnul(s, *c) - a;
}

char * strpbrk(const char * s, const char * b) {
	s += strcspn(s, b);
	return *s ? (char *)s : 0;
}

static char *strstr_2b(const unsigned char * h, const unsigned char * n) {
	uint16_t nw = n[0] << 8 | n[1];
	uint16_t hw = h[0] << 8 | h[1];
	for (h++; *h && hw != nw; hw = hw << 8 | *++h);
	return *h ? (char *)h - 1 : 0;
}

static char *strstr_3b(const unsigned char * h, const unsigned char * n) {
	uint32_t nw = n[0] << 24 | n[1] << 16 | n[2] << 8;
	uint32_t hw = h[0] << 24 | h[1] << 16 | h[2] << 8;
	for (h += 2; *h && hw != nw; hw = (hw | *++h) << 8);
	return *h ? (char *)h - 2 : 0;
}

static char *strstr_4b(const unsigned char * h, const unsigned char * n) {
	uint32_t nw = n[0] << 24 | n[1] << 16 | n[2] << 8 | n[3];
	uint32_t hw = h[0] << 24 | h[1] << 16 | h[2] << 8 | h[3];
	for (h += 3; *h && hw != nw; hw = hw << 8 | *++h);
	return *h ? (char *)h - 3 : 0;
}

static char *strstr_twoway(const unsigned char * h, const unsigned char * n) {
	size_t mem;
	size_t mem0;
	size_t byteset[32 / sizeof(size_t)] = { 0 };
	size_t shift[256];
	size_t l;

	/* Computing length of needle and fill shift table */
	for (l = 0; n[l] && h[l]; l++) {
		BITOP(byteset, n[l], |=);
		shift[n[l]] = l + 1;
	}

	if (n[l]) {
		return 0; /* hit the end of h */
	}

	/* Compute maximal suffix */
	size_t ip = -1;
	size_t jp = 0;
	size_t k = 1;
	size_t p = 1;
	while (jp + k<l) {
		if (n[ip + k] == n[jp + k]) {
			if (k == p) {
				jp += p;
				k = 1;
			}
			else {
				k++;
			}
		}
		else if (n[ip + k] > n[jp + k]) {
			jp += k;
			k = 1;
			p = jp - ip;
		}
		else {
			ip = jp++;
			k = p = 1;
		}
	}
	size_t ms = ip;
	size_t p0 = p;

	/* And with the opposite comparison */
	ip = -1;
	jp = 0;
	k = p = 1;
	while (jp + k<l) {
		if (n[ip + k] == n[jp + k]) {
			if (k == p) {
				jp += p;
				k = 1;
			}
			else {
				k++;
			}
		}
		else if (n[ip + k] < n[jp + k]) {
			jp += k;
			k = 1;
			p = jp - ip;
		}
		else {
			ip = jp++;
			k = p = 1;
		}
	}
	if (ip + 1 > ms + 1) {
		ms = ip;
	}
	else {
		p = p0;
	}

	/* Periodic needle? */
	if (memcmp(n, n + p, ms + 1)) {
		mem0 = 0;
		p = MAX(ms, l - ms - 1) + 1;
	}
	else {
		mem0 = l - p;
	}
	mem = 0;

	/* Initialize incremental end-of-haystack pointer */
	const unsigned char * z = h;

	/* Search loop */
	for (;;) {
		/* Update incremental end-of-haystack pointer */
		if ((size_t)(z - h) < l) {
			/* Fast estimate for MIN(l,63) */
			size_t grow = l | 63;
			const unsigned char *z2 = (unsigned char*)memchr(z, 0, grow);
			if (z2) {
				z = z2;
				if ((size_t)(z - h) < l) {
					return 0;
				}
			}
			else {
				z += grow;
			}
		}

		/* Check last byte first; advance by shift on mismatch */
		if (BITOP(byteset, h[l - 1], &)) {
			k = l - shift[h[l - 1]];
			if (k) {
				if (mem0 && mem && k < p) k = l - p;
				h += k;
				mem = 0;
				continue;
			}
		}
		else {
			h += l;
			mem = 0;
			continue;
		}

		/* Compare right half */
		for (k = MAX(ms + 1, mem); n[k] && n[k] == h[k]; k++);
		if (n[k]) {
			h += k - ms;
			mem = 0;
			continue;
		}
		/* Compare left half */
		for (k = ms + 1; k>mem && n[k - 1] == h[k - 1]; k--);
		if (k <= mem) {
			return (char *)h;
		}
		h += p;
		mem = mem0;
	}
}

char *strstr(const char * h, const char * n) {
	/* Return immediately on empty needle */
	if (!n[0])
		return (char *)h;

	/* Use faster algorithms for short needles */
	h = strchr(h, *n);
	if (!h || !n[1])
		return (char *)h;

	if (!h[1]) return 0;
	if (!n[2]) return strstr_2b((unsigned char *)h, (unsigned char *)n);
	if (!h[2]) return 0;
	if (!n[3]) return strstr_3b((unsigned char *)h, (unsigned char *)n);
	if (!h[3]) return 0;
	if (!n[4]) return strstr_4b((unsigned char *)h, (unsigned char *)n);

	/* Two-way on large needles */
	return strstr_twoway((unsigned char *)h, (unsigned char *)n);
}

static int isdigit(int ch) {
	return (unsigned int)ch - '0' < 10;
}

static int isspace(int ch) {
	return ch == ' ' || (unsigned int)ch - '\t' < 5;
}

int atoi(const char * s) {
	int n = 0;
	int neg = 0;
	while (isspace(*s))
		s++;

	switch (*s) {
	case '-':
		neg = 1;
		/* Fallthrough is intentional here */
	case '+':
		s++;
	}
	while (isdigit(*s))
		n = 10 * n - (*s++ - '0');
	/* The sign order may look incorrect here but this is correct as n is calculated
	* as a negative number to avoid overflow on INT_MAX.
	*/
	return neg ? n : -n;
}

size_t lfind(const char * str, const char accept) {
	return (size_t)strchr(str, accept);
}

size_t rfind(const char * str, const char accept) {
	return (size_t)strrchr(str, accept);
}

uint8_t startswith(const char * str, const char * accept) {
	return strstr(str, accept) == str;
}

/* Non-standard broken strtok_r */
char * strtok_r(char * str, const char * delim, char ** saveptr) {
	char * token;
	if (str == NULL) {
		str = *saveptr;
	}
	str += strspn(str, delim);
	if (*str == '\0') {
		*saveptr = str;
		return (char*)NULL;
	}
	token = str;
	str = strpbrk(token, delim);
	if (str == NULL) {
		*saveptr = (char *)lfind(token, '\0');
	}
	else {
		*str = '\0';
		*saveptr = str + 1;
	}
	return token;
}

uint32_t __attribute__((pure)) krand(void) {
	static uint32_t x = 123456789;
	static uint32_t y = 362436069;
	static uint32_t z = 521288629;
	static uint32_t w = 88675123;

	uint32_t t;
	t = x ^ (x << 11);
	x = y; y = z; z = w;
	return w = w ^ (w >> 19) ^ t ^ (t >> 8);
}

int tokenize(char * str, char * sep, char **buf) {
	char * pch_i;
	char * save_i;
	int    argc = 0;
	pch_i = strtok_r(str, sep, &save_i);
	if (!pch_i) { return 0; }
	while (pch_i != NULL) {
		buf[argc] = (char *)pch_i;
		++argc;
		pch_i = strtok_r((char*)NULL, sep, &save_i);
	}
	buf[argc] = (char*)NULL;
	return argc;
}
