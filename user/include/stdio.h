#ifndef __STDIO_H__
#define __STDIO_H__
#include <l4io.h>

#define EOF (-1)
#include <sys/types.h>

#include <lib9.h>

#ifdef __cplusplus
extern "C" {
#endif

//https://github.com/haiku/haiku/blob/449f7f5a7b9d381c1eeab3ddfbdba0db0ce05c22/headers/posix/stdio.h
//ftp://ftp.fr.openbsd.org/pub/OpenBSD/src/include/stdio.h defines FILE as a structure



//Do like OpenBSD for now...
typedef	struct __sFILE {
	unsigned char *_p;	/* current position in (some) buffer */
	int	_r;		/* read space left for getc() */
	int	_w;		/* write space left for putc() */
	short	_flags;		/* flags, below; this FILE is free if 0 */
} FILE;

extern FILE __sF[];

//We've already got this, somewhere else...
#ifndef NULL
#ifdef 	__GNUG__
#define	NULL	__null
#elif defined(__cplusplus)
#define	NULL	0L
#else
#define	NULL	((void *)0)
#endif
#endif

#define	stdin	(&__sF[0])
#define	stdout	(&__sF[1])
#define	stderr	(&__sF[2])
//End

//Haiku suggests doing the following...
//This is a hack, we should replace it with something nicer, ASAP
//extern FILE *stdin; //FILE is abstract, but what should it be?
//extern FILE *stdout;
//extern FILE *stderr;

//Be BeOS-compatible...
int is_computer_on();

//Print debugging text with a file name/class tag
void EDebugPrintf(const char *aTag, const char *aText);

#ifdef __cplusplus
}
#endif

#endif /* !__STDIO_H__ */

