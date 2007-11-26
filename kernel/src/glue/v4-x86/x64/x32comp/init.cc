/*********************************************************************
 *                
 * Copyright (C) 2006-2007,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/x64/x32comp/init.cc
 * Description:   System initialization for Compatibility Mode
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
 * $Id: init.cc,v 1.2 2006/10/21 13:52:15 reichelt Exp $
 *                
 ********************************************************************/
#include <init.h>

#include INC_API(types.h)
#include INC_API(kernelinterface.h)
#include INC_ARCH(x86.h)
#include INC_GLUE_SA(x32comp/kernelinterface.h)

static void SECTION(SEC_INIT) copy_boot_info(word_t *binfo, x32::word_t *binfo_32)
{
    word_t addr = *binfo;
    /* Check if boot info exists and is located below 4G */
    if (addr && (addr & 0xffffffff00000000UL) == 0)
    {
	u32_t magic_32 = *((u32_t *) addr);
	if (magic_32 == 0x14b0021dU)
	{
	    /* Generic bootinfo, 64 or 32 bit? */
	    u32_t version_32 = *((u32_t *) (addr + 4));
	    if (version_32)
	    {
		*binfo_32 = addr;
		*binfo = 0;
	    }
	}
	else
	    /* Not a generic bootinfo structure, just use it */
	    *binfo_32 = addr;
    }
}

static void SECTION(SEC_INIT) copy_clock_info(clock_info_t *cinfo, x32::clock_info_t *cinfo_32)
{
    cinfo_32->read_precision     = cinfo->get_read_precision();
    cinfo_32->schedule_precision = cinfo->get_schedule_precision();
}

#if 0
static void SECTION(SEC_INIT) copy_thread_info(thread_info_t *tinfo, x32::thread_info_t *tinfo_32)
{
    tinfo_32->set_user_base  (tinfo->get_user_base());
    tinfo_32->set_system_base(tinfo->get_system_base());
}
#endif

static void SECTION(SEC_INIT) copy_procdesc(procdesc_t *pdesc, x32::procdesc_t *pdesc_32)
{
    pdesc_32->set_external_frequency(pdesc->external_freq);
    pdesc_32->set_internal_frequency(pdesc->internal_freq);
}

static void SECTION(SEC_INIT) copy_processor_info(processor_info_t *pinfo, x32::processor_info_t *pinfo_32)
{
    word_t num_processors = pinfo->get_num_processors();

    for (word_t processor = 0; processor < num_processors; processor++)
	copy_procdesc(pinfo->get_procdesc(processor), pinfo_32->get_procdesc(processor));
    pinfo_32->processors = pinfo->processors;
}

static void SECTION(SEC_INIT) copy_memdesc(memdesc_t *mdesc, x32::memory_info_t *minfo_32)
{
    word_t low, high;

    low = (word_t) mdesc->low();

    if (!(low & ~0xffffffffUL))
    {
	high = (word_t) mdesc->high();
	if (high & ~0xffffffffUL)
	    high = 0xffffffffUL;
	if (mdesc->is_virtual()
	    && low < UTCB_MAPPING_32 + X86_PAGE_SIZE
	    && high >= UTCB_MAPPING_32)
	{
	    if (UTCB_MAPPING_32 > low)
		minfo_32->insert((x32::memdesc_t::type_e) mdesc->type(),
				 mdesc->subtype(), true,
				 (x32::addr_t) low, UTCB_MAPPING_32 - 1);
	    if (high >= UTCB_MAPPING_32 + X86_PAGE_SIZE)
		minfo_32->insert((x32::memdesc_t::type_e) mdesc->type(),
				 mdesc->subtype(), true,
				 UTCB_MAPPING_32 + X86_PAGE_SIZE, (x32::addr_t) high);
	}
	else
	{
	    minfo_32->insert((x32::memdesc_t::type_e) mdesc->type(),
			     mdesc->subtype(), mdesc->is_virtual(),
			     (x32::addr_t) low, (x32::addr_t) high);
	}
    }
}

static void SECTION(SEC_INIT) copy_memory_info(memory_info_t *minfo, x32::memory_info_t *minfo_32)
{
    word_t num_descriptors = minfo->get_num_descriptors();

    minfo_32->n = 0;

    for (word_t descriptor = 0; descriptor < num_descriptors; descriptor++)
	copy_memdesc(minfo->get_memdesc(descriptor), minfo_32);
}

static void SECTION(SEC_INIT) copy_root_server(root_server_t *serv, x32::root_server_t *serv_32)
{
    if (!(((word_t) serv->mem_region.high) & ~0xffffffffUL))
    {
	serv_32->sp              = serv->sp;
	serv_32->ip              = serv->ip;
	serv_32->mem_region.low  = (x32::addr_t) (word_t) serv->mem_region.low;
	serv_32->mem_region.high = (x32::addr_t) (word_t) serv->mem_region.high;
    }
}

static void SECTION(SEC_INIT) copy_root_server_info(kernel_interface_page_t *kip, x32::kernel_interface_page_t *kip_32)
{
    copy_root_server(&(kip->sigma0),      &(kip_32->sigma0));
    copy_root_server(&(kip->sigma1),      &(kip_32->sigma1));
    copy_root_server(&(kip->root_server), &(kip_32->root_server));
}

void SECTION(SEC_INIT) init_kip_32()
{
    kernel_interface_page_t       *kip    =       get_kip();
    x32::kernel_interface_page_t *kip_32 = x32::get_kip();

    copy_boot_info       (&(kip->boot_info),      &(kip_32->boot_info));
    copy_clock_info      (&(kip->clock_info),     &(kip_32->clock_info));
#if 0
    copy_thread_info     (&(kip->thread_info),    &(kip_32->thread_info));
#endif
    copy_processor_info  (&(kip->processor_info), &(kip_32->processor_info));
    copy_memory_info     (&(kip->memory_info),    &(kip_32->memory_info));
    copy_root_server_info(kip,                    kip_32);
}
