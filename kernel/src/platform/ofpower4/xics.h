/****************************************************************************
 *
 * Copyright (C) 2003,  National ICT Australia (NICTA)
 *
 * File path:	platform/ofpower4/intctrl.h
 * Description:	IBM XICS interrupt controller.
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
 * $Id: xics.h,v 1.3 2004/06/04 02:11:06 cvansch Exp $
 *
 ***************************************************************************/

#ifndef __ARCH__POWERPC64__XICS_H__
#define __ARCH__POWERPC64__XICS_H__

class xics_interrupt_node_t
{
public:
    word_t addr;
    word_t size;
};

#ifdef CONFIG_SMP
 #define NUM_XICS    CONFIG_SMP_MAX_CPUS
#else
 #define NUM_XICS    32
#endif

class xics_interrupt_table_t
{
public:
    xics_interrupt_node_t node[NUM_XICS];
};

class xics_ipl_t
{
public:
    union {
	u32_t	word;
	u8_t	bytes[4];
    } xirr_poll;
    union {
        u32_t	word;
	u8_t	bytes[4];
    } xirr;
    u32_t   dummy;
    union {
	u32_t	word;
	u8_t	bytes[4];
    } qirr;
};

class xics_info_t
{
public:
    volatile xics_ipl_t *per_cpu[NUM_XICS];
};

#endif /* __ARCH__POWERPC64__XICS_H__ */
