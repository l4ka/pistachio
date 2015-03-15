//http://en.wikibooks.org/wiki/C_Programming/Strings#The_.3Cstring.h.3E_Standard_Header
//http://www.cplusplus.com/reference/cstring/

#ifndef __STRING_H__
#define __STRING_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>

#ifdef __cplusplus
}
#endif


// BAD IDEA #include <lib/io/lib.h>

//Should be exposed in stddef, says http://stackoverflow.com/questions/924664/why-is-null-undeclared
#ifndef NULL
	#define NULL ((void *)0)
#endif

#include "stdarg.h"

#ifdef __cplusplus
extern "C" {
#endif

//GNU stuff
char *
strchrnul(const char *p, int ch);

//FreeBSD extensions (NEW!)
char *
stpncpy(char * __restrict dst, const char * __restrict src, size_t n);

char *
stpcpy(char * __restrict to, const char * __restrict from);


char *
strncat(char *dst, const char *src, size_t n);


size_t
strcspn(const char *s1, const char *s2);


int
strncmp(const char *s1, const char *s2, size_t n);


char *
strncpy(char *dst, const char *src, size_t n);


//The triad...
void *
memcpy(void *dst0, const void *src0, size_t length);

void *
memmove(void *dst0, const void *src0, size_t length);

void
bcopy(const void *src0, void *dst0, size_t length);



//Try exporting this for cstring...
void *
memcpy(void *s1, const void *s2, size_t n);

char *
strerror(int num);

void *
memccpy(void *t, const void *f, int c, size_t n);


//Probably breaks stuff...
int strcmp(const char *aString1, const char *aString2); 

char *
strpbrk(const char *s1, const char *s2);

void *
memchr(const void *s, int c, size_t n);


//BSD rindex(), like strrchr()
//https://github.com/toddfries/OpenBSD-lib-patches/blob/master/libc/string/rindex.c
char * rindex(const char *p, int ch);

char * index(const char *p, int ch);

int ffs(int mask);
int fls(int mask);
int ffsl(long mask);

void *
memrchr(const void *s, int c, size_t n);


char * strsignal(int sig);

char * strsep(char **stringp, const char *delim);

size_t strxfrm(char *dst, const char *src, size_t n);

int strncasecmp(const char *s1, const char *s2, size_t n);

int strcasecmp(const char *s1, const char *s2);

int obsd_strcmp(const char *s1, const char *s2);

char *
strstr(const char *s, const char *find);

void *
memmem(const void *l, size_t l_len, const void *s, size_t s_len);


char * strrchr(const char *p, int ch);

int
strcoll(const char *s1, const char *s2);

char *
strtok_r(char *s, const char *delim, char **last);
void
explicit_bzero(void *p, size_t n);

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

int
timingsafe_bcmp(const void *b1, const void *b2, size_t n);


#ifdef __cplusplus
}
#endif


#endif /* !__STRING_H__ */

