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
Authors: Charles Gray, Ben Leslie, Alex Webster
*/

#include <l4/kip.h>
#include <l4/kcp.h>
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include <l4e/regions.h>
#include <l4e/bootinfo.h>

#define ALIGN(x, y) (((x)+(y)-1) & ((uintptr_t)(~((y)-1))))

#define BASE_PAGESIZE 0x1000 /* FIXME: (benjl) should be in a header! */

#define page_round_up(address) (((address) + (BASE_PAGESIZE - 1)) & (~(BASE_PAGESIZE - 1)))
#define page_round_down(address) ((address) & (~(BASE_PAGESIZE - 1)))

#define MEM_RAM    1
#define MEM_IO     2
#define MEM_VM     4
#define MEM_OTHER  8

/* read a memory descriptor from the KIP.
 * Return 1 if it's physical (eg. we care) and the info about it
 */
static int
l4_memdesc_info( L4_KernelInterfacePage_t *kip, int pos, 
		 L4_Word_t *low, L4_Word_t *high, L4_Word_t *type )
{
	L4_MemoryDesc_t *l4memdesc;

	l4memdesc = L4_MemoryDesc(kip, pos);
	assert(l4memdesc != NULL);

	/* get the info out */
	*low = L4_MemoryDescLow(l4memdesc);
	*high = L4_MemoryDescHigh(l4memdesc);
	*type = l4memdesc->x.type;

	assert(*high > *low);

	/* return whether physical */
	return (l4memdesc->x.v == 0);
}


/* add another initial object */
void
l4e_add_initial_object(struct initial_obj *objs, const char *name, 
		       uintptr_t base, uintptr_t end, uintptr_t entry, 
		       char flags)
{
	/* so we have something to put in the array */
	if( name == NULL )
		name = "";

	/* copy the data */
	strncpy(objs->name, name, INITIAL_NAME_MAX);
	objs->name[INITIAL_NAME_MAX-1] = '\0';
	objs->base = base;
	objs->end = end;
	objs->entry_pt = entry;
	objs->flags = flags;
}

/* find the initial objects */
static unsigned
find_initial_objects(unsigned max, struct initial_obj *initial_objs)
{
	L4_Word_t low, high, type;
	L4_KernelInterfacePage_t *kip;
	L4_KernelConfigurationPage_t *kcp;
	unsigned i, phys, count = 0;
	
	assert(initial_objs != NULL);

	kip = L4_GetKernelInterface();
	kcp = (L4_KernelConfigurationPage_t*) kip;

	/* Find all the reserved objects */
	for (i = 0; i < kip->MemoryInfo.n; i++) {
		/* get the info */
		phys = l4_memdesc_info(kip, i, &low, &high, &type);

		/* is it physical? */
		if (!phys)
			continue;

		/* Add things which aren't available */
		if(type == L4_ReservedMemoryType) {
			/* add the object to the pool */
			l4e_add_initial_object(initial_objs++,
					       "L4 Obj", low, high, 
					       0, IOF_RESERVED | IOF_PHYS);
		} else {
			continue;
		}
		count++;
		if (count == max) goto overflow;
	}

	/* now add sigma0 */
	l4e_add_initial_object(initial_objs++, "Sigma0", kcp->sigma0.low,
			       kcp->sigma0.high, 0, IOF_RESERVED | IOF_PHYS);
	count++;
	if (count == max) goto overflow;

	/* Add objects from the L4 Bootinfo struct */
	count += bootinfo_find_initial_objects(max - count, initial_objs);

 overflow:
	return count;
}

