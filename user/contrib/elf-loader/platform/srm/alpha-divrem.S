/*	$NetBSD: divrem.m4,v 1.5 1996/10/17 04:26:25 cgd Exp $	*/

/*
 * Copyright (c) 1994, 1995 Carnegie-Mellon University.
 * All rights reserved.
 *
 * Author: Chris G. Demetriou
 * 
 * Permission to use, copy, modify and distribute this software and
 * its documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS" 
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND 
 * FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */

#include <l4/alpha/pal.h>
	
/*
 * Division and remainder.
 *
 * The use of m4 is modeled after the sparc code, but the algorithm is
 * simple binary long division.
 *
 * Note that the loops could probably benefit from unrolling.
 */

/*
 * M4 Parameters
 * __divqu		name of function to generate
 * div		div=div: t10 / t11 -> t12; div=rem: t10 % t11 -> t12
 * false		false=true: signed; false=false: unsigned
 * 64	total number of bits
 */
	
/*
 * LEAF
 *      Declare a global leaf function.
 *      A leaf function does not call other functions AND does not
 *      use any register that is callee-saved AND does not modify
 *      the stack pointer.
 */
#define LEAF(_name_,_n_args_)                                   \
        .globl  _name_;                                         \
        .ent    _name_ 0;                                       \
_name_:;                                                        \
        .frame  sp,0,ra

/*
 * END
 *      Function delimiter
 */
#define END(_name_)                                             \
	.end    _name_

#define ra $26
#define sp $30
#define a0 $16
#define t0 $1
#define t1 $2
#define t2 $3
#define t3 $4		
#define t9 $23	
#define t10 $24	
#define t11 $25
#define t12 $27
#define zero $31

LEAF(__divqu, 0)					/* XXX */
	lda	sp, -64(sp)
	stq	t0, 0(sp)
	stq	t1, 8(sp)
	stq	t2, 16(sp)
	stq	t3, 24(sp)

	stq	t10, 40(sp)
	stq	t11, 48(sp)
	mov	zero, t12			/* Initialize result to zero */




	/* kill the special cases. */
	beq	t11, L0dotrap			/* division by zero! */

	cmpult	t10, t11, t2			/* t10 < t11? */
	/* t12 is already zero, from above.  t10 is untouched. */
	bne	t2, L0ret_result

	cmpeq	t10, t11, t2			/* t10 == t11? */
	cmovne	t2, 1, t12
	cmovne	t2, zero, t10
	bne	t2, L0ret_result

	/*
	 * Find out how many bits of zeros are at the beginning of the divisor.
	 */
L0Bbits:
	ldiq	t3, 1				/* t1 = 0; t0 = 1<<64-1 */
	mov	zero, t1
	sll	t3, 64-1, t0
L0Bloop:
	and	t11, t0, t2			/* if bit in t11 is set, done. */
	bne	t2, L0Abits
	addq	t1, 1, t1				/* increment t1,  bit */
	srl	t0, 1, t0
	cmplt	t1, 64-1, t2		/* if t1 leaves one bit, done. */
	bne	t2, L0Bloop

L0Abits:
	beq	t1, L0dodiv			/* If t1 = 0, divide now.  */
	ldiq	t3, 1				/* t0 = 1<<64-1 */
	sll	t3, 64-1, t0

L0Aloop:
	and	t10, t0, t2			/* if bit in t10 is set, done. */
	bne	t2, L0dodiv
	subq	t1, 1, t1				/* decrement t1,  bit */
	srl     t0, 1, t0 
	bne	t1, L0Aloop			/* If t1 != 0, loop again */

L0dodiv:
	sll	t11, t1, t11				/* t11 <<= i */
	ldiq	t3, 1
	sll	t3, t1, t0

L0divloop:
	cmpult	t10, t11, t2
	or	t12, t0, t3
	cmoveq	t2, t3, t12
	subq	t10, t11, t3
	cmoveq	t2, t3, t10
	srl	t0, 1, t0	
	srl	t11, 1, t11
	beq	t10, L0ret_result
	bne	t0, L0divloop

