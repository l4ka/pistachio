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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <l4/bootinfo.h>
#include <l4/kip.h>

struct bootinfo {
	long int total_size;
	char *buffer;
};

static long int
bootinfo_eof(void *handle)
{
	struct bootinfo *bootinfo = handle;
	return bootinfo->total_size;
}

static size_t
bootinfo_read(void *data, long int position, size_t count, void *handle)
{
	struct bootinfo *bootinfo = handle;
	if (position + count > bootinfo->total_size) {
		return 0;
	}
	memcpy(data, &bootinfo->buffer[position], count);
	return count;
}

static size_t
bootinfo_write(void *data, long int position, size_t count, void *handle)
{
	struct bootinfo *bootinfo = handle;
	if (position + count > bootinfo->total_size) {
		return 0;
	}
	memcpy(&bootinfo->buffer[position], data, count);
	return count;
}

static int
bootinfo_close(void *handle)
{
	struct bootinfo *bootinfo = handle;
	free(bootinfo);
	return 0;
}

/*
  Given fname, find the bootinfo module associated with it.

  Return NULL on failure 
*/
static L4_BootRec_t *
find_bootrec(const char *fname)
{
	L4_KernelInterfacePage_t *kip;
	L4_BootRec_t *rec;
	void *bootinfo;
	unsigned num_recs, objs;

	/* Iterate through bootinfo */
	kip = L4_GetKernelInterface();
	bootinfo = (void*) L4_BootInfo(kip);
	num_recs = L4_BootInfo_Entries(bootinfo);
	rec = L4_BootInfo_FirstEntry(bootinfo);

	while(num_recs > 0) {
		L4_Word_t type;
		/* find what type it is */
		type = L4_BootRec_Type(rec);
		objs = 0;
		if (type == L4_BootInfo_Module && 
		    (strcmp(L4_Module_Cmdline(rec), fname) == 0)) {
			/* Found it! */
			break;
		}
		rec = L4_BootRec_Next(rec);
		num_recs--;
	}

	if (num_recs > 0) {
		return rec;
	} else {
		return NULL;
	}
}

FILE *
fopen(const char *fname, const char *prot)
{
	L4_BootRec_t *rec;
	struct bootinfo *handle;
	FILE *newfile;


	rec = find_bootrec(fname);
	
	if (rec == NULL) {
		return NULL;
	}

	/* Note: We may want to allocate froma different pool
	   of memory to minimise this being tramped on a by a
	   user */

	newfile = malloc(sizeof(FILE));

	if (newfile == NULL) {
		return NULL;
	}

	handle = malloc(sizeof(struct bootinfo));

	if (handle == NULL) {
		free(newfile);
		return NULL;
	}

	handle->total_size = L4_Module_Size(rec);
	handle->buffer = (char*) L4_Module_Start(rec);

	newfile->handle = handle;
	newfile->write_fn = bootinfo_write;
	newfile->read_fn = bootinfo_read;
	newfile->close_fn = bootinfo_close;
	newfile->eof_fn = bootinfo_eof;
	newfile->current_pos = 0;
	newfile->buffering_mode = _IONBF;
	newfile->buffer = NULL;
	newfile->unget_pos = 0;
	newfile->eof = 0;

#ifdef THREAD_SAFE
	newfile->mutex.holder = 0;
	newfile->mutex.needed = 0;
	newfile->mutex.count = 0;
#endif

	return newfile;
}
