
//https://sourceware.org/ml/newlib/2013/msg00234.html
#ifndef	__DECONST
#define	__DECONST(type, var)	((type)(uintptr_t)(const void *)(var))
#endif