L0ret_result:



	ldq	t0, 0(sp)
	ldq	t1, 8(sp)
	ldq	t2, 16(sp)
	ldq	t3, 24(sp)

	ldq	t10, 40(sp)
	ldq	t11, 48(sp)
	lda	sp, 64(sp)
	ret	zero, (t9), 1

L0dotrap:
	ldiq	a0, -2			/* This is the signal to SIGFPE! */
	call_pal PAL_gentrap

	br	zero, L0ret_result

END(__divqu)
/*	$NetBSD: divrem.m4,v 1.5 1996/10/17 04:26:25 cgd Exp $	*/

/*
 * Copyright (c) 1994, 1995 Carnegie-Mellon University.
 * All rights reserved.
 *
 * Author: Chris G. Demetriou
 * 
 * Permission to use, copy, modify and distribute this software and
 * its documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS" 
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND 
 * FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */

/*
 * Division and remainder.
 *
 * The use of m4 is modeled after the sparc code, but the algorithm is
 * simple binary long division.
 *
 * Note that the loops could probably benefit from unrolling.
 */

/*
 * M4 Parameters
 * __divlu		name of function to generate
 * div		div=div: t10 / t11 -> t12; div=rem: t10 % t11 -> t12
 * false		false=true: signed; false=false: unsigned
 * 32	total number of bits
 */

LEAF(__divlu, 0)					/* XXX */
	lda	sp, -64(sp)
	stq	t0, 0(sp)
	stq	t1, 8(sp)
	stq	t2, 16(sp)
	stq	t3, 24(sp)

	stq	t10, 40(sp)
	stq	t11, 48(sp)
	mov	zero, t12			/* Initialize result to zero */



	/*
	 * Clear the top 32 bits of each operand, as they may
	 * sign extension (if negated above), or random junk.
	 */
	zap	t10, 0xf0, t10
	zap	t11, 0xf0, t11


	/* kill the special cases. */
	beq	t11, L1dotrap			/* division by zero! */

	cmpult	t10, t11, t2			/* t10 < t11? */
	/* t12 is already zero, from above.  t10 is untouched. */
	bne	t2, L1ret_result

	cmpeq	t10, t11, t2			/* t10 == t11? */
	cmovne	t2, 1, t12
	cmovne	t2, zero, t10
	bne	t2, L1ret_result

	/*
	 * Find out how many bits of zeros are at the beginning of the divisor.
	 */
L1Bbits:
	ldiq	t3, 1				/* t1 = 0; t0 = 1<<32-1 */
	mov	zero, t1
	sll	t3, 32-1, t0
L1Bloop:
	and	t11, t0, t2			/* if bit in t11 is set, done. */
	bne	t2, L1Abits
	addq	t1, 1, t1				/* increment t1,  bit */
	srl	t0, 1, t0
	cmplt	t1, 32-1, t2		/* if t1 leaves one bit, done. */
	bne	t2, L1Bloop

L1Abits:
	beq	t1, L1dodiv			/* If t1 = 0, divide now.  */
	ldiq	t3, 1				/* t0 = 1<<32-1 */
	sll	t3, 32-1, t0

L1Aloop:
	and	t10, t0, t2			/* if bit in t10 is set, done. */
	bne	t2, L1dodiv
	subq	t1, 1, t1				/* decrement t1,  bit */
	srl     t0, 1, t0 
	bne	t1, L1Aloop			/* If t1 != 0, loop again */

L1dodiv:
	sll	t11, t1, t11				/* t11 <<= i */
	ldiq	t3, 1
	sll	t3, t1, t0

L1divloop:
	cmpult	t10, t11, t2
	or	t12, t0, t3
	cmoveq	t2, t3, t12
	subq	t10, t11, t3
	cmoveq	t2, t3, t10
	srl	t0, 1, t0	
	srl	t11, 1, t11
	beq	t10, L1ret_result
	bne	t0, L1divloop

