/*********************************************************************
 *                
 * Copyright (C) 2003-2004, University of New South Wales
 *                
 * File path:    arch/sparc64/frame.h
 * Description:  Saved state for traps, windows and thread switches
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
 * $Id: frame.h,v 1.7 2004/07/01 04:00:04 philipd Exp $
 *                
 ********************************************************************/

#ifndef __ARCH__SPARC64__FRAME_H__
#define __ARCH__SPARC64__FRAME_H__

/**********************************************************
* Note: All SPARC64 frames MUST be 16 byte aligned!       *
* See INC_ARCH(registers.h) for sizes of register fields. *
**********************************************************/

#ifndef ASSEMBLY

#include <kdb/console.h>

/**
 *  Stack frame for a register window.
 */
class window_frame_t {
public:
    word_t   l0; /*-0---*/
    word_t   l1; /* 8   */
    word_t   l2; /*-16  */ 
    word_t   l3; /* 24  */
    word_t   l4; /*-32--*/
    word_t   l5; /* 40  */
    word_t   l6; /*-48  */
    word_t   l7; /* 56  */
    word_t   i0; /*-64--*/
    word_t   i1; /* 72  */
    word_t   i2; /*-80  */
    word_t   i3; /* 88  */
    word_t   i4; /*-96--*/
    word_t   i5; /* 104 */
    word_t   i6; /*-112 */
    word_t   i7; /* 120 */

public:

    /* Printing */

    void print(void);

}; // window_frame_t

INLINE void
window_frame_t::print(void)
{
    printf("Window Frame:\n");
    printf("\tl0 0x%lx\nl1 0x%lx\n\tl2 0x%lx\n\tl3 0x%lx\n",
	   l0, l1, l2, l3);
    printf("\tl4 0x%lx\n\tl5 0x%lx\n\tl6 0x%lx\n\tl7 0x%lx\n",
	   l4, l5, l6, l7);
    printf("\ti0 0x%lx\n\ti1 0x%lx\n\ti2 0x%lx\n\ti3 0x%lx\n",
	   i0, i1, i2, i3);
    printf("\ti4 0x%lx\n\ti5 0x%lx\n\ti6 0x%lx\n\ti7 0x%lx\n",
	   i4, i5, i6, i7);

} // window_frame_t::print()

/**
 *  Stack frame for a general trap.
 */
class trap_frame_t {
public:
    window_frame_t   window;     /* 0   */
    word_t   args[8];            /* 128 */
    word_t   g1;                 /* 192 */
    word_t   g2;                 /* 200 */
    word_t   g3;                 /* 208 */
    word_t   g4;                 /* 216 */
    word_t   g5;                 /* 224 */
    word_t   o0;                 /* 232 */
    word_t   o1;                 /* 240 */
    word_t   o2;                 /* 248 */
    word_t   o3;                 /* 256 */
    word_t   o4;                 /* 264 */
    word_t   o5;                 /* 272 */
    word_t   o6;                 /* 280 */
    word_t   o7;                 /* 288 */
    word_t   i6;                 /* 296 */
    word_t   i7;                 /* 304 */
    word_t   unused;             /* 312 */

public:

    /* Printing */

    void print(void);

}; // trap_frame_t

INLINE void
trap_frame_t::print(void)
{
    printf("Trap Frame:\n");
    printf("\tg1 0x%lx\n", g1);
    printf("\tg2 0x%lx\n\tg3 0x%lx\n\tg4 0x%lx\n\tg5 0x%lx\n",
	   g2, g3, g4, g5);
    printf("\to0 0x%lx\n\to1 0x%lx\n\to2 0x%lx\n\to3 0x%lx\n",
	   o0, o1, o2, o3);
    printf("\to4 0x%lx\n\to5 0x%lx\n\to6 0x%lx\n\to7 0x%lx\n",
	   o4, o5, o6, o7);
    printf("\ti6 0x%lx\n\ti7 0x%lx\n",
	   i6, i7);

} // trap_frame_t::print()

/**
 * frame used by thread switch and notify
 */
class switch_frame_t {
public:
    window_frame_t window;  /* 0   */
    word_t args[6];         /* 128 */
    word_t o7;              /* 176 */
    word_t i6;              /* 184 */
    word_t i7;              /* 192 */
    word_t o0;              /* 200 */
    word_t o1;              /* 208 */
    word_t o2;              /* 216 */
    word_t y;               /* 224 */
    word_t unused;          /* 232 */
};

extern "C" void sparc64_do_notify();

#endif /* !ASSEMBLY */

#endif /* !__ARCH__SPARC64__FRAME_H__ */
