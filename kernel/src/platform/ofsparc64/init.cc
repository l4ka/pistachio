/*********************************************************************
 *                
 * Copyright (C) 2003, University of New South Wales
 *                
 * File path:    platform/ofsparc64/init.cc
 * Description:  Kernel initialisation for Open Firmware (OpenBoot)
 *               based SPARC v9 systems.
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
 * $Id: init.cc,v 1.5 2004/05/21 03:59:24 philipd Exp $
 *                
 ********************************************************************/

#include <kmemory.h>
#include INC_API(kernelinterface.h)
#include INC_CPU(tlb.h)
#include INC_ARCH(asi.h)
#include INC_GLUE(hwspace.h)

extern word_t _start_text[];

word_t plat_bus_freq;
word_t plat_cpu_freq;

extern "C" void SECTION(".boot")
ofsparc64_init(void)
{
    /**
     *  Load locked mappings for kernel pages.
     */
    hw_asid_t asid = NUCLEUS_CONTEXT;
    tlb_t tlb_entry;

    /**
     *  check the first memory descriptor to work out where the kernel BOOTPAGE
     *  is located in physical memory.
     */
    extern kernel_interface_page_t kip_laddr[];
    memdesc_t * memdesc =
	(memdesc_t *)((word_t)kip_laddr + (word_t)(kip_laddr->memory_info.memdesc_ptr));
    addr_t base_paddr = memdesc->low();

    tlb_entry.clear();
    tlb_entry.set_asid(asid);
    tlb_entry.set_va(_start_text);
    tlb_entry.set_writable(true);
    tlb_entry.set_privileged(true);
    tlb_entry.set_lock(true);
    tlb_entry.set_valid(true);
    tlb_entry.set_size(tlb_t::BOOTPAGE_PGSIZE);
    tlb_entry.set_cache_attrib(tlb_t::cache_vir);
    tlb_entry.set_pa(base_paddr);

    /* Add kernel data/code BOOTPAGE mappings. */
    tlb_entry.set(TLB_KERNEL_LOCKED, tlb_t::all_tlb);

    /**
     *  Setup cpu and bus frequencies from open firmware. 
     */
#warning philipd (02/02/04) XXX: hardcoded for the ultra 10
    plat_bus_freq = 100000000; /* root node, property "clock-frequency" */
    plat_cpu_freq = 300000000; /* cpu device node, property "clock-frequency" */
} // init_ofsparc64()

/**
 *  init_bootmem()
 *  Initialises kip memory descriptors for memory at boot time.
 *  returns true if more bootmem needs to be added to kmem.
 */
bool SECTION(".init")
init_bootmem(void)
{
    bool more_bootmem = 0;
    word_t bootmem_size = CONFIG_BOOTMEM_PAGES << SPARC64_PAGE_BITS;
    extern word_t _end_init[];
    addr_t start_bootmem = _end_init;
    addr_t end_bootmem;

    /**
     *  Feed kernel memory allocator initial memory.
     */

    if(bootmem_size > BOOTPAGE_SIZE) { // Only BOOTPAGE is mapped at this point.
	more_bootmem = 1;
	bootmem_size = BOOTPAGE_SIZE;
    }

    end_bootmem = addr_offset(_start_text, bootmem_size);
    kmem.init(start_bootmem, end_bootmem);

    /**
     *  Setup additional memory descriptors. Loader has set up some already.
     */

    return more_bootmem;

} // init_bootmem()