L1ret_result:



	ldq	t0, 0(sp)
	ldq	t1, 8(sp)
	ldq	t2, 16(sp)
	ldq	t3, 24(sp)

	ldq	t10, 40(sp)
	ldq	t11, 48(sp)
	lda	sp, 64(sp)
	ret	zero, (t9), 1

L1dotrap:
	ldiq	a0, -2			/* This is the signal to SIGFPE! */
	call_pal PAL_gentrap

	br	zero, L1ret_result

END(__divlu)
/*	$NetBSD: divrem.m4,v 1.5 1996/10/17 04:26:25 cgd Exp $	*/

/*
 * Copyright (c) 1994, 1995 Carnegie-Mellon University.
 * All rights reserved.
 *
 * Author: Chris G. Demetriou
 * 
 * Permission to use, copy, modify and distribute this software and
 * its documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS" 
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND 
 * FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */

/*
 * Division and remainder.
 *
 * The use of m4 is modeled after the sparc code, but the algorithm is
 * simple binary long division.
 *
 * Note that the loops could probably benefit from unrolling.
 */

/*
 * M4 Parameters
 * __remlu		name of function to generate
 * rem		rem=div: t10 / t11 -> t12; rem=rem: t10 % t11 -> t12
 * false		false=true: signed; false=false: unsigned
 * 32	total number of bits
 */

LEAF(__remlu, 0)					/* XXX */
	lda	sp, -64(sp)
	stq	t0, 0(sp)
	stq	t1, 8(sp)
	stq	t2, 16(sp)
	stq	t3, 24(sp)

	stq	t10, 40(sp)
	stq	t11, 48(sp)
	mov	zero, t12			/* Initialize result to zero */



	/*
	 * Clear the top 32 bits of each operand, as they may
	 * sign extension (if negated above), or random junk.
	 */
	zap	t10, 0xf0, t10
	zap	t11, 0xf0, t11


	/* kill the special cases. */
	beq	t11, L2dotrap			/* division by zero! */

	cmpult	t10, t11, t2			/* t10 < t11? */
	/* t12 is already zero, from above.  t10 is untouched. */
	bne	t2, L2ret_result

	cmpeq	t10, t11, t2			/* t10 == t11? */
	cmovne	t2, 1, t12
	cmovne	t2, zero, t10
	bne	t2, L2ret_result

	/*
	 * Find out how many bits of zeros are at the beginning of the divisor.
	 */
L2Bbits:
	ldiq	t3, 1				/* t1 = 0; t0 = 1<<32-1 */
	mov	zero, t1
	sll	t3, 32-1, t0
L2Bloop:
	and	t11, t0, t2			/* if bit in t11 is set, done. */
	bne	t2, L2Abits
	addq	t1, 1, t1				/* increment t1,  bit */
	srl	t0, 1, t0
	cmplt	t1, 32-1, t2		/* if t1 leaves one bit, done. */
	bne	t2, L2Bloop

L2Abits:
	beq	t1, L2dodiv			/* If t1 = 0, divide now.  */
	ldiq	t3, 1				/* t0 = 1<<32-1 */
	sll	t3, 32-1, t0

L2Aloop:
	and	t10, t0, t2			/* if bit in t10 is set, done. */
	bne	t2, L2dodiv
	subq	t1, 1, t1				/* decrement t1,  bit */
	srl     t0, 1, t0 
	bne	t1, L2Aloop			/* If t1 != 0, loop again */

L2dodiv:
	sll	t11, t1, t11				/* t11 <<= i */
	ldiq	t3, 1
	sll	t3, t1, t0

L2divloop:
	cmpult	t10, t11, t2
	or	t12, t0, t3
	cmoveq	t2, t3, t12
	subq	t10, t11, t3
	cmoveq	t2, t3, t10
	srl	t0, 1, t0	
	srl	t11, 1, t11
	beq	t10, L2ret_result
	bne	t0, L2divloop

L2ret_result:
	mov	t10, t12



	ldq	t0, 0(sp)
	ldq	t1, 8(sp)
	ldq	t2, 16(sp)
	ldq	t3, 24(sp)

	ldq	t10, 40(sp)
	ldq	t11, 48(sp)
	lda	sp, 64(sp)
	ret	zero, (t9), 1

