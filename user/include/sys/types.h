#ifndef __SYS_TYPES_H__
#define __SYS_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

//http://www.cplusplus.com/reference/cstdlib/

#include <l4/types.h>

//Rand wants this
//http://fxr.watson.org/fxr/source/sys/types.h?v=OPENBSD
typedef unsigned int    u_int;

#define	RAND_MAX	2147483647 // Like Linux

void * memset(void *s1, int c, size_t n);
size_t strlen(const char *str);




#ifdef __cplusplus
}
#endif


#endif /* !__SYS_TYPES_H__ */

