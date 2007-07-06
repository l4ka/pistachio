/*********************************************************************
 *                
 * Copyright (C) 2003, University of New South Wales
 *                
 * File path:    glue/v4-sparc64/window_trap.S
 * Description:  Inline trap handlers for window spills/fills/cleans.
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
 * $Id: window_trap.S,v 1.9 2004/07/01 04:05:43 philipd Exp $
 *                
 ********************************************************************/

#include INC_ARCH(asi.h)
#include INC_ARCH(asm.h)
#include INC_ARCH(ttable.h)

#include <asmsyms.h>

/**
 * FIND_UTCB_SAVE_AREA
 * Using %g2 as a pointer to the bottom of the UTCB save area, find the location
 * in the UTCB that must be used to spill / fill the current window.
 */
#define FIND_UTCB_SAVE_AREA(reg) \
	rdpr	%cwp, reg;					\
	mulx	reg, WINDOW_FRAME_SIZE, reg;			\
	add	reg, %g2, reg

/**
 *  WINDOW_CLEAN
 *  Window clean handler.
 *  Constraints: 32 Instructions or less.	
 */
#define WINDOW_CLEAN                                             \
	rdpr	%cleanwin, %l0;                                  \
	inc	%l0;                                             \
	wrpr	%l0, %cleanwin;                                  \
	clr	%l0;                                             \
	clr	%l1;                                             \
	clr	%l2;                                             \
	clr	%l3;                                             \
	clr	%l4;                                             \
	clr	%l5;                                             \
	clr	%l6;                                             \
	clr	%l7;                                             \
	clr	%o0;                                             \
	clr	%o1;                                             \
	clr	%o2;                                             \
	clr	%o3;                                             \
	clr	%o4;                                             \
	clr	%o5;                                             \
	clr	%o6;                                             \
	clr	%o7;                                             \
	retry

/* End WINDOW_CLEAN */

/**
 * WINDOW_SPILL
 * Spill the local and in registers of a window to a given frame
 */
#define WINDOW_SPILL(frame)					\
	stx	%l0, [frame + WINDOW_FRAME_L0];			\
	stx	%l1, [frame + WINDOW_FRAME_L1];			\
	stx	%l2, [frame + WINDOW_FRAME_L2];			\
	stx	%l3, [frame + WINDOW_FRAME_L3];			\
	stx	%l4, [frame + WINDOW_FRAME_L4];			\
	stx	%l5, [frame + WINDOW_FRAME_L5];			\
	stx	%l6, [frame + WINDOW_FRAME_L6];			\
	stx	%l7, [frame + WINDOW_FRAME_L7];			\
	stx	%i0, [frame + WINDOW_FRAME_I0];			\
	stx	%i1, [frame + WINDOW_FRAME_I1];			\
	stx	%i2, [frame + WINDOW_FRAME_I2];			\
	stx	%i3, [frame + WINDOW_FRAME_I3];			\
	stx	%i4, [frame + WINDOW_FRAME_I4];			\
	stx	%i5, [frame + WINDOW_FRAME_I5];			\
	stx	%i6, [frame + WINDOW_FRAME_I6];			\
	stx	%i7, [frame + WINDOW_FRAME_I7]
	
/**
 * WINDOW_FILL
 * Fill the local and in registers of a window from a given frame
 */
#define WINDOW_FILL(frame)					\
	ldx	[frame + WINDOW_FRAME_L0], %l0;			\
	ldx	[frame + WINDOW_FRAME_L1], %l1;			\
	ldx	[frame + WINDOW_FRAME_L2], %l2;			\
	ldx	[frame + WINDOW_FRAME_L3], %l3;			\
	ldx	[frame + WINDOW_FRAME_L4], %l4;			\
	ldx	[frame + WINDOW_FRAME_L5], %l5;			\
	ldx	[frame + WINDOW_FRAME_L6], %l6;			\
	ldx	[frame + WINDOW_FRAME_L7], %l7;			\
	ldx	[frame + WINDOW_FRAME_I0], %i0;			\
	ldx	[frame + WINDOW_FRAME_I1], %i1;			\
	ldx	[frame + WINDOW_FRAME_I2], %i2;			\
	ldx	[frame + WINDOW_FRAME_I3], %i3;			\
	ldx	[frame + WINDOW_FRAME_I4], %i4;			\
	ldx	[frame + WINDOW_FRAME_I5], %i5;			\
	ldx	[frame + WINDOW_FRAME_I6], %i6;			\
	ldx	[frame + WINDOW_FRAME_I7], %i7

