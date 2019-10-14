
#include <fmt.h>
#include <inttypes.h>
#include <hash.h>
#include <stdint.h>

#include <math.h> /* For HUGE_VAL */

/*
 * compiler directive on Plan 9
 */
#ifndef USED
#define USED(x) if(x);else
#endif
//typedef unsigned int		uintptr_t;
//typedef uintptr_t uintptr;

//HAX!
typedef unsigned int		uintptr;
#define _fmtuintptr uintptr	
/*
 * easiest way to make sure these are defined
 */
typedef uchar	_fmtuchar;
typedef ushort	_fmtushort;
typedef uint	_fmtuint;
typedef ulong	_fmtulong;
typedef vlong	_fmtvlong;
typedef uvlong	_fmtuvlong;


//

//#undef uintptr_t
//#define uintptr_t unsigned long int


/*
 * nil cannot be ((void*)0) on ANSI C,
 * because it is used for function pointers
 */
#undef	nil
#define	nil	0

#undef	nelem
#define	nelem(x)	(sizeof (x)/sizeof (x)[0])

