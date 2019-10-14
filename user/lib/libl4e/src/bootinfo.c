/*
 * Australian Public Licence B (OZPLB)
 * 
 * Version 1-0
 * 
 * Copyright (c) 2004 National ICT Australia
 * 
 * All rights reserved. 
 * 
 * Developed by: Embedded, Real-time and Operating Systems Program (ERTOS)
 *               National ICT Australia
 *               http://www.ertos.nicta.com.au
 * 
 * Permission is granted by National ICT Australia, free of charge, to
 * any person obtaining a copy of this software and any associated
 * documentation files (the "Software") to deal with the Software without
 * restriction, including (without limitation) the rights to use, copy,
 * modify, adapt, merge, publish, distribute, communicate to the public,
 * sublicense, and/or sell, lend or rent out copies of the Software, and
 * to permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimers.
 * 
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimers in the documentation and/or other materials provided
 *       with the distribution.
 * 
 *     * Neither the name of National ICT Australia, nor the names of its
 *       contributors, may be used to endorse or promote products derived
 *       from this Software without specific prior written permission.
 * 
 * EXCEPT AS EXPRESSLY STATED IN THIS LICENCE AND TO THE FULL EXTENT
 * PERMITTED BY APPLICABLE LAW, THE SOFTWARE IS PROVIDED "AS-IS", AND
 * NATIONAL ICT AUSTRALIA AND ITS CONTRIBUTORS MAKE NO REPRESENTATIONS,
 * WARRANTIES OR CONDITIONS OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO ANY REPRESENTATIONS, WARRANTIES OR CONDITIONS
 * REGARDING THE CONTENTS OR ACCURACY OF THE SOFTWARE, OR OF TITLE,
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NONINFRINGEMENT,
 * THE ABSENCE OF LATENT OR OTHER DEFECTS, OR THE PRESENCE OR ABSENCE OF
 * ERRORS, WHETHER OR NOT DISCOVERABLE.
 * 
 * TO THE FULL EXTENT PERMITTED BY APPLICABLE LAW, IN NO EVENT SHALL
 * NATIONAL ICT AUSTRALIA OR ITS CONTRIBUTORS BE LIABLE ON ANY LEGAL
 * THEORY (INCLUDING, WITHOUT LIMITATION, IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHERWISE) FOR ANY CLAIM, LOSS, DAMAGES OR OTHER
 * LIABILITY, INCLUDING (WITHOUT LIMITATION) LOSS OF PRODUCTION OR
 * OPERATION TIME, LOSS, DAMAGE OR CORRUPTION OF DATA OR RECORDS; OR LOSS
 * OF ANTICIPATED SAVINGS, OPPORTUNITY, REVENUE, PROFIT OR GOODWILL, OR
 * OTHER ECONOMIC LOSS; OR ANY SPECIAL, INCIDENTAL, INDIRECT,
 * CONSEQUENTIAL, PUNITIVE OR EXEMPLARY DAMAGES, ARISING OUT OF OR IN
 * CONNECTION WITH THIS LICENCE, THE SOFTWARE OR THE USE OF OR OTHER
 * DEALINGS WITH THE SOFTWARE, EVEN IF NATIONAL ICT AUSTRALIA OR ITS
 * CONTRIBUTORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH CLAIM, LOSS,
 * DAMAGES OR OTHER LIABILITY.
 * 
 * If applicable legislation implies representations, warranties, or
 * conditions, or imposes obligations or liability on National ICT
 * Australia or one of its contributors in respect of the Software that
 * cannot be wholly or partly excluded, restricted or modified, the
 * liability of National ICT Australia or the contributor is limited, to
 * the full extent permitted by the applicable legislation, at its
 * option, to:
 * a.  in the case of goods, any one or more of the following:
 * i.  the replacement of the goods or the supply of equivalent goods;
 * ii.  the repair of the goods;
 * iii. the payment of the cost of replacing the goods or of acquiring
 *  equivalent goods;
 * iv.  the payment of the cost of having the goods repaired; or
 * b.  in the case of services:
 * i.  the supplying of the services again; or
 * ii.  the payment of the cost of having the services supplied again.
 * 
 * The construction, validity and performance of this licence is governed
 * by the laws in force in New South Wales, Australia.
 */
#include <l4/bootinfo.h>
#include <l4/kip.h>
#include <l4e/bootinfo.h>
#include <l4e/regions.h>
#include <elf/elf.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>

#define DIT_INITIAL     32L /* Root Task */

