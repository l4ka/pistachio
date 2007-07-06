/*********************************************************************
 *                
 * Copyright (C) 2002-2004,  University of New South Wales
 *                
 * File path:     arch/mips64/page.h
 * Created:       30/07/2002 10:48:30 by Daniel Potts (danielp)
 * Description:   MIPS64 specific MM 
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
 * $Id: page.h,v 1.12 2004/06/04 02:14:25 cvansch Exp $
 *                
 ********************************************************************/

#ifndef __ARCH__MIPS64__PAGE_H__
#define __ARCH__MIPS64__PAGE_H__

#include <config.h>
#include INC_PLAT(config.h)

/* #define MIPS64_PT_LEVELS         4 */

#if CONFIG_MIPS64_ADDRESS_BITS == 40

#define TOPLEVEL_PT_BITS (39)

#if 0
/* We are using a 3 level PT */
#define MIPS64_PT_LEVELS         3

/* How many bits each level maps */
#define MIPS64_PT_BITS           {33, 23, 13}

#define PTBR_SPACE_OFFSET       (512 * 8)

#endif

/* HW_PGSHIFTS must match pgsize_e (pgent.h) */
/* space_t is (1024)*8  - 2^10 entries */
/* so we have 12 -> 22 -> 32 -> 40 (42 -> 44 for sibyte) */
/* #define HW_PGSHIFTS            { 12, 14, 16, 18, 20, 22, 24, 32, 40 }  XXX */
#define HW_PGSHIFTS            { 12, 14, 16, 18, 20, 22, 24, 32, 41 } /* 41 - virtual ktcb */

#define MDB_NUM_PGSIZES        (8)

#define HW_VALID_PGSIZES       ((1 << 12) |  /*   4KB */ \
                                (1 << 14) |  /*  16KB */ \
                                (1 << 16) |  /*  64KB */ \
                                (1 << 18) |  /* 256KB */ \
                                (1 << 20) |  /*   1MB */ \
                                (1 << 22) |  /*   4MB */ \
                                (1 << 24))   /*  16MB */


#elif CONFIG_MIPS64_ADDRESS_BITS == 44

#define TOPLEVEL_PT_BITS (39)

#if 0
/* We are using a 4 level PT */
#define MIPS64_PT_LEVELS         4
#define MIPS64_PT_BITS           {43, 33, 23, 13}
#define TOPLEVEL_PT_BITS        43

#define PTBR_SPACE_OFFSET       (512 * 8)
#endif


/* HW_PGSHIFTS must match pgsize_e (pgent.h) */
/* space_t is (1024)*8  - 2^10 entries */
/* so we have 12 -> 22 -> 32 -> 40 (42 -> 44 for sibyte) */
/*#define HW_PGSHIFTS            { 12, 14, 16, 18, 20, 22, 24, 32, 40 } * XXX */
#define HW_PGSHIFTS            { 12, 14, 16, 18, 20, 22, 24, 26, 28, 32, 41 } /* 41 - virtual ktcb */

#define MDB_NUM_PGSIZES        (10)

#define HW_VALID_PGSIZES       ((1 << 12) |  /*   4KB */ \
                                (1 << 14) |  /*  16KB */ \
                                (1 << 16) |  /*  64KB */ \
                                (1 << 18) |  /* 256KB */ \
                                (1 << 20) |  /*   1MB */ \
                                (1 << 22) |  /*   4MB */ \
                                (1 << 24) |  /*  16MB */ \
				(1 << 26) |  /*  64MB */ \
				(1 << 28))   /* 256MB */



#else
#error We only support 40 and 44 bit address spaces!
#endif /* CONFIG_MIPS64_ADDRESS_BITS */

/* Basic page sizes etc. */

#define MIPS64_PAGE_BITS         12
#define MIPS64_PAGE_SIZE         (1UL << MIPS64_PAGE_BITS)
#define MIPS64_PAGE_MASK         (~(MIPS64_PAGE_SIZE - 1))
#define MIPS64_OFFSET_MASK       (~MIPS64_PAGE_MASK)


/* Address space layout */


#define AS_XKUSEG_START		(0)
#define AS_XKUSEG_SIZE		(1UL << (CONFIG_MIPS64_ADDRESS_BITS))
#define AS_XKUSEG_END		(AS_XKUSEG_START + AS_XKUSEG_SIZE - 1)

/* XKPHYS is 8 segments of 2^PABITS each with different mapping modes.
 * SIZE and END are defined only for one of these segments.
 * If we want different cache attributes for this region, we could adjust START.
 */
#define AS_XKPHYS_START		(0x8000000000000000)
#define AS_XKPHYS_SIZE		(1UL << CONFIG_MIPS64_PHYS_ADDRESS_BITS)
#define AS_XKPHYS_END		(AS_XKPHYS_START + AS_XKPHYS_SIZE - 1)

#define AS_CKSEG0_START		(0xFFFFFFFF80000000)
#define AS_CKSEG0_SIZE		(1UL << 29)
#define AS_CKSEG0_END		(AS_CKSEG0_START + AS_CKSEG0_SIZE - 1)

#define AS_CKSEG1_START		(0xFFFFFFFFa0000000)
#define AS_CKSEG1_SIZE		(1UL << 29)
#define AS_CKSEG1_END		(AS_CKSEG2_START + AS_CKSEG2_SIZE - 1)

#define AS_CKSSEG_START		(0xFFFFFFFFc0000000)
#define AS_CKSSEG_SIZE		(1UL << 29)
#define AS_CKSSEG_END		(AS_CKSEG2_START + AS_CKSEG2_SIZE - 1)

#define AS_CKSEG3_START		(0xFFFFFFFFe0000000)
#define AS_CKSEG3_SIZE		(1UL << 29)
#define AS_CKSEG3_END		(AS_CKSEG3_START + AS_CKSEG3_SIZE - 1)

#define AS_XKSEG_START		(0x4000000000000000)
#define AS_XKSEG_SIZE		((1UL << CONFIG_MIPS64_ADDRESS_BITS))
#define AS_XKSEG_END		(AS_XKSEG_START + AS_XKSEG_SIZE - 1)

/* #define KERNEL_OFFSET           (AS_XKPHYS_START + CONFIG_MIPS64_CONSOLE_RESERVE) */

/* define where kernel starts */
/*
#define AS_KSEG_START 		AS_XKPHYS_START
#define AS_KSEG_END		AS_XKPHYS_END
*/
#define AS_KSEG_START 		AS_CKSEG0_START
#define AS_KSEG_END		AS_CKSEG0_END
#define AS_KSEG_SIZE		AS_CKSEG0_SIZE

#endif /* __ARCH__MIPS64__PAGE_H__ */
