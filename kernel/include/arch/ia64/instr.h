/*********************************************************************
 *                
 * Copyright (C) 2002,  Karlsruhe University
 *                
 * File path:     arch/ia64/instr.h
 * Description:   IA64 instruction encodings
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
 * $Id: instr.h,v 1.2 2003/09/24 19:04:28 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__IA64__INSTR_H__
#define __ARCH__IA64__INSTR_H__


/**
 * ia64_instr_t: single IA-64 instruction
 */
class ia64_instr_t
{
public:
    union {
	//
	// Raw access to instruction
	//
	struct {
	    word_t raw		: 41;

	    inline word_t get (void)
		{ return raw; }
	} raw;

	//
	// (qp) break.m imm21
	// (qp) nop.m   imm21
	//
	struct {
	    word_t qp		: 6;
	    word_t imm20a	: 20;
	    word_t __ig		: 1;
	    word_t x4		: 4;
	    word_t x2		: 2;
	    word_t x3		: 3;
	    word_t imm		: 1;
	    word_t opcode	: 4;

	    inline bool is_break (void)
		{ return opcode == 0 && x3 == 0 && x4 == 0 && x2 == 0; }

	    inline bool is_nop (void)
		{ return opcode == 0 && x3 == 0 && x4 == 1 && x2 == 1; }

	    inline word_t immediate (void)
		{ return (imm << 20) + imm20a; }
	} m_nop;

	//
	// (qp) movl r1 = imm64
	//
	struct {
	    word_t qp		: 6;
	    word_t r1		: 7;
	    word_t imm7b	: 7;
	    word_t vc		: 1;
	    word_t ic		: 1;
	    word_t imm5c	: 5;
	    word_t imm9d	: 9;
	    word_t imm		: 1;
	    word_t opcode	: 4;

	    inline bool is_movl (void)
		{ return opcode == 6 && vc == 0; }

	    inline word_t immediate (ia64_instr_t lslot)
		{
		    return (imm << 63) + (lslot.raw.get () << 22) +
			(ic << 21) + (imm5c << 16) + (imm9d << 7) + imm7b;
		}

	    inline word_t reg (void)
		{ return r1; }
	} x_movl;
    };

} __attribute__ ((packed));



/**
 * ia64_bundle_t: bundle of three IA64 instructions, including template
 */
class ia64_bundle_t
{
public:
    enum template_e {
	mii 	= 0x00,
	mii_s3	= 0x01,
	mii_s2	= 0x02,
	mii_s23	= 0x03,
	mlx	= 0x04,
	mlx_s3	= 0x05,
	mmi	= 0x08,
	mmi_s3	= 0x09,
	mmi_s1	= 0x0a,
	mii_s13	= 0x0b,
	mfi	= 0x0c,
	mfi_s3	= 0x0d,
	mmf	= 0x0e,
	mmf_s3	= 0x0f,
	mib	= 0x10,
	mib_s3	= 0x11,
	mbb	= 0x12,
	mbb_s3	= 0x13,
	bbb	= 0x16,
	bbb_s3	= 0x17,
	mmb	= 0x18,
	mmb_s3	= 0x19,
	mfb	= 0x1c,
	mbf_s3	= 0x1d
    };

    union {
	u8_t	raw8[16];
	u64_t	raw64[2];
	unsigned int __attribute__ ((__mode__(__TI__))) raw128;
    };

    inline template_e get_template (void)
	{ return (template_e) (raw8[0] & 0x01f); }

    inline ia64_instr_t slot (word_t num)
	{
	    ia64_instr_t ret;
#if 0
	    u64_t t0, t1;
	    t0 = (word_t) raw8[0] + (((word_t) raw8[1]) << 8) +
		(((word_t) raw8[2]) << 16) + (((word_t) raw8[3]) << 24) +
		(((word_t) raw8[4]) << 32) + (((word_t) raw8[5]) << 40) +
		(((word_t) raw8[6]) << 48) + (((word_t) raw8[7]) << 56);
	    t1 = (word_t) raw8[8] + (((word_t) raw8[9]) << 8) +
		(((word_t) raw8[10]) << 16) + (((word_t) raw8[11]) << 24) +
		(((word_t) raw8[12]) << 32) + (((word_t) raw8[13]) << 40) +
		(((word_t) raw8[14]) << 48) + (((word_t) raw8[15]) << 56);
	    switch (num) {
	    case 0:
		ret.raw.raw = ((t0 >> 5) & 0x1ffffffffffUL);
		break;
	    case 1:
		ret.raw.raw = (((t0 >> 46) & 0x3ffff) +
			       ((t1 & 0x7fffffUL) << 18));
		break;
	    case 2:
		ret.raw.raw = ((t1 >> 23) & 0x1ffffffffffUL);
		break;
	    };
#else
	    switch (num) {
	    case 0:
		ret.raw.raw = ((raw64[0] >> 5) & 0x1ffffffffffUL);
		break;
	    case 1:
		ret.raw.raw = ((raw128 >> 46)  & 0x1ffffffffffUL);
		break;
	    case 2:
		ret.raw.raw = ((raw128 >> 87)  & 0x1ffffffffffUL);
		break;
	    };
#endif
	    return ret;
	}

} __attribute__ ((aligned (16)));




#endif /* !__ARCH__IA64__INSTR_H__ */
