
//https://sourceware.org/ml/newlib/2013/msg00234.html
#ifndef	__DECONST
#define	__DECONST(type, var)	((type)(uintptr_t)(const void *)(var))
#endif

//HAX! - http://www.opensource.apple.com/source/gm4/gm4-15/lib/stdint_.h
//These will probably break things badly, in the future

/*
#ifdef intptr_t
	#undef intptr_t
	#define intptr_t long int
#endif 

#ifdef uintptr_t
	#undef uintptr_t
	#define uintptr_t unsigned long int
#endif

*/
