#pragma once

#include <stdint.h>
//https://sourceware.org/ml/newlib/2013/msg00234.html
#ifndef	__DECONST
#define	__DECONST(type, var)	((type)(uintptr_t)(const void *)(var))
#endif

//http://stackoverflow.com/questions/8087438/should-i-have-to-use-an-extern-c-block-to-include-the-c-headers
/* C++ needs to know that types and declarations are C, not C++.  */
#ifdef   __cplusplus
# define __BEGIN_DECLS  extern "C" {                                            
# define __END_DECLS }
#else
# define __BEGIN_DECLS
# define __END_DECLS
#endif

//HAX! - http://www.opensource.apple.com/source/gm4/gm4-15/lib/stdint_.h
//These will probably break things badly, in the future


//#ifdef intptr_t
//	#undef intptr_t
	#define intptr_t long int
//#endif 

//#ifdef uintptr_t
	//#undef uintptr_t
//	#define uintptr_t unsigned long int
//#endif