/* find chunks of regions of memory */
static unsigned
find_memory_regions(unsigned max, int memory_type, int except_type, struct memdesc *memdescs)
{
	L4_Word_t low, high, type;
	L4_KernelInterfacePage_t *kip;
	unsigned i, r, j, pos = 0;
	int memdesc_type;
	
	assert(memdescs != NULL);

	kip = L4_GetKernelInterface();

	for (i = 0; i < kip->MemoryInfo.n; i++) 
	{
		/* get the info */
		r = l4_memdesc_info(kip, i, &low, &high, &type);

		/* is it physical? */
		if (r == 1) {
			switch(type) {
			case L4_ConventionalMemoryType:
				memdesc_type = MEM_RAM;
				break;
			case L4_SharedMemoryType:
			case L4_DedicatedMemoryType:
				memdesc_type = MEM_IO;
				break;
			default:
				memdesc_type = MEM_OTHER;
				break;
			}
		}  else {
			/* Can't do VM here */
			memdesc_type = MEM_VM;
		}

		if (memdesc_type & memory_type) {
			/* add it to the array */
			bool covered = 0;
			if (pos >= (max-1))
				assert(!"memory region array too small!\n");
			/* Check that this region isn't already covered! */
			for (j = 0; j < pos; j++) {
				/* Extend top of memsection */
				if (low >= memdescs[j].base && low < memdescs[j].end && high >= memdescs[j].end) {
					memdescs[j].end = high;
					covered = true;
					break;
				} else if (low <= memdescs[j].base && high <= memdescs[j].end && high > memdescs[j].base) {
					memdescs[j].base = high + 1;
					covered = true;
					break;
				} else if (low > memdescs[j].base && low < memdescs[j].end && 
					   high < memdescs[j].end && high > memdescs[j].base) {
					covered = true;
					break;
				}
			}

			if (! covered) {
				memdescs[pos].base = low;
				memdescs[pos].end  = high;
				pos++;
			}
		} else if (memdesc_type & except_type) {
			/* Need to remove this from the memdescs list, annoying! */
			pos = _l4e_remove_chunk(memdescs, pos, max, low, high);
		}
	}

	/* the number of actual descriptors we copied */
	return pos;
}


void
l4e_remove_virtmem(struct l4e_memory_info *meminfo,
		   uintptr_t base, uintptr_t end, unsigned pagesize)
{
	meminfo->num_vmregions = _l4e_remove_chunk(meminfo->vmregions, 
			meminfo->num_vmregions, 
			meminfo->max_vmregions,
			page_round_down(base), 
			page_round_up(end) - 1);
}

static void
set_flags(struct l4e_memory_info *meminfo, char match, char set)
{
	int i;
	for (i=0; i < meminfo->num_objects; i++)
		if (meminfo->objects[i].flags & match)
			meminfo->objects[i].flags |= set;
}
#include <l4/kdebug.h>
void
l4e_get_memory_info(struct l4e_memory_info *meminfo, 
		    struct memdesc *regions, int num_regions,
		    struct memdesc *ioregions, int num_ioregions,
		    struct memdesc *vmregions, int num_vmregions,
		    struct initial_obj *objects, int num_objects,
		    int apps_virtual, int restrict_vm)
{
	int i;

	meminfo->regions = regions;
	meminfo->max_regions = num_regions;
	meminfo->num_regions = find_memory_regions(num_regions, MEM_RAM, MEM_IO, regions);

	meminfo->ioregions = ioregions;
	meminfo->max_ioregions = num_ioregions;
	meminfo->num_ioregions = find_memory_regions(num_ioregions, MEM_IO, MEM_RAM, ioregions);

	meminfo->vmregions = vmregions;
	meminfo->max_vmregions = num_vmregions;
	meminfo->num_vmregions = find_memory_regions(num_vmregions, MEM_VM, 0, vmregions);
	
	/* Create a guard page */
	meminfo->num_vmregions = _l4e_remove_chunk(meminfo->vmregions, meminfo->num_vmregions, num_vmregions,
					      0, 0xfff);

	if (restrict_vm == 1) {
		/* Remove physical and I/O regions */
		for (i = 0; i < meminfo->num_regions; i++) {
			meminfo->num_vmregions = _l4e_remove_chunk(meminfo->vmregions, meminfo->num_vmregions, num_vmregions,
							      meminfo->regions[i].base, meminfo->regions[i].end);
		}
#if 0
		for (i = 0; i < meminfo->num_ioregions; i++) {
			meminfo->num_vmregions = _l4e_remove_chunk(meminfo->vmregions, meminfo->num_vmregions, num_vmregions,
							      meminfo->ioregions[i].base, meminfo->ioregions[i].end);
		}
#endif
	}

	meminfo->objects = objects;
	meminfo->max_objects = num_objects;
	meminfo->num_objects = find_initial_objects(num_objects, objects);

	/* Remove any initial objects from free physical ram */
	for (i=0; i < meminfo->num_objects; i++) {
		if (meminfo->objects[i].flags & IOF_PHYS) {
			meminfo->num_regions = _l4e_remove_chunk(meminfo->regions, 
							    meminfo->num_regions, 
							    num_regions,
							    page_round_down(meminfo->objects[i].base), 
							    page_round_up(meminfo->objects[i].end) - 1);
		}
	}

	if (apps_virtual)
		set_flags(meminfo, IOF_APP, IOF_VIRT);
}