/**
 * WINDOW_SPILL_ASI
 * Spill the local and in registers of a window to a given frame,
 * which is in the address space %asi
 */
#define WINDOW_SPILL_ASI(frame)					\
	stxa	%l0, [frame + WINDOW_FRAME_L0] %asi;		\
	stxa	%l1, [frame + WINDOW_FRAME_L1] %asi;		\
	stxa	%l2, [frame + WINDOW_FRAME_L2] %asi;		\
	stxa	%l3, [frame + WINDOW_FRAME_L3] %asi;		\
	stxa	%l4, [frame + WINDOW_FRAME_L4] %asi;		\
	stxa	%l5, [frame + WINDOW_FRAME_L5] %asi;		\
	stxa	%l6, [frame + WINDOW_FRAME_L6] %asi;		\
	stxa	%l7, [frame + WINDOW_FRAME_L7] %asi;		\
	stxa	%i0, [frame + WINDOW_FRAME_I0] %asi;		\
	stxa	%i1, [frame + WINDOW_FRAME_I1] %asi;		\
	stxa	%i2, [frame + WINDOW_FRAME_I2] %asi;		\
	stxa	%i3, [frame + WINDOW_FRAME_I3] %asi;		\
	stxa	%i4, [frame + WINDOW_FRAME_I4] %asi;		\
	stxa	%i5, [frame + WINDOW_FRAME_I5] %asi;		\
	stxa	%i6, [frame + WINDOW_FRAME_I6] %asi;		\
	stxa	%i7, [frame + WINDOW_FRAME_I7] %asi
	
/**
 * WINDOW_FILL_ASI
 * Fill the local and in registers of a window from a given frame,
 * which is in the address space %asi
 */
#define WINDOW_FILL_ASI(frame)					\
	ldxa	[frame + WINDOW_FRAME_L0] %asi, %l0;		\
	ldxa	[frame + WINDOW_FRAME_L1] %asi, %l1;		\
	ldxa	[frame + WINDOW_FRAME_L2] %asi, %l2;		\
	ldxa	[frame + WINDOW_FRAME_L3] %asi, %l3;		\
	ldxa	[frame + WINDOW_FRAME_L4] %asi, %l4;		\
	ldxa	[frame + WINDOW_FRAME_L5] %asi, %l5;		\
	ldxa	[frame + WINDOW_FRAME_L6] %asi, %l6;		\
	ldxa	[frame + WINDOW_FRAME_L7] %asi, %l7;		\
	ldxa	[frame + WINDOW_FRAME_I0] %asi, %i0;		\
	ldxa	[frame + WINDOW_FRAME_I1] %asi, %i1;		\
	ldxa	[frame + WINDOW_FRAME_I2] %asi, %i2;		\
	ldxa	[frame + WINDOW_FRAME_I3] %asi, %i3;		\
	ldxa	[frame + WINDOW_FRAME_I4] %asi, %i4;		\
	ldxa	[frame + WINDOW_FRAME_I5] %asi, %i5;		\
	ldxa	[frame + WINDOW_FRAME_I6] %asi, %i6;		\
	ldxa	[frame + WINDOW_FRAME_I7] %asi, %i7

/**
 *  WINDOW_SPILL_KERNEL
 *  Kernel spill handler.
 *  Constraints: 32 Instructions or less.
 */
#define WINDOW_SPILL_KERNEL			   \
	WINDOW_SPILL(%sp + STACK_BIAS_64BIT);	   \
	saved;                                     \
	retry

/* End WINDOW_SPILL_KERNEL */
	
/**
 *  WINDOW_FILL_KERNEL
 *  Kernel fill handler.
 *  Constraints: 32 Instructions or less.
 */	
#define WINDOW_FILL_KERNEL                         \
	WINDOW_FILL(%sp + STACK_BIAS_64BIT);	   \
	restored;                                  \
	retry

/* End WINDOW_FILL_KERNEL */

/**
 *  WINDOW_SPILL_KERNEL_UTCB
 *  Kernel spill handler for user windows.
 *  Constraints: 32 Instructions or less.
 */
#define WINDOW_SPILL_KERNEL_UTCB		   \
	FIND_UTCB_SAVE_AREA(%g3);		   \
	WINDOW_SPILL(%g3);			   \
	add     %g1, 1, %g1;                       \
	saved;                                     \
	retry

/* End WINDOW_SPILL_KERNEL */
	
/**
 *  WINDOW_FILL_KERNEL_UTCB
 *  Kernel fill handler for user windows.
 *  Constraints: 32 Instructions or less.
 */	
