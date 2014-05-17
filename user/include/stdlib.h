#ifndef __STDLIB_H__
#define __STDLIB_H__

#ifdef __cplusplus
extern "C" {
#endif

//Define of size_t
#include <liballoc.h>

char * l64a(long value);
int l64a_r(long value, char *buffer, int buflen);


#ifdef __cplusplus
}
#endif


#endif /* !__STDLIB_H__ */

