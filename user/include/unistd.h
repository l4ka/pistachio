
#ifndef __UNISTD_H__
#define __UNISTD_H__

#include <l4/types.h>

#include "stdarg.h"

extern char **environ; //Tres dumb

#ifdef __cplusplus
extern "C" {
#endif

//BSD swab
void swab(const void *from, void *to, size_t len);

//http://man7.org/linux/man-pages/man2/getpagesize.2.html
//OSF/1, Linux getpagesize()
int getpagesize(void);

#ifdef __cplusplus
}
#endif


#endif /* !__UNISTD_H__ */