#if 0
uintptr_t
l4e_region_count_frames(struct l4e_memory_info *meminfo, unsigned pagesize)
{
	uintptr_t nframes = 0;
	int i;
	/* first count free frames */
	for( i = 0; i < meminfo->num_regions; i++ )
	{
		uintptr_t region_size = meminfo->regions[i].end - 
			meminfo->regions[i].base + 1;
		uintptr_t region_frames = region_size / pagesize;

		nframes += region_frames;
	}
	return nframes;
}
#endif

#if 0
static void
fprint_flags(FILE *stream, char flags)
{
	if (flags & IOF_RESERVED)
		fputc('R', stream);
	if (flags & IOF_APP)
		fputc('A', stream);
	if (flags & IOF_ROOT)
		fputc('r', stream);
	if (flags & IOF_BOOT)
		fputc('B', stream);

	if ((flags & IOF_VIRT) && (flags & IOF_PHYS))
		fputc('D', stream);
	else if ((flags & IOF_VIRT))
		fputc('V', stream);
	else if ((flags & IOF_PHYS))
		fputc('P', stream);
}

static void
fprint_size(FILE *stream, uintptr_t size)
{
#define KB (1024)
#define MB (1024 * KB)
#define GB (1024 * MB)

	if (size > GB) {
		fprintf(stream, "%" PRIdPTR " GB", size / GB);
		size %= GB;
	}
	if (size > MB) {
		fprintf(stream, " %" PRIdPTR " MB", size / MB);
		size %= MB;
	}
	if (size > KB) {
		fprintf(stream, " %" PRIdPTR " KB", size / KB);
		size %= KB;
	}
	if (size > 0) {
		fprintf(stream, " %" PRIdPTR " B", size);
	}
}

void
l4e_fprintf_memory_info(FILE *stream, struct l4e_memory_info *mem_info)
{
	int i;

	fprintf(stream, "mem_info <%p>\n", mem_info);

	/* Print out physical memory */
	fprintf(stream, "Physical regions\n");
	for (i=0; i < mem_info->num_regions; i++) {
		fprintf(stream, " %d: <%p:%p>\n", i, 
			(void*) mem_info->regions[i].base,
			(void*) mem_info->regions[i].end);
	}
	
	/* Print out I/O Regions */
	fprintf(stream, "I/O regions\n");
	for (i=0; i < mem_info->num_ioregions; i++) {
		fprintf(stream, " %d: <%p:%p>\n", i, 
			(void*) mem_info->ioregions[i].base,
			(void*) mem_info->ioregions[i].end);
	}

	/* Print out VM */
	fprintf(stream, "VM regions\n");
	for (i=0; i < mem_info->num_vmregions; i++) {
		fprintf(stream, " %d: <%p:%p>\n", i, 
			(void*) mem_info->vmregions[i].base,
			(void*) mem_info->vmregions[i].end);
	}

	/* Print out objects */
	fprintf(stream, "Objects:\n");
	for (i=0; i < mem_info->num_objects; i++) {
		fprintf(stream, " %d: %s (", i, 
			mem_info->objects[i].name);
		fprint_flags(stream, mem_info->objects[i].flags);
		fprintf(stream, ") <%p:%p> (",
			(void*) mem_info->objects[i].base, 
			(void*) mem_info->objects[i].end);
		fprint_size(stream, mem_info->objects[i].end - 
			    mem_info->objects[i].base + 1);
		fprintf(stream, ") (%p)\n", (void*) mem_info->
			objects[i].entry_pt);
	}
}
#endif
