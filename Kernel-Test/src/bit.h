#pragma once

#define NTH_BIT(nth) (1 << nth)
#define IS_BIT_SET(flag, nth) flag & NTH_BIT(nth)
#define FETCH_BIT(val, nth) (IS_BIT_SET((val), (nth)) >> (nth)))

#define BIT_SET(p,m) ((p) |= NTH_BIT((m)))
#define BIT_CLEAR(p,m) ((p) &= ~NTH_BIT((m)))
#define BIT_WRITE(c,p,m) (c ? BIT_SET(p, m) : BIT_CLEAR(p,m))