static unsigned
bootinfo_add_simpleexec(unsigned max, struct initial_obj *initial_objs, L4_BootRec_t *rec)
{
	int recs = 1, flags = 0;
 	L4_Word_t tstart, tsize, dstart, dsize;
 	
 	tstart = L4_SimpleExec_TextVstart(rec);
 	tsize = L4_SimpleExec_TextSize(rec);
 	dstart = L4_SimpleExec_DataVstart(rec);
 	dsize = L4_SimpleExec_DataSize(rec);	

	if (L4_SimpleExec_Flags(rec) & DIT_INITIAL) {
		flags = IOF_ROOT | IOF_VIRT;
	} else {
		flags = IOF_APP;
	}
	
	l4e_add_initial_object(initial_objs, L4_SimpleExec_Cmdline(rec),
			       L4_SimpleExec_TextVstart(rec), L4_SimpleExec_TextVstart(rec)+
			       L4_SimpleExec_TextSize(rec) - 1,L4_SimpleExec_InitialIP(rec), 
			       flags | IOF_PHYS);

 	if ((! (dstart < (tstart + tsize) && dstart >= tstart)) /* It isn't inside */
 		&& (dsize != 0))
	{
		char data_name[255];	/* XXX get proper length */

		initial_objs++;
		recs++;

		strncpy(data_name, L4_SimpleExec_Cmdline(rec), 254-5);
		strncat(data_name, ".data", 255);

		l4e_add_initial_object(initial_objs, data_name,
			       dstart, dstart + dsize - 1, 0, 
			       flags | IOF_PHYS);
	}
	
	return recs;
}

static unsigned
bootinfo_add_simplemodule(unsigned max, struct initial_obj *initial_objs, L4_BootRec_t *rec)
{
	int recs = 1, flags = 0;

	if (L4_SimpleExec_Flags(rec) & DIT_INITIAL) {
		flags = IOF_ROOT | IOF_VIRT;
	} else {
		flags = IOF_APP;
	}
	
	l4e_add_initial_object(initial_objs, L4_Module_Cmdline(rec),
			       L4_Module_Start(rec), 
			       L4_Module_Start(rec) + L4_Module_Size(rec) - 1, 
			       0, flags | IOF_PHYS);
	return recs;
}


#if 0
static unsigned
bootinfo_add_simpleelf(unsigned max, struct initial_obj *initial_objs, L4_BootRec_t *rec)
{
	uint64_t start = 0, totalsize = 0;
	unsigned int i;
	void *ehdr;

	struct L4e_SimpleElf *elfrec = (struct L4e_SimpleElf *) rec;
	ehdr = L4e_SimpleElf_ElfHeader(elfrec);
	printf("elf rec is %p, elf header is %p\n", elfrec, ehdr);
	printf("add simpleelf: cmd line %s\n", L4e_SimpleElf_CmdLine(elfrec));
	if (elf_checkFile(ehdr) != 0) {
		printf("Not an Elf file (!) - not adding\n");
		return 0;
	}
	/* Parse the section headers of the elf file and add the relevant bits. 
	 * This just adds, en masse, all sections with a non-zero address and size.
	 * We don't deal with, nor do we check for, overlapping sections.
	 * (parts of?) this functionality could probably go into l4e or the elf
	 * library.
	*/
	printf("Num sections: %u\n", elf_getNumSections(ehdr));
	for (i=0; i < elf_getNumSections(ehdr); i++) {
		uint64_t addr, size;
		addr = elf_getSectionAddr(ehdr, i);
		size = elf_getSectionSize(ehdr, i);
		if (addr && size) {
			if (start == 0)
				start = addr;
			totalsize += size;
		}
	}
	printf("l4e_add_initial_object (%p, %s, %llx, %llx, %x, %x)\n",
	       initial_objs, 
	       L4e_SimpleElf_CmdLine(elfrec),
	       start, start + totalsize, 
	       (uintptr_t)elf_getEntryPt(ehdr), IOF_APP | IOF_PHYS);

	l4e_add_initial_object(initial_objs, 
			       L4e_SimpleElf_CmdLine(elfrec),
			       start, start + totalsize - 1, 
			       (uintptr_t)elf_getEntryPt(ehdr), IOF_APP | IOF_PHYS);
	return 1;
}
#endif
				
unsigned int
bootinfo_find_initial_objects(unsigned max, struct initial_obj *initial_objs)
{
	L4_KernelInterfacePage_t *kip;
	L4_BootRec_t *rec;
	void *bootinfo;
	unsigned num_recs, count=0, objs;

	kip = L4_GetKernelInterface();

	bootinfo = (void*) L4_BootInfo( kip );

	/* check its validity */
	if(L4_BootInfo_Valid(bootinfo) == 0) {
		return 0;
	}

	num_recs = L4_BootInfo_Entries( bootinfo );
	rec = L4_BootInfo_FirstEntry( bootinfo );

	while(num_recs > 0) {
		L4_Word_t type;

		assert(rec != NULL);
		
		/* find what type it is */
		type = L4_BootRec_Type(rec);
		objs = 0;
		switch(type) {
		case L4_BootInfo_Module:
			objs = bootinfo_add_simplemodule(max-count, initial_objs++, rec);
			break;
		case L4_BootInfo_SimpleExec:
			objs = bootinfo_add_simpleexec(max-count, initial_objs++, rec);
			break;
		default:
			break;
		}
		if (objs) {
			count += objs;
			initial_objs += (objs-1);
			if (count == max) 
				goto overflow;
		}
		
		/* move to the next record */
		rec = L4_BootRec_Next(rec);
		num_recs--;
	}

	l4e_add_initial_object(initial_objs++, "Boot", 
			       (uintptr_t) bootinfo, 
			       (uintptr_t) bootinfo + L4_BootInfo_Size(bootinfo) - 1,
			       0, IOF_BOOT | IOF_PHYS | IOF_VIRT);
	count++;

 overflow:
	return count;
}
