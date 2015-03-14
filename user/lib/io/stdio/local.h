

#pragma once

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

//ftp://ftp.fr.openbsd.org/pub/OpenBSD/src/lib/libc/stdio/local.h
//https://platform--bionic.android-source-browsing.googlecode.com/git/libc/stdio/local.h

extern int __sdidinit;

int	__sflush(FILE *);
int	__sflush_locked(FILE *);
int	_fwalk(int (*)(FILE *));
void    __sinit(void);
/*
 * Test whether the given stdio file has an active ungetc buffer;
 * release such a buffer, without restoring ordinary unread data.
 */

//http://code.metager.de/source/xref/OpenBSD/src/lib/libc/stdio/fileext.h is what defines _UB

//EVIL HAX
struct __sfileext {
	struct	__sbuf _ub; /* ungetc buffer */
//	struct wchar_io_data _wcio;	/* wide char io status */
};

#define _EXT(fp) ((struct __sfileext *)((fp)->_ext._base))
#define _UB(fp) _EXT(fp)->_ub
//That's what makes McDonald's(TM)

#define  HASUB(fp) (_UB(fp)._base != NULL)
#define  FREEUB(fp) { \
  if (_UB(fp)._base != (fp)->_ubuf) \
    free(_UB(fp)._base); \
  _UB(fp)._base = NULL; \
}

//http://stackoverflow.com/questions/21154211/freebsds-isthreaded-equivalent-in-linux
//This is a hack
#define __isthreaded 1
#define FLOCKFILE(fp)	do { if (__isthreaded) flockfile(fp); } while (0)
#define FUNLOCKFILE(fp)	do { if (__isthreaded) funlockfile(fp); } while (0)

#ifdef __cplusplus
}
#endif

