/*********************************************************************
 *
 * Copyright (C) 2002-2004,  Karlsruhe University
 *
 * File path:     macros.h
 * Description:   Omnipresent macro definitions
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: macros.h,v 1.38 2006/02/21 10:22:59 stoess Exp $
 *
 ********************************************************************/

#ifndef __MACROS_H__
#define __MACROS_H__

/*
 * This file is automatically included before the first line of any
 * source file, using gcc's -imacro command line option.  Only macro
 * definitions will be extracted.
 */


/*
 * Computed include paths for header files
 *
 * Precondition: __API__, __ARCH__, __CPU__ and __PLATFORM__ are defined
 * 
 * Use: If __API__=v4 and __ARCH__=x86 and __CPU__=p4 __PLATFORM__=pc99
 *    #include INC_ARCH(x.h) includes include/arch/x86/x.h
 *    #include INC_CPU(x.h) includes include/arch/x86/p4/x.h
 *    #include INC_PLAT(x.h) includes include/platform/pc99/x.h
 *    #include INC_GLUE(x.h) includes include/glue/v4-x86/x.h
 *
 */
#define INC_ARCH(x)             <arch/__ARCH__/x>
#define INC_CPU(x)              <arch/__ARCH__/__CPU__/x>
#define INC_PLAT(x)             <platform/__PLATFORM__/x>
#define INC_API(x)              <api/__API__/x>
#define INC_GLUE(x)             <glue/__API__-__ARCH__/x>
#define INC_GLUE_API_ARCH(x)    <glue/__API__-__ARCH__/x>
#define INC_GLUE_API_CPU(x)     <glue/__API__-__ARCH__/__CPU__/x>
#define INC_GLUE_API_PLAT(x)    <glue/__API__-__PLATFORM__/x>

/* use this to place code/data in a certain section */
#define SECTION(x) __attribute__((section(x)))
#define UNIT(x) __attribute__((section(".data." x)))

/* 
 * use WEAK to specifiy a weak function which can be replaced
 * by a architecture specific optimization
 * example: void WEAK bla()
 */
#define WEAK __attribute__(( weak ))

/**
 * Marks a function as noreturn.
 */
#define NORETURN __attribute__(( noreturn ))

/**
 * Marks a functions as not to be generated inline.
 */
#if (__GNUC__ >= 3) && (__GNUC_MINOR__ >= 1)
# define NOINLINE __attribute__ ((noinline))
#else
# define NOINLINE
#endif

/*
 * Mark variables as unused.
 */
#define UNUSED __attribute__(( unused ))

/* Functions with this are NEVER generated standalone. 
 * They are inlined only. Carefull: if the maximum inlining 
 * limit is reached gcc 3.x does not inline even if explicitly 
 * specified. Use -finline-limit=<large number> here. */
#define INLINE extern inline

/* Functions for critical path optimizations */
#if (__GNUC__ >= 3)
#define EXPECT_FALSE(x)		__builtin_expect((x), false)
#define EXPECT_TRUE(x)		__builtin_expect((x), true)
#define EXPECT_VALUE(x,val)	__builtin_expect((x), (val))
#else /* __GNUC__ < 3 */
#define EXPECT_TRUE(x)		(x)
#define EXPECT_FALSE(x)		(x)
#define EXPECT_VALUE(x,val)	(x)
#endif /* __GNUC__ < 3 */


/* Convenience functions for memory sizes. */
#define KB(x)	((typeof (x)) ((word_t) x * 1024))
#define MB(x)	((typeof (x)) ((word_t) x * 1024*1024))
#define GB(x)	((typeof (x)) ((word_t) x * 1024*1024*1024))

/* Offset for struct fields. */
#if (__GNUC__ >= 4)
#define offsetof(type, member)  __builtin_offsetof(type, member)
#else
#define offsetof(type, field)	((word_t) (&((type *) 0)->field))
#endif

/* Turn preprocessor symbol definition into string */
#define	MKSTR(sym)	MKSTR2(sym)
#define	MKSTR2(sym)	#sym

/* Safely "append" an UL suffix for also asm values */
#if defined(ASSEMBLY)
#define __UL(x)		x
#else
#define __UL(x)		x##UL
#endif


