/*********************************************************************
 *                
 * Copyright (C) 2002-2003,  University of New South Wales
 *                
 * File path:     glue/v4-mips64/config.h
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
 * $Id: config.h,v 1.17 2004/12/02 00:06:19 cvansch Exp $
 *                
 ********************************************************************/

#ifndef __GLUE__V4_MIPS64__CONFIG_H__
#define __GLUE__V4_MIPS64__CONFIG_H__

#include INC_API(config.h)
#include INC_ARCH(page.h)

/**
 * Size of a kernel TCB in bytes
 */
#define KTCB_SIZE	4096
#define KTCB_BITS       12
#define KTCB_MASK       (~((1 << KTCB_BITS) - 1))

#define UTCB_SIZE       1024
#define UTCB_BITS       10
#define UTCB_MASK       (~((1 << UTCB_BITS) - 1))


/**
   attributes for system call functions
   @param x is the name of the system call lacking the leading sys_ .
   This makes it possible to place every system call in its own section
   if required. Default is empty.
 */
#define KIP_SYSCALL(x)          ((word_t) (x) - (word_t) &kip)

/*
 * Values for Kernel Interface Page (KIP).
 */
#if CONFIG_MIPS64_LITTLE_ENDIAN
  #define KIP_API_FLAGS	{endian:0, word_size:1} /* 64-bit, little endian */
#else
  #define KIP_API_FLAGS	{SHUFFLE2(endian:1, word_size:1)} /* 64-bit, big endian */
#endif


/*
 * minimum size of UTCB area and number of UTCBs in this
 */
/* 8 byte aligned, 1KB size, 4KB area size */
#define KIP_UTCB_INFO		{SHUFFLE3(multiplier:1, alignment:UTCB_BITS, size:MIPS64_PAGE_BITS)}

/*
 * size of kernel interface page
 */
#define KIP_KIP_AREA	{size:MIPS64_PAGE_BITS}   /* 4KB */


#define KIP_ARCH_PAGEINFO {SHUFFLE2(rwx:3, size_mask:\
						HW_VALID_PGSIZES >> 10)}


/* Shared */

/* VALID_THREADNO_BITS:
 *  As Simon says, Somewhat arbitrary ... just give them 1 top level PT..
 * Perhaps we should define this more intelligently. 
 *  eg. half the size of the XKSEG (mappings fit there too??)
 */
#define VALID_THREADNO_BITS	(TOPLEVEL_PT_BITS - KTCB_BITS)
#define VALID_THREADNO_MASK	((1ul << VALID_THREADNO_BITS)-1)
#define KTCB_AREA_SIZE		(1UL << (KTCB_BITS + VALID_THREADNO_BITS))
#define KTCB_AREA_START		AS_XKSEG_START
#define KTCB_AREA_END		(KTCB_AREA_START + KTCB_AREA_SIZE - 1)

#define COPY_AREA_START		(AS_CKSEG3_START)
#define COPY_AREA_SIZE		(AS_CKSEG3_SIZE)
#define COPY_AREA_END		(COPY_AREA_START + COPY_AREA_SIZE - 1)

#define CPU_AREA_START		(AS_CKSSEG_START)
#define CPU_AREA_SIZE		(64*1024)
#define CPU_AREA_END		(CPU_AREA_START + CPU_AREA_SIZE)

#define USER_AREA_START		AS_XKUSEG_START
#define USER_AREA_END		AS_XKUSEG_END

#define ROOT_UTCB_START		(1UL << 32)
#define ROOT_KIP_START		(1UL << 33)

#define CACHE_LINE_SIZE		(CONFIG_MIPS64_CACHE_LINE_SIZE)

/* Number of usec in a timer tick. NB CONFIG_CPU_CLOCK_SPEED in kHz!! */
#ifdef CONFIG_PLAT_SB1
# define TIMER_PERIOD		(CONFIG_CPU_CLOCK_SPEED*2)
#else
# define TIMER_PERIOD		(CONFIG_CPU_CLOCK_SPEED)
#endif
/* This works because:
 * 500 ints/sec,
 * CPU_CLOCK_SPEED/2 timer ticks/sec
 * PERIOD = (CPU_CLOCK_SPEED/2)/500
 *	  = CPU_CLOCK_SPEED/1000
 *	  = CONFIG_CPU_CLOCK (kHz)
 */

#define TIMER_TICK_LENGTH	(2000) /* usec */

#endif /* !__GLUE__V4_MIPS64__CONFIG_H__ */
