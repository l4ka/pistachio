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
/** 
 * \file
 * 
 * blah blh blah
 */
#ifndef _L4E_REGIONS_H_
#define _L4E_REGIONS_H_

#include <stdint.h>
#include <stdio.h>

#define INVALID_ADDR (~0)

/**
 * Describe a memory region. The memory is (base, end].
 */
struct memdesc {
	uintptr_t base;
	uintptr_t end;
};

/**
 * Maximum name length.
 */
#define INITIAL_NAME_MAX 31

/**
 * Define an executable 
 */
struct initial_obj {
	char name[INITIAL_NAME_MAX]; /**< Name of the objects */
	char flags;
	uintptr_t base; /**< Base address */
	uintptr_t end; /**< End address */
	uintptr_t entry_pt; /**< Executable entry point */
};

#define IOF_RESERVED  0x01 /**< Reserverd by the kernel */
#define IOF_APP       0x02 /**< Application */
#define IOF_ROOT      0x04 /**< Root server */
#define IOF_BOOT      0x08 /**< Boot info */

#define IOF_VIRT      0x10 /**< Set if this memory is in physical */
#define IOF_PHYS      0x20 /**< Set if this memory is in virtual */ 
                           /* Both can be set for direct mapped */

struct l4e_memory_info {
	unsigned num_regions;
	unsigned max_regions;
	struct memdesc *regions;

	unsigned num_ioregions;
	unsigned max_ioregions;
	struct memdesc *ioregions;

	unsigned num_vmregions;
	unsigned max_vmregions;
	struct memdesc *vmregions;

	unsigned num_objects;
	unsigned max_objects;
	struct initial_obj *objects;
};

/* Functions */

/**
 *   Foobar does blaz asdf asdf as,.n ozx asfasd
 * @param objs list of objects
 * @param name name of object
 * @param base base address
 * @param end end address
 * \return Nothing!
 */
void l4e_add_initial_object(struct initial_obj *objs, const char *name, 
			    uintptr_t base, uintptr_t end, uintptr_t entry, 
			    char flags);
/**
 *  This function does stuff
 */
void l4e_get_memory_info(struct l4e_memory_info *mem_info, 
			 struct memdesc *regions, int num_regions,
			 struct memdesc *ioregions, int num_ioregions,
			 struct memdesc *vmregions, int num_vmregions,
			 struct initial_obj *objects, int num_objects,
			 int apps_virtual, int restricted_vm);

void l4e_fprintf_memory_info(FILE *stream, struct l4e_memory_info *mem_info);
/**
 * \return Number of frames
 */
uintptr_t l4e_region_count_frames(struct l4e_memory_info *meminfo, 
				  unsigned pagesize);
uintptr_t l4e_find_physmem(struct l4e_memory_info *mem_info, uintptr_t size, 
			   unsigned pagesize, char *name);
uintptr_t l4e_find_virtmem(struct l4e_memory_info *mem_info, uintptr_t size,
			   unsigned pagesize, char *name);
void l4e_remove_virtmem(struct l4e_memory_info *mem_info, 
			uintptr_t base, uintptr_t end, unsigned pagesize);


uintptr_t
_l4e_find_mem_in_region(struct l4e_memory_info *meminfo, struct memdesc *region, 
			uintptr_t size, unsigned pagesize, char mode);
int
_l4e_remove_chunk(struct memdesc *memdescs, int pos, int max, uintptr_t low, uintptr_t high);

#endif /* _L4E_REGIONS_H_ */