#if defined(CONFIG_BIGENDIAN)
#define BITFIELD2(t,a,b)		t b; t a;
#define BITFIELD3(t,a,b,c)		t c; t b; t a;
#define BITFIELD4(t,a,b,c,d)		t d; t c; t b; t a;
#define BITFIELD5(t,a,b,c,d,e)		t e; t d; t c; t b; t a;
#define BITFIELD6(t,a,b,c,d,e,f)	t f; t e; t d; t c; t b; t a;
#define BITFIELD7(t,a,b,c,d,e,f,g)	t g; t f; t e; t d; t c; t b; t a;
#define BITFIELD8(t,a,b,c,d,e,f,g,h)	t h; t g; t f; t e; t d; t c; t b; t a;
#define BITFIELD9(t,a,b,c,d,e,f,g,h,i)	t i; t h; t g; t f; t e; t d; t c; t b; t a;
#define BITFIELD10(t,a,b,c,d,e,f,g,h,i,j) t j; t i; t h; t g; t f; t e; t d; t c; t b; t a;
#define BITFIELD11(t,a,b,c,d,e,f,g,h,i,j,k) t k; t j; t i; t h; t g; t f; t e; t d; t c; t b; t a;
#define BITFIELD12(t,a,b,c,d,e,f,g,h,i,j,k,l) t l; t k; t j; t i; t h; t g; t f; t e; t d; t c; t b; t a;
#define BITFIELD13(t,a,b,c,d,e,f,g,h,i,j,k,l,m) t m; t l; t k; t j; t i; t h; t g; t f; t e; t d; t c; t b; t a;

#define BITFIELD17(t,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q) t q; t p; t o; t n; t m; t l; t k; t j;t i; t h; t g; t f; t e; t d; t c; t b; t a;

#define SHUFFLE2(a,b)			b,a
#define SHUFFLE3(a,b,c)			c,b,a
#define SHUFFLE4(a,b,c,d)		d,c,b,a
#define SHUFFLE5(a,b,c,d,e)		e,d,c,b,a
#define SHUFFLE6(a,b,c,d,e,f)		f,e,d,c,b,a
#define SHUFFLE7(a,b,c,d,e,f,g)		g,f,e,d,c,b,a
#else
#define BITFIELD2(t,a,b)		t a; t b;
#define BITFIELD3(t,a,b,c)		t a; t b; t c;
#define BITFIELD4(t,a,b,c,d)		t a; t b; t c; t d;
#define BITFIELD5(t,a,b,c,d,e)		t a; t b; t c; t d; t e;
#define BITFIELD6(t,a,b,c,d,e,f)	t a; t b; t c; t d; t e; t f;
#define BITFIELD7(t,a,b,c,d,e,f,g)	t a; t b; t c; t d; t e; t f; t g;
#define BITFIELD8(t,a,b,c,d,e,f,g,h)	t a; t b; t c; t d; t e; t f; t g; t h;
#define BITFIELD9(t,a,b,c,d,e,f,g,h,i)	t a; t b; t c; t d; t e; t f; t g; t h; t i;
#define BITFIELD10(t,a,b,c,d,e,f,g,h,i,j) t a; t b; t c; t d; t e; t f; t g; t h; t i; t j;
#define BITFIELD11(t,a,b,c,d,e,f,g,h,i,j,k) t a; t b; t c; t d; t e; t f; t g; t h; t i; t j; t k;
#define BITFIELD12(t,a,b,c,d,e,f,g,h,i,j,k,l) t a; t b; t c; t d; t e; t f; t g; t h; t i; t j; t k; t l;
#define BITFIELD13(t,a,b,c,d,e,f,g,h,i,j,k,l,m) t a; t b; t c; t d; t e; t f; t g; t h; t i; t j; t k; t l; t m;
#define BITFIELD17(t,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q) t a; t b; t c; t d; t e; t f; t g; t h; t i; t j; t k; t l; t m; t n; t o; t p; t q;
#define SHUFFLE2(a,b)			a,b
#define SHUFFLE3(a,b,c)			a,b,c
#define SHUFFLE4(a,b,c,d)		a,b,c,d
#define SHUFFLE5(a,b,c,d,e)		a,b,c,d,e
#define SHUFFLE6(a,b,c,d,e,f)		a,b,c,d,e,f
#define SHUFFLE7(a,b,c,d,e,f,g)		a,b,c,d,e,f,g
#endif

#endif /* !__MACROS_H__ */
