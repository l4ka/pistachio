/*********************************************************************
 *                
 * Copyright (C) 2006,  Karlsruhe University
 *                
 * File path:     glue/v4-mips32/config.h
 * Description:   Common configuration settings for MIPS32
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
 * $Id: config.h,v 1.1 2006/02/23 21:07:40 ud3 Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_MIPS32__CONFIG_H__
#define __GLUE__V4_MIPS32__CONFIG_H__

#define KIP_SYSCALL(x)          ((word_t) (x) - (word_t) &kip)

#define KUSEG_BASE              0x00000000
#define KSEG0_BASE              0x80000000
#define KSEG1_BASE              0xa0000000
#define KSEG2_BASE              0xc0000000
#define KSEG3_BASE              0xe0000000

#define COPY_AREA_SIZE          ( 8 * 1024 * 1024 )
#define COPY_AREA_START         ( KSEG2_BASE )
#define COPY_AREA_END           ( COPY_AREA_START + COPY_AREA_SIZE )

/* virtual asid configuration */
#define CONFIG_ASIDS_START	1
#define CONFIG_ASIDS_END	16
#define KERNEL_ASID		0

#define CONFIG_MAX_NUM_ASIDS    (CONFIG_ASIDS_END + 1) 
#define CONFIG_PREEMPT_ASIDS


#define HW_PGSHIFTS		{ 12, 22, 32 }
#define HW_VALID_PGSIZES	((1 << 12) | (1 << 22))

#define MDB_PGSHIFTS		{ 12, 22, 32 }
#define MDB_NUM_PGSIZES		(2)



/**
 * Size of a kernel TCB in bytes
 */
#define KTCB_SIZE               4096
#define KTCB_MASK               (~(KTCB_SIZE - 1))

#define UTCB_SIZE               1024
#define UTCB_MASK               (~(UTCB_SIZE - 1))


#define KTCB_AREA_START         ( KSEG2_BASE + COPY_AREA_SIZE )

#define MIPS32_PAGE_SIZE        4096
#define MIPS32_PAGE_BITS        12

/**
 * endianess and word size
 */
#define KIP_API_FLAGS           {endian:0, word_size:0} // 32-bit, little endian

/**
 * size of kernel interface page
 */
#define KIP_KIP_AREA            {size:12}   // 4KB

/**
 * supported page sizes and access bits
 */

#define KIP_ARCH_PAGEINFO       {SHUFFLE2(rwx:6, size_mask:(1 << 12) >> 10)}

#define KIP_MIN_MEMDESCS        (16)

/*
 * minimum size of UTCB area and number of UTCBs in this
 */
#define KIP_UTCB_INFO           {SHUFFLE3(multiplier:1, alignment:12, size:12)} // XXX

/**
 * Base address of the root task's UTCB area
 */
#define ROOT_UTCB_START		(1UL << 30)  // XXX

/**
 * Address of the KIP in the root task
 */
#define ROOT_KIP_START		((1UL << 30) - 0x100000 ) // XXX

#define VALID_THREADNO_BITS     17 // XXX


/**
 * Timer related stuff
 **/
#define TIMER_PERIOD	  	(5000) /* 10MHz -> 1ms */
#define TIMER_TICK_LENGTH	(1000)

/* magic constant put into GP[at] on a KernelInterface syscall */
#define MAGIC_KIP_REQUEST       (0x141fca11)

#endif /* !__GLUE__V4_MIPS32__CONFIG_H__ */
