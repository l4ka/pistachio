/*********************************************************************
 *                
 * Copyright (C) 2002-2004, 2007,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/resources.h
 * Description:   ia32 specific resources
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
 ********************************************************************/
#ifndef __GLUE__V4_X86__RESOURCES_H__
#define __GLUE__V4_X86__RESOURCES_H__

#include INC_API(resources.h)

#define HAVE_RESOURCE_TYPE_E
enum resource_type_e {
    FPU			= 0,
    COPY_AREA		= 1,
#if defined(CONFIG_X86_SMALL_SPACES)
    IPC_PAGE_TABLE	= 2,
#endif
#if defined(CONFIG_SMP)
    SMP_PAGE_TABLE	= 3,
#endif
#if defined(CONFIG_IS_64BIT)
    COMPATIBILITY_MODE	= 4,
#endif
};


class thread_resources_t : public generic_thread_resources_t
{
public:
    void dump(tcb_t * tcb);
    void save(tcb_t * tcb) __asm__ ("tcb_resources_save");
    void load(tcb_t * tcb) __asm__ ("tcb_resources_load");
    void purge(tcb_t * tcb);
    void init(tcb_t * tcb);
    void free(tcb_t * tcb);

public:
    void x86_no_math_exception(tcb_t * tcb);
    void smp_xcpu_pagetable (tcb_t * tcb, cpuid_t cpu);
    void enable_copy_area (tcb_t * tcb, addr_t * saddr,
			   tcb_t * partner, addr_t * daddr);
    void release_copy_area (tcb_t * tcb, bool disable_copyarea);
    
    addr_t copy_area_address (word_t n);
    addr_t copy_area_real_address (word_t n);
    word_t copy_area_pdir_idx (word_t n, word_t p);
   
private:
    addr_t fpu_state;
    word_t last_copy_area;

    word_t pdir_idx[COPY_AREA_COUNT][COPY_AREA_PDIRS];
};




#endif /* !__GLUE__V4_X86__RESOURCES_H__ */
