#ifndef __STDLIB_H__
#define __STDLIB_H__

#ifdef __cplusplus
extern "C" {
#endif

//Define of size_t
#include <liballoc.h>

#include <sys/cdefs.h>
#include <sys/types.h>

#include <unistd.h>

#include <gdtoa.h>

//From http://unix.superglobalmegacorp.com/xnu/newsrc/bsd/include/stdlib.h.html
typedef struct {
	int quot;		/* quotient */
	int rem;		/* remainder */
} div_t;

typedef struct {
	long quot;		/* quotient */
	long rem;		/* remainder */
} ldiv_t;

//http://freebsd70.unixmanpages.net/man3/lldiv.html
typedef struct {
	long long quot; /* Quotient. */
	long long rem;  /* Remainder. */
} lldiv_t;

//Wrapper of free()
void cfree(void *p);

int atoi(const char *str);
long atol(const char *str);
long long atoll(const char *str);


//POSIX style environment variables
int putenv(char *str);
int setenv(const char *name, const char *value, int rewrite);
int unsetenv(const char *name);
char *getenv(const char *name);

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
long labs(long j);

int
heapsort(void *vbase, size_t nmemb, size_t size,
    int (*compar)(const void *, const void *));

char *
ecvt(double value, int ndigit, int *decpt, int *sign);

char *
fcvt(double value, int ndigit, int *decpt, int *sign);

void *
bsearch(const void *key, const void *base0, size_t nmemb, size_t size,
	int (*compar)(const void *, const void *));

long
jrand48(unsigned short xseed[3]);

div_t div(int num, int denom);

ldiv_t ldiv(long num, long denom);

lldiv_t
lldiv(long long num, long long denom);

//Radix sort...

int
radixsort(const u_char **a, int n, const u_char *tab, u_int endch);

int
sradixsort(const u_char **a, int n, const u_char *tab, u_int endch);

//void
//r_sort_a(const u_char **a, int n, int i, const u_char *tr, u_int endch);

//void
//r_sort_b(const u_char **a, const u_char **ta, int n, int i, const u_char *tr,
 //   u_int endch);

//static __inline void
//simplesort(const u_char **a, int n, int b, const u_char *tr, u_int endch);

//Really?
size_t strlen(const char *str);

long
strtol(const char *nptr, char **endptr, int base);


//void * calloc(size_t num, size_t size);
void * memset(void *s1, int c, size_t n);
void * memcpy(void *s1, const void *s2, size_t n);
long long
strtonum(const char *numstr, long long minval, long long maxval,
    const char **errstrp);

double
atof(const char *ascii);

void
qsort(void *aa, size_t n, size_t es, int (*cmp)(const void *, const void *));

//Dummy implementation for WaterFront BASIC
int system(const char *command);

long long llabs(long long j);

#ifdef __cplusplus
}
#endif


#endif /* !__STDLIB_H__ */