#define WINDOW_FILL_KERNEL_UTCB                    \
	FIND_UTCB_SAVE_AREA(%g3);		   \
	WINDOW_FILL(%g3);			   \
	sub     %g1, 1, %g1;                       \
	restored;                                  \
	retry

/* End WINDOW_FILL_KERNEL */

/**
 *  WINDOW_SPILL_USER_NORMAL_ABI64
 *  User spill handler (normal) for standard 64bit ABI.
 *  Constraints: 32 Instructions or less.
 */
#define WINDOW_SPILL_USER_NORMAL_ABI64                                     \
	wr	%g0, ASI_AIUP, %asi;                                       \
	WINDOW_SPILL_ASI(%sp + STACK_BIAS_64BIT);			   \
	saved;                                                             \
	retry

/* End WINDOW_SPILL_USER_NORMAL_ABI64 */
	
/**
 *  WINDOW_FILL_USER_NORMAL_ABI64
 *  User fill handler (normal, from user stack) for standard 64bit ABI.
 *  Constraints: 32 Instructions or less.
 */	
#define WINDOW_FILL_USER_NORMAL_ABI64                                      \
	wr	%g0, ASI_AIUP, %asi;                                       \
	WINDOW_FILL_ASI(%sp + STACK_BIAS_64BIT);			   \
	restored;                                                          \
	retry

/**
 *  WINDOW_SPILL_USER_OTHER_ABI64
 *  User spill handler (other, from utcb save area) for standard 64bit ABI.
 *  Constraints: 32 Instructions or less.
 */
/* This trap is supposed to be spilling a register window, but it isn't really
 * filled yet; it's marked as an "other" window because it was saved to the UTCB
 * on a trap or thread switch. To avoid running out of space in the UTCB window
 * save area, the saved window must be moved to the user's stack, so this
 * handler fills from the UTCB and then spills to the stack. */
#define WINDOW_SPILL_USER_OTHER_ABI64					\
	FIND_UTCB_SAVE_AREA(%g3);					\
	WINDOW_FILL(%g3);						\
	/* need to find the correct %sp to save to. If this is the	\
	 * last other window (ie %otherwin == 1), then it's in %sp	\
	 * and we can just jump to the normal spill handler. Else	\
	 * it's in %fp of the next saved window in the UTCB. */ 	\
	rdpr	%otherwin, %g4;						\
	xorcc	%g4, 1, %g0;						\
	beq	%xcc, ttable_O_spill_0_normal;				\
	 ldx     [%g3 + WINDOW_FRAME_I7], %i7;				\
	rdpr	%cwp, %g3;						\
	inc	%g3;							\
	and	%g3, (NWINDOWS-1), %g3;					\
	mulx	%g3, WINDOW_FRAME_SIZE, %g3;				\
	add	%g3, %g2, %g3;						\
	ba,pt	%xcc, ttable_O_spill_0_normal;				\
	 ldx	[%g3 + WINDOW_FRAME_I6], %sp

/**
 *  WINDOW_FILL_USER_OTHER_ABI64
 *  User fill handler (other, from utcb save area) for standard 64bit ABI.
 *  Constraints: 32 Instructions or less.
 */	
#define WINDOW_FILL_USER_OTHER_ABI64			\
	FIND_UTCB_SAVE_AREA(%g3);			\
	WINDOW_FILL(%g3);				\
	restored;					\
	retry

/* End WINDOW_FILL_USER_NORMAL_ABI64 */

/***************************
* Used User (TL = 0) traps *
***************************/

BEGIN_TTABLE_ENTRY(clean_window,O)
	WINDOW_CLEAN

BEGIN_TTABLE_ENTRY(spill_0_normal,O)
	WINDOW_SPILL_USER_NORMAL_ABI64

BEGIN_TTABLE_ENTRY(fill_0_normal,O)
	WINDOW_FILL_USER_NORMAL_ABI64

BEGIN_TTABLE_ENTRY(spill_0_other,O)
	WINDOW_SPILL_USER_OTHER_ABI64

BEGIN_TTABLE_ENTRY(fill_0_other,O)
	WINDOW_FILL_USER_OTHER_ABI64

/*****************************
* Used Kernel (TL > 0) traps *
*****************************/

BEGIN_TTABLE_ENTRY(clean_window,X)
	rdpr	%cleanwin, %l0
	inc	%l0
	wrpr	%l0, %cleanwin
	retry

