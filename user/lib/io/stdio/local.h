

#pragma once

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif


//#include <stdio.h>
//ftp://ftp.fr.openbsd.org/pub/OpenBSD/src/lib/libc/stdio/local.h


extern int __sdidinit;

int	__sflush(FILE *);
int	__sflush_locked(FILE *);
int	_fwalk(int (*)(FILE *));

//http://stackoverflow.com/questions/21154211/freebsds-isthreaded-equivalent-in-linux
//This is a hack
#define __isthreaded 1
#define FLOCKFILE(fp)	do { if (__isthreaded) flockfile(fp); } while (0)
#define FUNLOCKFILE(fp)	do { if (__isthreaded) funlockfile(fp); } while (0)

#ifdef __cplusplus
}
#endif

