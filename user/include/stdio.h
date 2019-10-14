#ifndef __STDIO_H__
#define __STDIO_H__
#include <l4io.h>



#include <mutex/mutex.h>
#define EOF (-1)
#include <sys/types.h>

#include <lib9.h>

#ifdef __cplusplus
extern "C" {
#endif

//https://github.com/haiku/haiku/blob/449f7f5a7b9d381c1eeab3ddfbdba0db0ce05c22/headers/posix/stdio.h
//ftp://ftp.fr.openbsd.org/pub/OpenBSD/src/include/stdio.h defines FILE as a structure


#include <stdint.h> //Is another hack
//http://code.metager.de/source/xref/OpenBSD/src/include/stdio.h
typedef	__off_t	off_t;
typedef off_t fpos_t;		/* stdio file position type */

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
	short	_file;		/* fileno, if Unix descriptor, else -1 */
	struct	__sbuf _bf;	/* the buffer (at least 1 byte, if !NULL) */
	int	_lbfsize;	/* 0 or -_bf._size, for inline putc */

	void	*_cookie;	/* cookie passed to io functions */
	int	(*_read)(void *, char *, int);
	int	(*_write)(void *, const char *, int);

	/* extension data, to avoid further ABI breakage */
	struct	__sbuf _ext;

	/* data for long sequences of ungetc() */
	unsigned char *_up;	/* saved _p when _p is doing ungetc data */
	int	_ur;		/* saved _r when _r is counting ungetc data */
	
	/* tricks to meet minimum requirements even when malloc() fails */
	unsigned char _ubuf[3];	/* guarantee an ungetc() buffer */
	unsigned char _nbuf[1];	/* guarantee a getc() buffer */

//NICTA extension
	void *handle;
	size_t (*read_fn)(void *, long int, size_t, void *);
	size_t (*write_fn)(void *, long int, size_t, void *);

	int (*close_fn)(void *);
	long int (*eof_fn)(void *);

	unsigned char buffering_mode;
	char *buffer;

	int eof;
	unsigned char unget_pos;
	long int current_pos;

	struct mutex mutex;

	/* Unix stdio files get aligned to block boundaries on fseek() */
	int	_blksize;	/* stat.st_blksize (may be != _bf._size) */
	fpos_t	_offset;	/* current lseek offset */
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

/*

struct __file __stdout = {
	NULL,
	NULL,
	l4kdb_write,
	NULL,
	NULL,
	_IONBF,
	NULL,
	0,
	0
};
*/

//End

//NICTA

#include <mutex/mutex.h>
#define lock_stream(s) mutex_count_lock(&(s)->mutex)
#define unlock_stream(s) mutex_count_unlock(&(s)->mutex)

#define SEEK_CUR 0
#define SEEK_END 1
#define SEEK_SET 2

//SysV yuckiness
#define	BUFSIZ	1024		/* size of buffer used by setbuf */

#define _IOFBF 0
#define _IOLBF 1
#define _IONBF 2

//Various magic values...
#define	__SLBF	0x0001		/* line buffered */
#define	__SNBF	0x0002		/* unbuffered */
#define	__SRD	0x0004		/* OK to read */
#define	__SWR	0x0008		/* OK to write */
	/* RD and WR are never simultaneously asserted */
#define	__SRW	0x0010		/* open for reading & writing */
#define	__SEOF	0x0020		/* found EOF */
#define	__SERR	0x0040		/* found error */
#define	__SMBF	0x0080		/* _buf is from malloc */
#define	__SAPP	0x0100		/* fdopen()ed in append mode */
#define	__SSTR	0x0200		/* this is an sprintf/snprintf string */
#define	__SOPT	0x0400		/* do fseek() optimisation */
#define	__SNPT	0x0800		/* do not do fseek() optimisation */
#define	__SOFF	0x1000		/* set iff _offset is in fact correct */
#define	__SMOD	0x2000		/* true => fgetln modified _p text */
#define	__SALC	0x4000		/* allocate string space dynamically */
#define __SIGN	0x8000		/* ignore this file in _fwalk */

//Haiku suggests doing the following...
//This is a hack, we should replace it with something nicer, ASAP
//extern FILE *stdin; //FILE is abstract, but what should it be?
//extern FILE *stdout;
//extern FILE *stderr;

//Be BeOS-compatible...
int is_computer_on();

//Print debugging text with a file name/class tag
void EDebugPrintf(const char *aTag, const char *aText);

int fseek(FILE *, long int, int);
void rewind(FILE *); //When the crowd say "Bo Selecta"
char *
fgets(char *buf, register int n, register FILE *fp);
void flockfile(FILE *fp);
void funlockfile(FILE *fp); //FunLock(TM) Files : The best kind of file


//Should be namespaced
//int
//__sfvwrite(FILE *fp, struct __suio *uio);

#ifdef __cplusplus
}
#endif

#endif /* !__STDIO_H__ */