BEGIN_TTABLE_ENTRY(spill_0_normal,X)
	WINDOW_SPILL_KERNEL

BEGIN_TTABLE_ENTRY(fill_0_normal,X)
	WINDOW_FILL_KERNEL

BEGIN_TTABLE_ENTRY(spill_0_other,X)
	WINDOW_SPILL_KERNEL_UTCB

BEGIN_TTABLE_ENTRY(fill_0_other,X)
	WINDOW_FILL_KERNEL_UTCB

/****************
* Unused traps. *
****************/

UNUSED_TTABLE_ENTRY(spill_1_normal,O)
UNUSED_TTABLE_ENTRY(spill_2_normal,O)
UNUSED_TTABLE_ENTRY(spill_3_normal,O)
UNUSED_TTABLE_ENTRY(spill_4_normal,O)
UNUSED_TTABLE_ENTRY(spill_5_normal,O)
UNUSED_TTABLE_ENTRY(spill_6_normal,O)
UNUSED_TTABLE_ENTRY(spill_7_normal,O)

UNUSED_TTABLE_ENTRY(spill_1_other,O)
UNUSED_TTABLE_ENTRY(spill_2_other,O)
UNUSED_TTABLE_ENTRY(spill_3_other,O)
UNUSED_TTABLE_ENTRY(spill_4_other,O)
UNUSED_TTABLE_ENTRY(spill_5_other,O)
UNUSED_TTABLE_ENTRY(spill_6_other,O)
UNUSED_TTABLE_ENTRY(spill_7_other,O)
	
UNUSED_TTABLE_ENTRY(fill_1_normal,O)
UNUSED_TTABLE_ENTRY(fill_2_normal,O)
UNUSED_TTABLE_ENTRY(fill_3_normal,O)
UNUSED_TTABLE_ENTRY(fill_4_normal,O)
UNUSED_TTABLE_ENTRY(fill_5_normal,O)
UNUSED_TTABLE_ENTRY(fill_6_normal,O)
UNUSED_TTABLE_ENTRY(fill_7_normal,O)

UNUSED_TTABLE_ENTRY(fill_1_other,O)
UNUSED_TTABLE_ENTRY(fill_2_other,O)
UNUSED_TTABLE_ENTRY(fill_3_other,O)
UNUSED_TTABLE_ENTRY(fill_4_other,O)
UNUSED_TTABLE_ENTRY(fill_5_other,O)
UNUSED_TTABLE_ENTRY(fill_6_other,O)
UNUSED_TTABLE_ENTRY(fill_7_other,O)

UNUSED_TTABLE_ENTRY(spill_1_normal,X)
UNUSED_TTABLE_ENTRY(spill_2_normal,X)
UNUSED_TTABLE_ENTRY(spill_3_normal,X)
UNUSED_TTABLE_ENTRY(spill_4_normal,X)
UNUSED_TTABLE_ENTRY(spill_5_normal,X)
UNUSED_TTABLE_ENTRY(spill_6_normal,X)
UNUSED_TTABLE_ENTRY(spill_7_normal,X)

UNUSED_TTABLE_ENTRY(spill_1_other,X)
UNUSED_TTABLE_ENTRY(spill_2_other,X)
UNUSED_TTABLE_ENTRY(spill_3_other,X)
UNUSED_TTABLE_ENTRY(spill_4_other,X)
UNUSED_TTABLE_ENTRY(spill_5_other,X)
UNUSED_TTABLE_ENTRY(spill_6_other,X)
UNUSED_TTABLE_ENTRY(spill_7_other,X)

UNUSED_TTABLE_ENTRY(fill_1_normal,X)
UNUSED_TTABLE_ENTRY(fill_2_normal,X)
UNUSED_TTABLE_ENTRY(fill_3_normal,X)
UNUSED_TTABLE_ENTRY(fill_4_normal,X)
UNUSED_TTABLE_ENTRY(fill_5_normal,X)
UNUSED_TTABLE_ENTRY(fill_6_normal,X)
UNUSED_TTABLE_ENTRY(fill_7_normal,X)

UNUSED_TTABLE_ENTRY(fill_1_other,X)
UNUSED_TTABLE_ENTRY(fill_2_other,X)
UNUSED_TTABLE_ENTRY(fill_3_other,X)
UNUSED_TTABLE_ENTRY(fill_4_other,X)
UNUSED_TTABLE_ENTRY(fill_5_other,X)
UNUSED_TTABLE_ENTRY(fill_6_other,X)
UNUSED_TTABLE_ENTRY(fill_7_other,X)
