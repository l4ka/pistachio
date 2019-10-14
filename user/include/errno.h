#ifndef __ERRNO_H__
#define __ERRNO_H__

//http://fxr.watson.org/fxr/source/sys/errno.h?v=OPENBSD;im=excerpts#L80

//This probably won't work
extern int errno;

#define ENOENT          2               /* No such file or directory */
#define EBADF           9               /* Bad file descriptor */
#define EEXIST          17              /* File exists */
#define EINVAL 22
#define ERANGE 34
#endif
