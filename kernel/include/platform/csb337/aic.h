/*********************************************************************
 *                
 * Copyright (C) 2003-2004,  National ICT Australia (NICTA)
 *                
 * File path:     platform/csb337/aic.h
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
 * $Id: aic.h,v 1.1 2004/08/12 10:58:53 cvansch Exp $
 *                
 ********************************************************************/
#ifndef __PLATFORM__CSB337__AIC_H__
#define __PLATFORM__CSB337__AIC_H__

/* Atmel AT91RM9200 Advanced Interrupt Controller (AIC) */
#define		AIC_OFFSET	0x000
#define		AIC_VADDR	(SYS_VADDR | AIC_OFFSET)

#define		AIC(x)		*((volatile word_t*)(AIC_VADDR + (x)))

#define		AIC_SMR(n)  (0x00+4*((n)&31))/* Source Mode Registers Read/Write	*/
#define		AIC_SVR(n)  (0x80+4*((n)&31))/* Source Vector Registers Read/Write	*/

#define		AIC_IVR		0x100	/* Interrupt Vector Register Read-only 0x0	*/
#define		AIC_FVR		0x104	/* Fast Interrupt Vector Register Read-only 0x0	*/
#define		AIC_ISR		0x108	/* Interrupt Status Register Read-only 0x0	*/
#define		AIC_IPR		0x10C	/* Interrupt Pending Register Read-only 0x0(1)	*/
#define		AIC_IMR		0x110	/* Interrupt Mask Register Read-only 0x0	*/
#define		AIC_CISR	0x114	/* Core Interrupt Status Register Read-only 0x0	*/

#define		AIC_IECR	0x120	/* Interrupt Enable Command Register Write-only	*/
#define		AIC_IDCR	0x124	/* Interrupt Disable Command Register Write-only*/
#define		AIC_ICCR	0x128	/* Interrupt Clear Command Register Write-only	*/
#define		AIC_ISCR	0x12C	/* Interrupt Set Command Register Write-only	*/
#define		AIC_EOICR	0x130	/* End of Interrupt Command Register Write-only	*/
#define		AIC_SPU		0x134	/* Spurious Interrupt Vector Register Read/Write 0x0 */
#define		AIC_DCR		0x138	/* Debug Control Register Read/Write 0x0	*/

#define		AIC_FFER	0x140	/* Fast Forcing Enable Register Write-only	*/
#define		AIC_FFDR	0x144	/* Fast Forcing Disable Register Write-only	*/
#define		AIC_FFSR	0x148	/* Fast Forcing Status Register Read-only 0x0	*/

/* Some hardcoded interrupt numbers */
#define		AIC_IRQ_FIQ	0
#define		AIC_IRQ_SYS	1	/* System interrupt - multiplexed ST/RTC/PMC/DBGU/MC */

union aic_smr {
    struct {
	BITFIELD4(word_t,
	    prior	: 3,
	    res1	: 2,
	    src_type    : 2,
	    res2	: 25
	);
    };
    word_t raw;
};

#define		AIC_SMR_PRIOR_MIN	0
#define		AIC_SMR_PRIOR_MAX	7
#define		AIC_SMR_SRC_LEVEL_LOW	0
#define		AIC_SMR_SRC_EDGE_LOW	1
#define		AIC_SMR_SRC_LEVEL_HIGH	2
#define		AIC_SMR_SRC_EDGE_HIGH	3

union aic_isr {
    struct {
	BITFIELD2(word_t,
	    irqid	: 3,
	    res1	: 29
	);
    };
    word_t raw;
};

union aic_csir {
    struct {
	BITFIELD3(word_t,
	    nifq	: 1,	/* nFIQ line active/deactivated */
	    nirq	: 1,	/* nIRQ line active/deactivated */
	    res1	: 30
	);
    };
    word_t raw;
};

union aic_debug {
    struct {
	BITFIELD3(word_t,
	    prot	: 1,	/* Protection Mode (Safe reads of AIC_IVR) */
	    gmsk	: 1,	/* Global Mask */
	    res1	: 30
	);
    };
    word_t raw;
};

#endif /* __PLATFORM__CSB337__AIC_H__ */
