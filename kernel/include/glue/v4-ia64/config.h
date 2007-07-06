/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     glue/v4-ia64/config.h
 * Description:   
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
 * $Id: config.h,v 1.32 2003/09/24 19:04:36 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_IA64__CONFIG_H__
#define __GLUE__V4_IA64__CONFIG_H__

#include INC_API(config.h)
#include INC_ARCH(config.h)

#define MIN_PAGE_SIZE		(__UL(4096))
#define MIN_PAGE_MASK		(~(MIN_PAGE_SIZE-1))

#define KTCB_BITSIZE		(13)
#define KTCB_SIZE		(__UL(1) << KTCB_BITSIZE)
#define KTCB_MASK		(~(KTCB_SIZE-1))

#define KIP_SIZE_LOG2		(15)


/**
 * Offset from empty kernel SP to empty RSE SP
 */
#define IA64_SP_TO_RSE_OFFSET	(KTCB_SIZE - 0x200)


/*
 * Setup for user-stubs of system calls.
 */

#define KIP_SYSCALL(x)		((word_t *) (x))[0]
#define ARCH_SYSCALL0		KIP_SYSCALL (user_pal_call)
#define ARCH_SYSCALL1		KIP_SYSCALL (user_sal_call)

#if !defined(ASSEMBLY) && defined(__cplusplus)
extern "C" void SECTION (".user.syscall.pal_call") user_pal_call (void);
extern "C" void SECTION (".user.syscall.sal_call") user_sal_call (void);
#endif


/*
 * Values for kernel interface page.
 */

/* 64bit, little endian */
#define KIP_API_FLAGS		{SHUFFLE2(endian:0, word_size:1)}	

/* 8 byte aligned, 1KB size, 4KB area size */
#define KIP_UTCB_INFO		{SHUFFLE3(multiplier:1, alignment:10, size:12)}

/* 2*8KB */
#define KIP_KIP_AREA		{ KIP_SIZE_LOG2 }

/* write+exec */
#define KIP_ARCH_PAGEINFO	{SHUFFLE2(rwx:3, size_mask:\
                                           ((1 << 12) |  /*   4KB */ \
					    (1 << 13) |  /*   8KB */ \
					    (1 << 14) |  /*  16KB */ \
					    (1 << 16) |  /*  64KB */ \
					    (1 << 18) |  /* 256KB */ \
					    (1 << 20) |  /*   1MB */ \
					    (1 << 22) |  /*   4MB */ \
					    (1 << 24) |  /*  16MB */ \
					    (1 << 26) |  /*  64MB */ \
					    (1 << 28))   /* 256MB */ >> 10)}

/* Minimum number of memory descriptors required in KIP */
#define KIP_MIN_MEMDESCS	(32)


/* Number of usec in a timer tick */
#define TIMER_TICK_LENGTH	(2000)

#define CACHE_LINE_SIZE		(IA64_CACHE_LINE_SIZE)

#define KERNEL_AREA_START	(7UL << 61)

#define VALID_THREADNO_BITS	(17)
#define KTCB_REGION_ID		(__UL(5))
#define KTCB_AREA_START		(KTCB_REGION_ID << 61)
#define KTCB_AREA_SIZE		(KTCB_SIZE << L4_GLOBAL_THREADNO_BITS)
#define KTCB_AREA_END		(KTCB_AREA_START + KTCB_AREA_SIZE)

#define USER_AREA_START		(0UL)
#define USER_AREA_END		(1UL << 61)

#define ROOT_UTCB_START		(1UL << 32)
#define ROOT_KIP_START		(1UL << 33)

#endif /* !__GLUE__V4_IA64__CONFIG_H__ */
