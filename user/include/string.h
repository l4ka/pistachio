//http://en.wikibooks.org/wiki/C_Programming/Strings#The_.3Cstring.h.3E_Standard_Header
//http://www.cplusplus.com/reference/cstring/

#ifndef __STRING_H__
#define __STRING_H__

//#include <l4/types.h>

// BAD IDEA #include <lib/io/lib.h>

//Should be exposed in stddef, says http://stackoverflow.com/questions/924664/why-is-null-undeclared
#ifndef NULL
	#define NULL ((void *)0)
#endif

#include "stdarg.h"

#ifdef __cplusplus
extern "C" {
#endif

//BSD rindex(), like strrchr()
//https://github.com/toddfries/OpenBSD-lib-patches/blob/master/libc/string/rindex.c
char * rindex(const char *p, int ch);

#ifdef __cplusplus
}
#endif


#endif /* !__STRING_H__ */

