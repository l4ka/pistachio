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
/*
  Authors: Ben Leslie, Alex Webster
  Created: Tue Jul  6 2004 
*/

#include <stdio.h>
#include <l4/ipc.h>
#include <l4/kdebug.h>
#include <l4/space.h>
#include <l4e/misc.h>
#include <l4e/sigma0.h>

#define BASEPAGESIZE 0x1000 /* FIXME: (benjl) */

#define SIGMA0_REQUEST_LABEL ((-6UL) << 4)

int
l4e_sigma0_map_fpage(L4_Fpage_t virt_page, L4_Fpage_t phys_page)
{
/* 
 * XXX: These two special cases are workarounds for broken superpage
 * support in pistachio.  On ARM, 1M superpages are disabled by
 * pistachio to reduce the size of the mapping database, however due to
 * bugs in the mapping code, any mappings >= 1M get converted into 4K
 * mappings (rather than 64K).  For MIPS, the tlb refill code assumes
 * only 4K mappings are used, even the the pagetable building code will
 * use superpages where possible.  -- alexw
 */
#if defined(ARCH_ARM)
	uintptr_t virt_base = L4_Address(virt_page);
	uintptr_t phys_base = L4_Address(phys_page);
	uintptr_t offset = 0;
	uintptr_t step = L4_Size(virt_page) > 0x10000 ? 0x10000 : L4_Size(virt_page);
	uintptr_t limit = L4_Size(virt_page) - 1;

	for (virt_page = L4_Fpage(virt_base + offset, step),
	     phys_page = L4_Fpage(phys_base + offset, step);
	     offset < limit;
	     offset += step,
	     virt_page = L4_Fpage(virt_base + offset, step),
	     phys_page = L4_Fpage(phys_base + offset, step))
#elif defined(ARCH_MIPS64)
	uintptr_t virt_base = L4_Address(virt_page);
	uintptr_t phys_base = L4_Address(phys_page);
	uintptr_t offset = 0;
	uintptr_t step = 0x1000;
	uintptr_t limit = L4_Size(virt_page) - 1;

	for (virt_page = L4_Fpage(virt_base + offset, step),
	     phys_page = L4_Fpage(phys_base + offset, step);
	     offset < limit;
	     offset += step,
	     virt_page = L4_Fpage(virt_base + offset, step),
	     phys_page = L4_Fpage(phys_base + offset, step))
#endif
	{
		L4_ThreadId_t   tid;
		L4_MsgTag_t     tag;
		L4_Msg_t        msg;
		L4_MapItem_t    map;
		/*
		 * find our pager's ID 
		 */
		tid = L4_Pager();

		L4_Set_Rights(&phys_page, L4_FullyAccessible);
		/* accept fpages */
		L4_Accept(L4_MapGrantItems(virt_page));

		/* send it to our pager */
		L4_MsgClear(&msg);
		L4_MsgAppendWord(&msg, (L4_Word_t) phys_page.raw);
		L4_MsgAppendWord(&msg, (L4_Word_t) 0);
		L4_Set_Label(&msg.tag, SIGMA0_REQUEST_LABEL);

		L4_MsgLoad(&msg);

		/* make the call */
		tag = L4_Call(tid);

		/* check for an error */
		if (L4_IpcFailed(tag)) {
			return 2;
		}

		L4_MsgStore(tag, &msg);
		L4_MsgGetMapItem(&msg, 0, &map);

		/*
		 * rejected mapping? 
		 */
		if (map.X.snd_fpage.raw == L4_Nilpage.raw) {
			return 1;
		}

	}
	return 0;
}

int
l4e_sigma0_map(uintptr_t virt_addr, uintptr_t phys_addr, uintptr_t size)
{
	uintptr_t vbase = virt_addr, vend = vbase + size - 1;
	uintptr_t pbase = phys_addr, pend = pbase + size - 1;
	L4_Fpage_t vpage, ppage;

	while (vbase < vend) {
		vpage = l4e_biggest_fpage(vbase, vbase, vend);
		ppage = l4e_biggest_fpage(pbase, pbase, pend);
		if (L4_Size(vpage) > L4_Size(ppage))
			vpage = L4_Fpage(vbase, L4_Size(ppage));
		else if (L4_Size(ppage) > L4_Size(vpage))
			ppage = L4_Fpage(pbase, L4_Size(vpage));
		l4e_sigma0_map_fpage(vpage, ppage);
		vbase += L4_Size(vpage);
		pbase += L4_Size(ppage);
	}
	return 0;
}
