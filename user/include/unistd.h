
#ifndef __UNISTD_H__
#define __UNISTD_H__

#include <l4/types.h>

#include "stdarg.h"

#ifdef __cplusplus
extern "C" {
#endif

//BSD swab
void swab(const void *from, void *to, size_t len);

#ifdef __cplusplus
}
#endif


#endif /* !__UNISTD_H__ */

