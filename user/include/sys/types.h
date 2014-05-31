#ifndef __SYS_TYPES_H__
#define __SYS_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif


#include <l4/types.h>

//Rand wants this
//http://fxr.watson.org/fxr/source/sys/types.h?v=OPENBSD
typedef unsigned int    u_int;

#define	RAND_MAX	2147483647 // Like Linux

//Really?
size_t strlen(const char *str);

//
void * memset(void *s1, int c, size_t n);
void * memcpy(void *s1, const void *s2, size_t n);


#ifdef __cplusplus
}
#endif


#endif /* !__SYS_TYPES_H__ */

