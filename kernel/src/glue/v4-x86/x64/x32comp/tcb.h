/*********************************************************************
 *                
 * Copyright (C) 2002-2007,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/x64/x32comp/tcb.h
 * Description:   TCB for AMD64 compatibility mode
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
 * $Id: tcb.h,v 1.2 2006/10/21 00:15:21 reichelt Exp $
 *                
 ********************************************************************/


#ifndef __GLUE__V4_X86__X64__X32COMP__TCB_H__
#define __GLUE__V4_X86__X64__X32COMP__TCB_H__

#ifndef __GLUE_V4_X86__X64__TCB_H__
#error not for stand-alone inclusion
#endif

INLINE void tcb_t::set_space(space_t * space)
{
    this->space = space;
    this->pdir_cache = space ? (word_t)space->get_top_pdir_phys(get_cpu()) : NULL;
    if (space && space->is_compatibility_mode())
	resource_bits += COMPATIBILITY_MODE;
    
}


/* Copied from glue/v4-amd64/tcb.h */

/**
 * sets the CPU
 * @param cpuid
 */
INLINE void tcb_t::set_cpu(cpuid_t cpu)
{
    this->cpu = cpu;

    /* only update UTCB if there is one */
    utcb_t *utcb = get_utcb();
    if (utcb)
    {
	if (EXPECT_FALSE(resource_bits.have_resource(COMPATIBILITY_MODE)))
	    utcb->x32.processor_no = cpu;
	else
	    utcb->x64.processor_no = cpu;
    }

    /* update the pdir cache on migration */
    if (space && !this->pdir_cache) {
	this->pdir_cache = (word_t)space->get_top_pdir_phys(cpu);
	ASSERT(this->pdir_cache);
    }
}


/**********************************************************************
 * 
 *            utcb state manipulation
 *
 **********************************************************************/

INLINE void tcb_t::set_utcb_location(word_t utcb_location)
{ 
    /* utcb address points to mr0 at offset 0x200 */
    myself_local.set_raw (utcb_location + 0x200);
}

INLINE word_t tcb_t::get_utcb_location()
{
    /*
     * srXXX: This is rather ugly!
     * To create a 32-bit thread in a new address space, an application
     * must call ThreadControl without a pager, then SpaceControl with
     * appropriate flags, then ThreadControl with a pager.
     * The ThreadControl implementation will call space_t::allocate_utcb,
     * which is implemented in glue, and the AMD64 implementation will
     * call this function. Therefore we update the resource bits here.
     * To make this a little less ugly, it should be done in
     * allocate_utcb, but unfortunately the resource bits are private.
     * The correct way to implement this would be to maintain a list of
     * TCBs in every address space, and update all of these in
     * space_control.
     */
    if (get_space() && get_space()->is_compatibility_mode())
	resource_bits += COMPATIBILITY_MODE;
    return myself_local.get_raw() - 0x200;
}


/**
 * read value of message register
 * @param index number of message register
 */
INLINE word_t tcb_t::get_mr(word_t index)
{
    if (EXPECT_FALSE(resource_bits.have_resource(COMPATIBILITY_MODE)))
    {
	x32::word_t result = get_utcb()->x32.mr[index];
	/* Sign-extend the label (mainly for page fault protocol),
	   but do not sign-extend other MRs since that would mean
	   that page faults beyond 2G cannot be handled. */
	if (index == 0)
	    return (s32_t) result;
	else
	    return result;
    }
    else
	return get_utcb()->x64.mr[index];
}

/*
 * set the value of a message register
 * @param index number of message register
 * @param value value to set
 */
INLINE void tcb_t::set_mr(word_t index, word_t value)
{
    if (EXPECT_FALSE(resource_bits.have_resource(COMPATIBILITY_MODE)))
	get_utcb()->x32.mr[index] = value;
    else
	get_utcb()->x64.mr[index] = value;
}

/**
  * copies a set of message registers from one UTCB to another
  * @param dest destination TCB
  * @param start MR start index
  * @param count number of MRs to be copied
  */
INLINE void tcb_t::copy_mrs(tcb_t *dest, word_t start, word_t count)
{
    ASSERT(start + count <= IPC_NUM_MR);
    ASSERT(count > 0);

    utcb_t *this_utcb = get_utcb();
    utcb_t *dest_utcb = dest->get_utcb();

    /* Optimized copy loops for 32/32 and 64/64 transfers */
    if (EXPECT_FALSE(resource_bits.have_resource(COMPATIBILITY_MODE)))
    {
	if (EXPECT_FALSE(dest->resource_bits.have_resource(COMPATIBILITY_MODE)))
	{
	    word_t dummy;
	    __asm__ __volatile__ (
		"repnz movsl (%%rsi), (%%rdi)\n"
		: /* output */
		"=S"(dummy), "=D"(dummy), "=c"(dummy)
		: /* input */
		"c"(count), "S"(&this_utcb->x32.mr[start]),
		"D"(&dest_utcb->x32.mr[start]));
	}
	else
	{
	    if (start == 0)
	    {
		/* Sign-extend the label (for page fault protocol). */
		dest_utcb->x64.mr[0] = (s32_t) this_utcb->x32.mr[0];
		count--;
		start++;
	    }
	    for (; count > 0; count--, start++)
		dest_utcb->x64.mr[start] = this_utcb->x32.mr[start];
	}
    }
    else
    {
	if (EXPECT_FALSE(dest->resource_bits.have_resource(COMPATIBILITY_MODE)))
	{
	    for (; count > 0; count--, start++)
		dest_utcb->x32.mr[start] = this_utcb->x64.mr[start];
	}
	else
	{
	    word_t dummy;
	    __asm__ __volatile__ (
		"repnz movsq (%%rsi), (%%rdi)\n"
		: /* output */
		"=S"(dummy), "=D"(dummy), "=c"(dummy)
		: /* input */
		"c"(count), "S"(&this_utcb->x64.mr[start]),
		"D"(&dest_utcb->x64.mr[start]));
	}
    }
}

/**
 * read value of buffer register
 * @param index number of buffer register
 */
INLINE word_t tcb_t::get_br(word_t index)
{
    if (EXPECT_FALSE(resource_bits.have_resource(COMPATIBILITY_MODE)))
	return get_utcb()->x32.br[32U-index];
    else
	return get_utcb()->x64.br[32U-index];
}


/**
 * set the value of a buffer register
 * @param index number of buffer register
 * @param value value to set
 */
INLINE void tcb_t::set_br(word_t index, word_t value)
{
    if (EXPECT_FALSE(resource_bits.have_resource(COMPATIBILITY_MODE)))
	get_utcb()->x32.br[32U-index] = value;
    else
	get_utcb()->x64.br[32U-index] = value;
}


#endif /* !__GLUE__V4_X86__X64__X32COMP__TCB_H__ */
