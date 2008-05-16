/*********************************************************************
 *                
 * Copyright (C) 2005, 2007-2008,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/io_space.h
 * Description:   IO space specific declarations
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
 * $Id: io_space.h,v 1.3 2005/05/19 08:43:48 stoess Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_X86__IO_SPACE_H__
#define __GLUE__V4_X86__IO_SPACE_H__
#if !defined(CONFIG_X86_IO_FLEXPAGES)

#include INC_API(generic-archmap.h)

#else

#include <debug.h>
#include INC_API(config.h)
#include INC_API(fpage.h)
#include INC_API(ipc.h)
#include INC_GLUE(vrt_io.h)
#include INC_GLUE(mdb_io.h)

#define IPC_MR0_IO_PAGEFAULT                ((-8UL) << 4)

void arch_map_fpage (tcb_t * src, fpage_t snd_fpage, word_t snd_base,
		     tcb_t * dst, fpage_t rcv_fpage, bool grant);

void arch_unmap_fpage (tcb_t * from, fpage_t fpage, bool flush);

INLINE fpage_t acceptor_t::get_arch_specific_rcvwindow(tcb_t *dest)
{
    return fpage_t::complete_arch();
}

bool handle_io_pagefault(tcb_t *tcb, u16_t port, u16_t size, addr_t ip);
void zero_io_bitmap(space_t *space, word_t port, word_t log2size);
void set_io_bitmap(space_t *space, word_t port, word_t log2size);

#endif /* !defined(CONFIG_X86_IO_FLEXPAGES) */




#endif /* !__GLUE__V4_X86__IO_SPACE_H__ */
