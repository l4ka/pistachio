#ifndef __STDLIB_H__
#define __STDLIB_H__

#ifdef __cplusplus
extern "C" {
#endif

//Define of size_t
#include <liballoc.h>

#include <sys/cdefs.h>
#include <sys/types.h>

#include <gdtoa.h>

//POSIX style environment variables
int putenv(char *str);
int setenv(const char *name, const char *value, int rewrite);
int unsetenv(const char *name);

//Temporary definition from http://riot-os.org/api/quad_8h_source.html
//typedef long long quad_t;

quad_t qabs(quad_t j);

void *
reallocarray(void *optr, size_t nmemb, size_t size);

// Functions for working with floats
// double strtod (CONST char *s00, char **se);

/* Pseudo-random sequence generation */
int rand_r(u_int *seed);
int rand(void);
void srand(u_int seed);

char * l64a(long value);
int l64a_r(long value, char *buffer, int buflen);
long a64l(const char *s);


int abs(int j);

int
heapsort(void *vbase, size_t nmemb, size_t size,
    int (*compar)(const void *, const void *));


void *
bsearch(const void *key, const void *base0, size_t nmemb, size_t size,
	int (*compar)(const void *, const void *));



//Really?
size_t strlen(const char *str);

void * calloc(size_t num, size_t size);
void * memset(void *s1, int c, size_t n);
void * memcpy(void *s1, const void *s2, size_t n);

//Dummy implementation for WaterFront BASIC
int system(const char *command);

#ifdef __cplusplus
}
#endif


#endif /* !__STDLIB_H__ */

