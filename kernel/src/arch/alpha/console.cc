/*********************************************************************
 *                
 * Copyright (C) 2002,  University of New South Wales
 *                
 * File path:     arch/alpha/console.cc
 * Description:   Console remapping routines 
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
 * $Id: console.cc,v 1.7 2004/08/25 07:11:21 awiggins Exp $
 *                
 ********************************************************************/

#include INC_API(types.h)
#include INC_API(space.h)
#include INC_ARCH(hwrpb.h)
#include INC_ARCH(pgent.h)
#include INC_ARCH(console.h)

#define INIT_TEXT  SECTION(".init.text")


extern "C" void fixup(word_t base_va, word_t hwrpb_va);

void update_hwrpb_checksum(struct hwrpb *hwrpb)
{
    word_t *p, sum;
    unsigned int i;

    for (i = 0, p = (word_t *) hwrpb, sum = 0;
	 i < (offsetof(struct hwrpb, rpb_checksum) / sizeof (word_t));
	 i++, p++)
	sum += *p;

    hwrpb->rpb_checksum = sum;
}


/* Insert mappings into the kernel's page table to cover the console area.  Note
 * that we are still in the initial AS.
 */
void INIT_TEXT remap_console(void)
{
    struct vf_map *vf;
    word_t base_va = CONSOLE_AREA_START + ALPHA_PAGE_SIZE;

    /* Move HWRPB and CRB */
    PAL::wrfen(1); // Required for M5 simulators console.
    fixup(base_va, CONSOLE_AREA_START); 
    PAL::wrfen(0);

    /* Map HWRPB */
    get_kernel_space()->add_mapping((addr_t) CONSOLE_AREA_START, (addr_t) INIT_HWRPB->rpb_phys, pgent_t::size_base, true, true);
    struct crb *crb = (struct crb *) phys_to_virt((addr_t) (INIT_HWRPB->rpb_phys + INIT_HWRPB->rpb_crb_off));
    /* Map CRB after HWRPB */
    word_t va = base_va;

    for(unsigned int i = 0; i < crb->crb_map_cnt; i++) {
	vf = &crb->map[i];
	vf->va = (word_t) va;
	for(unsigned int j = 0; j < vf->count; j++) {
	    get_kernel_space()->add_mapping((addr_t) va, (addr_t) (vf->pa + (j << ALPHA_PAGE_BITS)), pgent_t::size_base, true, true);
	    va += ALPHA_PAGE_SIZE;
	}
    }

    /* Update DISPATCH and FIXUP */
    vf = &crb->map[0];
    crb->crb_v_dispatch = (struct crd *) ((word_t) crb->crb_p_dispatch - vf->pa + vf->va);
    
    crb->crb_v_fixup = (struct crd *) ((word_t) crb->crb_p_fixup - vf->pa + vf->va);

    /* Update VPTB */
    INIT_HWRPB->rpb_vptb = VLPT_AREA_START;
    update_hwrpb_checksum(INIT_HWRPB);

}
