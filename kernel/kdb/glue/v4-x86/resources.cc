/*********************************************************************
 *                
 * Copyright (C) 2003-2004, 2007,  Karlsruhe University
 *                
 * File path:     kdb/glue/v4-x86/resources.cc
 * Description:   resource dumping
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
 * $Id: resources.cc,v 1.4 2006/10/18 12:06:45 reichelt Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include INC_API(tcb.h)

void thread_resources_t::dump (tcb_t * tcb)
{
    if (tcb->resource_bits.have_resource(FPU))
	printf("FPU ");
    if (tcb->resource_bits.have_resource(COPY_AREA))
    {
	printf("COPYAREA (");
	for (word_t i = 0;  i < COPY_AREA_COUNT; i++)
	    for (word_t j = 0; j < COPY_AREA_PDIRS; j++)
		printf(" %x ", copy_area_pdir_idx(i,j));
	printf(") ");
    }
#if defined(CONFIG_SMP)
    if (tcb->resource_bits.have_resource(SMP_PAGE_TABLE))
	printf("SMPPGT ");
#endif
#if defined(CONFIG_X86_COMPATIBILITY_MODE)
    if (tcb->resource_bits.have_resource(COMPATIBILITY_MODE))
	printf("COMPATIBILITY_MODE ");
#endif
}
