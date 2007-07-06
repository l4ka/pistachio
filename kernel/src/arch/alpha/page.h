/*********************************************************************
 *                
 * Copyright (C) 2002,  University of New South Wales
 *                
 * File path:     arch/alpha/page.h
 * Created:       27/07/2002 20:35:30 by Simon Winwood (sjw)
 * Description:   Alpha specific MM 
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
 * $Id: page.h,v 1.8 2003/09/24 19:04:25 skoglund Exp $
 *                
 ********************************************************************/

#ifndef __ARCH__ALPHA__PAGE_H__
#define __ARCH__ALPHA__PAGE_H__

#include <config.h>


/* If we are using a 43 bit AS we stick space_t in the kseg mapping area (which is unused), 
 * the first 256 entries in the second half of the L1 PT.  In a 48 bit AS, we stick space_t in the
 * same spot.
 */
#if CONFIG_ALPHA_ADDRESS_BITS == 43
/* We are using a 3 level PT */
#define ALPHA_PT_LEVELS         3
#define TOPLEVEL_PT_BITS        33

#define PTBR_SPACE_OFFSET       (512 * 8)

#define HW_PGSHIFTS            { 13, 16, 19, 22, 23, 33, 43}
#define NUM_HW_PGSHIFTS         7

#elif CONFIG_ALPHA_ADDRESS_BITS == 48
/* We are using a 4 level PT */
#define ALPHA_PT_LEVELS         4
#define TOPLEVEL_PT_BITS        43

#define PTBR_SPACE_OFFSET       (512 * 8)

#define HW_PGSHIFTS            { 13, 16, 19, 22, 23, 33, 43, 48}
#define NUM_HW_PGSHIFTS         8
#else
#error We only support 43 and 48 bit address spaces!
#endif /* CONFIG_ALPHA_ADDRESS_BITS */

/* Basic page sizes etc. */

#define ALPHA_PAGE_BITS         13
#define ALPHA_PAGE_SIZE         (1UL << ALPHA_PAGE_BITS)
#define ALPHA_PAGE_MASK         (~(ALPHA_PAGE_SIZE - 1))
#define ALPHA_OFFSET_MASK       (~ALPHA_PAGE_MASK)

/* sjw (17/09/2002): FIXME --- CPU specific? */
#define ALPHA_CACHE_LINE_SIZE   64

/* sjw (17/09/2002): Correct? */
#define HW_VALID_PGSIZES       ((1 << 13) | (1 << 16) | (1 << 19) | (1 << 22))


/* Address space layout */

/* The basic AS layout is defined by OSF PAL.  It consists of 3 segments (for a 43 bit AS):
 *
 *  seg0:                    0 ... (0x40000000000 - 1)
 *  kseg:   0xfffffc0000000000 ... (0xfffffd0000000000 - 1)
 *  seg1:   0xfffffd0000000000 ... 0xffffffffffffffff
 */

#define AS_SEG0_START           (0)
#define AS_SEG0_SIZE            (1UL << (CONFIG_ALPHA_ADDRESS_BITS - 1))
#define AS_SEG0_END             (AS_SEG0_START + AS_SEG0_SIZE)

#define AS_KSEG_START           (-1UL << (CONFIG_ALPHA_ADDRESS_BITS - 1))
#define AS_KSEG_SIZE            (1UL << (CONFIG_ALPHA_ADDRESS_BITS - 2))
#define AS_KSEG_END             (AS_KSEG_START + AS_KSEG_SIZE)

#define AS_SEG1_START           (-1UL << (CONFIG_ALPHA_ADDRESS_BITS - 2))
#define AS_SEG1_SIZE            (1UL << (CONFIG_ALPHA_ADDRESS_BITS - 2))
#define AS_SEG1_END             (AS_SEG1_START + AS_SEG1_SIZE)

/* #define KERNEL_OFFSET           (AS_KSEG_START + CONFIG_ALPHA_CONSOLE_RESERVE) */

#endif /* __ARCH__ALPHA__PAGE_H__ */
