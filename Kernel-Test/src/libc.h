#pragma once
#include "va_list.h"
#include "stdint.h"
#include "limits.h"

#define ALIGN (sizeof(size_t))
#define ONES ((size_t)-1/UCHAR_MAX)
#define HIGHS (ONES * (UCHAR_MAX/2+1))
#define HASZERO(X) (((X)-ONES) & ~(X) & HIGHS)

int sprintf(char * buf, const char *fmt, ...);

void * memset(void * dest, int c, size_t n);
int strcmp(const char * l, const char * r);
size_t strlen(const char * s);