L2dotrap:
	ldiq	a0, -2			/* This is the signal to SIGFPE! */
	call_pal PAL_gentrap
	mov	zero, t10			/* so that zero will be returned */

	br	zero, L2ret_result

END(__remlu)
/*	$NetBSD: divrem.m4,v 1.5 1996/10/17 04:26:25 cgd Exp $	*/

/*
 * Copyright (c) 1994, 1995 Carnegie-Mellon University.
 * All rights reserved.
 *
 * Author: Chris G. Demetriou
 * 
 * Permission to use, copy, modify and distribute this software and
 * its documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS" 
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND 
 * FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */

/*
 * Division and remainder.
 *
 * The use of m4 is modeled after the sparc code, but the algorithm is
 * simple binary long division.
 *
 * Note that the loops could probably benefit from unrolling.
 */

/*
 * M4 Parameters
 * __remqu		name of function to generate
 * rem		rem=div: t10 / t11 -> t12; rem=rem: t10 % t11 -> t12
 * false		false=true: signed; false=false: unsigned
 * 64	total number of bits
 */


LEAF(__remqu, 0)					/* XXX */
	lda	sp, -64(sp)
	stq	t0, 0(sp)
	stq	t1, 8(sp)
	stq	t2, 16(sp)
	stq	t3, 24(sp)

	stq	t10, 40(sp)
	stq	t11, 48(sp)
	mov	zero, t12			/* Initialize result to zero */




	/* kill the special cases. */
	beq	t11, L3dotrap			/* division by zero! */

	cmpult	t10, t11, t2			/* t10 < t11? */
	/* t12 is already zero, from above.  t10 is untouched. */
	bne	t2, L3ret_result

	cmpeq	t10, t11, t2			/* t10 == t11? */
	cmovne	t2, 1, t12
	cmovne	t2, zero, t10
	bne	t2, L3ret_result

	/*
	 * Find out how many bits of zeros are at the beginning of the divisor.
	 */
L3Bbits:
	ldiq	t3, 1				/* t1 = 0; t0 = 1<<64-1 */
	mov	zero, t1
	sll	t3, 64-1, t0
L3Bloop:
	and	t11, t0, t2			/* if bit in t11 is set, done. */
	bne	t2, L3Abits
	addq	t1, 1, t1				/* increment t1,  bit */
	srl	t0, 1, t0
	cmplt	t1, 64-1, t2		/* if t1 leaves one bit, done. */
	bne	t2, L3Bloop

L3Abits:
	beq	t1, L3dodiv			/* If t1 = 0, divide now.  */
	ldiq	t3, 1				/* t0 = 1<<64-1 */
	sll	t3, 64-1, t0

L3Aloop:
	and	t10, t0, t2			/* if bit in t10 is set, done. */
	bne	t2, L3dodiv
	subq	t1, 1, t1				/* decrement t1,  bit */
	srl     t0, 1, t0 
	bne	t1, L3Aloop			/* If t1 != 0, loop again */

L3dodiv:
	sll	t11, t1, t11				/* t11 <<= i */
	ldiq	t3, 1
	sll	t3, t1, t0

L3divloop:
	cmpult	t10, t11, t2
	or	t12, t0, t3
	cmoveq	t2, t3, t12
	subq	t10, t11, t3
	cmoveq	t2, t3, t10
	srl	t0, 1, t0	
	srl	t11, 1, t11
	beq	t10, L3ret_result
	bne	t0, L3divloop

L3ret_result:
	mov	t10, t12



	ldq	t0, 0(sp)
	ldq	t1, 8(sp)
	ldq	t2, 16(sp)
	ldq	t3, 24(sp)

	ldq	t10, 40(sp)
	ldq	t11, 48(sp)
	lda	sp, 64(sp)
	ret	zero, (t9), 1

L3dotrap:
	ldiq	a0, -2			/* This is the signal to SIGFPE! */
	call_pal PAL_gentrap
	mov	zero, t10			/* so that zero will be returned */

	br	zero, L3ret_result

END(__remqu)
