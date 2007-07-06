/*********************************************************************
 *                
 * Copyright (C) 2002-2006,  Karlsruhe University
 *                
 * File path:     glue/v4-amd64/ia32/tcb.h
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
#ifndef __GLUE__V4_AMD64__IA32__TCB_H__
#define __GLUE__V4_AMD64__IA32__TCB_H__

#ifndef __GLUE__V4_AMD64__TCB_H__
#error not for stand-alone inclusion
#endif


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
	    utcb->ia32.processor_no = cpu;
	else
	    utcb->amd64.processor_no = cpu;
    }

    /* update the pdir cache on migration */
    if (this->space)
	this->pdir_cache = (word_t) space->get_pml4(get_cpu());
}

/**
 * read value of message register
 * @param index number of message register
 */
INLINE word_t tcb_t::get_mr(word_t index)
{
    if (EXPECT_FALSE(resource_bits.have_resource(COMPATIBILITY_MODE)))
    {
	ia32::word_t result = get_utcb()->ia32.mr[index];
	/* Sign-extend the label (mainly for page fault protocol),
	   but do not sign-extend other MRs since that would mean
	   that page faults beyond 2G cannot be handled. */
	if (index == 0)
	    return (s32_t) result;
	else
	    return result;
    }
    else
	return get_utcb()->amd64.mr[index];
}

/*
 * set the value of a message register
 * @param index number of message register
 * @param value value to set
 */
INLINE void tcb_t::set_mr(word_t index, word_t value)
{
    if (EXPECT_FALSE(resource_bits.have_resource(COMPATIBILITY_MODE)))
	get_utcb()->ia32.mr[index] = value;
    else
	get_utcb()->amd64.mr[index] = value;
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
		"c"(count), "S"(&this_utcb->ia32.mr[start]),
		"D"(&dest_utcb->ia32.mr[start]));
	}
	else
	{
	    if (start == 0)
	    {
		/* Sign-extend the label (for page fault protocol). */
		dest_utcb->amd64.mr[0] = (s32_t) this_utcb->ia32.mr[0];
		count--;
		start++;
	    }
	    for (; count > 0; count--, start++)
		dest_utcb->amd64.mr[start] = this_utcb->ia32.mr[start];
	}
    }
    else
    {
	if (EXPECT_FALSE(dest->resource_bits.have_resource(COMPATIBILITY_MODE)))
	{
	    for (; count > 0; count--, start++)
		dest_utcb->ia32.mr[start] = this_utcb->amd64.mr[start];
	}
	else
	{
	    word_t dummy;
	    __asm__ __volatile__ (
		"repnz movsq (%%rsi), (%%rdi)\n"
		: /* output */
		"=S"(dummy), "=D"(dummy), "=c"(dummy)
		: /* input */
		"c"(count), "S"(&this_utcb->amd64.mr[start]),
		"D"(&dest_utcb->amd64.mr[start]));
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
	return get_utcb()->ia32.br[32U-index];
    else
	return get_utcb()->amd64.br[32U-index];
}


/**
 * set the value of a buffer register
 * @param index number of buffer register
 * @param value value to set
 */
INLINE void tcb_t::set_br(word_t index, word_t value)
{
    if (EXPECT_FALSE(resource_bits.have_resource(COMPATIBILITY_MODE)))
	get_utcb()->ia32.br[32U-index] = value;
    else
	get_utcb()->amd64.br[32U-index] = value;
}

#endif /* !__GLUE__V4_AMD64__IA32__TCB_H__ */
