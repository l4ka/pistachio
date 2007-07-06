/*********************************************************************
 *                
 * Copyright (C) 2003,  National ICT Australia (NICTA)
 *                
 * File path:     arch/powerpc64/stab.cc
 * Description:   segment table management
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
 * $Id: stab.cc,v 1.4 2004/06/04 03:40:20 cvansch Exp $
 *                
 ********************************************************************/

#include <debug.h>
#include <kmemory.h>
#include <kdb/tracepoints.h>
#include INC_ARCH(segment.h)
#include INC_GLUE(hwspace.h)

DECLARE_KMEM_GROUP (kmem_stab);

void ppc64_stab_t::init()
{
    word_t _stab;
    addr_t page = kmem.alloc( kmem_stab, POWERPC64_STAB_SIZE );
    //TRACEF( "created segement table at %p\n", page );

    base.raw = (word_t)virt_to_phys( page );
    base.x.valid = 1;

    _stab = (word_t)virt_to_phys( page );
    /* Get the segment table */
    ppc64_stab_t *stab = (ppc64_stab_t *)&_stab;

    word_t vsid = get_kernel_space()->get_vsid( (addr_t)KERNEL_OFFSET );
    word_t esid = ESID( KERNEL_OFFSET );

    ppc64_ste_t *ste = stab->find_insertion( vsid, esid );
    ste->set_entry( esid, 0, 1, 0, vsid );
 
    /* XXX hack hack - were should this happen */
    vsid = get_kernel_space()->get_vsid( (addr_t)KTCB_AREA_START );
    esid = ESID( KTCB_AREA_START );
    ste = stab->find_insertion( vsid, esid );
    ste->set_entry( esid, 0, 1, 0, vsid );

    /* XXX hack hack - were should this happen */
    vsid = get_kernel_space()->get_vsid( (addr_t)CPU_AREA_START );
    esid = ESID( CPU_AREA_START );
    ste = stab->find_insertion( vsid, esid );
    ste->set_entry( esid, 0, 1, 0, vsid );

    /* XXX hack hack - were should this happen */
    vsid = get_kernel_space()->get_vsid( (addr_t)0xfffd0000f80003fdul );
    esid = ESID( 0xfffd0000f80003fdul );
    ste = stab->find_insertion( vsid, esid );
    ste->set_entry( esid, 0, 1, 0, vsid );
}

void ppc64_stab_t::free()
{
    addr_t page = (addr_t)get_stab();
    kmem.free( kmem_stab, page, POWERPC64_STAB_SIZE );
}

