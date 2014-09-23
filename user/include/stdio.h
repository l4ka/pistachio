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

/* stdio buffers */
struct __sbuf {
	unsigned char *_base;
	int	_size;
};

//Do like OpenBSD for now...
typedef	struct __sFILE {
	unsigned char *_p;	/* current position in (some) buffer */
	int	_r;		/* read space left for getc() */
	int	_w;		/* write space left for putc() */
	short	_flags;		/* flags, below; this FILE is free if 0 */
	struct	__sbuf _bf;	/* the buffer (at least 1 byte, if !NULL) */
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

//SysV yuckiness
#define	BUFSIZ	1024		/* size of buffer used by setbuf */

//Various magic values...
#define	__SLBF	0x0001		/* line buffered */
#define	__SNBF	0x0002		/* unbuffered */
#define	__SWR	0x0008		/* OK to write */
#define	__SEOF	0x0020		/* found EOF */
#define	__SERR	0x0040		/* found error */
#define	__SSTR	0x0200		/* this is an sprintf/snprintf string */
#define	__SALC	0x4000		/* allocate string space dynamically */

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

