//http://en.wikibooks.org/wiki/C_Programming/Strings#The_.3Cstring.h.3E_Standard_Header
//http://www.cplusplus.com/reference/cstring/

#ifndef __STRING_H__
#define __STRING_H__

#include <sys/types.h>

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

char * index(const char *p, int ch);

int strncasecmp(const char *s1, const char *s2, size_t n);

int strcasecmp(const char *s1, const char *s2);

int obsd_strcmp(const char *s1, const char *s2);

char * strrchr(const char *p, int ch);

char *
strtok_r(char *s, const char *delim, char **last);

char *
strtok(char *s, const char *delim);

char * strdup(const char *str);

char *
strchr(const char *p, int ch);

char *
strcat(char *s, const char *append);

size_t
strlcat(char *dst, const char *src, size_t siz);

int
memcmp(const void *s1, const void *s2, size_t n);

char *
strcpy(char *to, const char *from);

size_t strspn(const char *s1, const char *s2);

//Android/Bionic version, not locale-aware...
size_t
strxfrm(char *s1, const char *s2, size_t n);


#ifdef __cplusplus
}
#endif


#endif /* !__STRING_H__ */

