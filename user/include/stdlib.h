#ifndef __STDLIB_H__
#define __STDLIB_H__

#ifdef __cplusplus
extern "C" {
#endif

//Define of size_t
#include <liballoc.h>

#include <sys/cdefs.h>
#include <sys/types.h>

int rand_r(u_int *seed);
int rand(void);
void srand(u_int seed);

char * l64a(long value);
int l64a_r(long value, char *buffer, int buflen);

void *
bsearch(const void *key, const void *base0, size_t nmemb, size_t size,
	int (*compar)(const void *, const void *));

//Dummy implementation for WaterFront BASIC
int system(const char *command);

#ifdef __cplusplus
}
#endif


#endif /* !__STDLIB_H__ */

