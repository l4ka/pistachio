/*********************************************************************
 *
 * Copyright (C) 2004,  National ICT Australia (NICTA)
 *
 * File path:     glue/v4-arm/resource_functions.h
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
 * $Id: resource_functions.h,v 1.9 2004/09/30 08:58:13 cvansch Exp $
 *
 ********************************************************************/


#ifndef __GLUE__V4_ARM__RESOURCE_FUNCTIONS_H__
#define __GLUE__V4_ARM__RESOURCE_FUNCTIONS_H__

#include INC_GLUE(resources.h)
#include INC_API(tcb.h)

/* FIXME - this is a naieve implementation of string IPC - it requires
 * the flushing of the data cache prior to the data cache synonym creation,
 * and after the operation completes. When using FASS, it is possible to
 * make this much more efficient, by doing a direct copy between the
 * address spaces when the source/destination sections are disjoint. With
 * non-FASS we can delay the flush until context switch.
 */

INLINE void thread_resources_t::sync_copy_area(tcb_t *tcb, tcb_t *partner)
{
    unsigned int i, start_section = copy_dest_base >> 20;

    arm_cache::cache_flush();

    for (i = 0; i < COPY_AREA_BLOCK_SECTIONS; ++i) {
#ifdef CONFIG_ENABLE_FASS
        cpd->pt.pd_split.copy_area[i] =
                virt_to_page_table(partner->get_space())->pt.pdir[start_section
                        + i];

#else
        virt_to_page_table(tcb->get_space())->pt.pd_split.copy_area[i] =
                virt_to_page_table(partner->get_space())->pt.pdir[start_section
                        + i];
#endif
    }
}

/**
 * Enable a copy area.  
 *
 * @param tcb                   TCB of current thread
 * @param partner               TCB of partner thread
 * @param addr                  address within destination space
 *
 * @return an address into the copy area where kernel should perform
 * the IPC copy.
 */
INLINE addr_t thread_resources_t::enable_copy_area(tcb_t *tcb, tcb_t *partner,
        addr_t addr)
{
    unsigned int start_section = (word_t)addr >> 20;

    tcb->resource_bits += COPY_AREA; 

    copy_dest_base = start_section << 20;

    sync_copy_area(tcb, partner);

    return addr_offset((addr_t)COPY_AREA_START, 
            (word_t) addr & (ARM_SECTION_SIZE - 1));
}

/**
 * Release all copy areas.
 *
 * @param tcb                   TCB of current thread
 * @param disable_copyarea      should copy area resource be disabled or not
 */
INLINE void thread_resources_t::release_copy_area (tcb_t *tcb,
                                                   bool disable_copyarea)
{
    if (tcb->resource_bits.have_resource(COPY_AREA)) {
        unsigned int i;

        arm_cache::cache_flush(); 

        for (i = 0; i < COPY_AREA_BLOCK_SECTIONS; ++i) {
#ifdef CONFIG_ENABLE_FASS
            cpd->pt.pd_split.copy_area[i].raw = 0;
#else
            virt_to_page_table(tcb->get_space())->pt.pd_split.copy_area[i].raw = 0;
#endif
        }

        arm_cache::tlb_flush();
    
        if (disable_copyarea)
            tcb->resource_bits -= COPY_AREA;
    }
}

INLINE addr_t thread_resources_t::copy_area_real_address(addr_t addr)
{
    return (addr_t)((word_t)addr - COPY_AREA_START + copy_dest_base);
}


/**
 * Mark the thread as being in a kernel IPC
 *
 * @param tcb			current TCB
 */
INLINE void thread_resources_t::set_kernel_ipc (tcb_t * tcb)
{
    tcb->resource_bits += KIPC;
}


/**
 * Clear the kernel IPC bit
 *
 * @param tcb			current TCB
 */
INLINE void thread_resources_t::clear_kernel_ipc (tcb_t * tcb)
{
    tcb->resource_bits -= KIPC;
}


#endif /* __GLUE__V4_ARM__RESOURCE_FUNCTIONS_H__ */
