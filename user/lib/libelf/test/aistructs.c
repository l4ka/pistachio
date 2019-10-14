#if 0
/*
 * Australian Public Licence B (OZPLB)
 * 
 * Version 1-0
 * 
 * Copyright (c) 2004 University of New South Wales
 * 
 * All rights reserved. 
 * 
 * Developed by: Operating Systems and Distributed Systems Group (DiSy)
 *               University of New South Wales
 *               http://www.disy.cse.unsw.edu.au
 * 
 * Permission is granted by University of New South Wales, free of charge, to
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
 *     * Neither the name of University of New South Wales, nor the names of its
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
 * conditions, or imposes obligations or liability on University of New South
 * Wales or one of its contributors in respect of the Software that
 * cannot be wholly or partly excluded, restricted or modified, the
 * liability of University of New South Wales or the contributor is limited, to
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
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "aistructs.h"

uint64_t
ai_sizeof(struct aiarchinfo *arch, enum aistruct_types type)
{
	switch (type) {
		case AI_WORD:
			assert(arch->word_size != 0);
			switch(arch->word_size){
				case AI_WS_32BIT:
					return 4;
				case AI_WS_64BIT:
					return 8;
				default:
					assert(0);
			}
			break;
		case AI_INT8:
			return 1;
		case AI_INT16:
			return 2;
		case AI_INT32:
			return 4;
		case AI_INT64:
			return 8;
		case AI_UNKNOWN:
		default:
			printf("Warning: unknown AI type %d!\n", type);
			exit(1);
	}
}

struct aistruct *
ai_find_name(struct aistruct *aitype, char *name)
{
	while (aitype->name != NULL) {
		if (strcmp(aitype->name, name) == 0)
			return aitype;
		aitype++;
	}
	return NULL;
}

uint64_t
ai_offset(struct aiarchinfo *arch, struct aistruct *aitype, char *name)
{
	uint64_t counter=0;

	while (aitype->name != NULL) {
		if (strcmp(aitype->name, name) == 0)
			break;
		counter += ai_sizeof(arch, aitype->type) * aitype->multiplicity;
		aitype ++;
	}
	if (aitype->name == NULL) {
		printf("Warning: type name '%s' not found in AI struct.\n", name);
		exit(1);
	}
	return counter;
}

uint64_t
ai_structsize(struct aiarchinfo *arch, struct aistruct *aitype)
{
    uint64_t counter=0;

	while (aitype->name != NULL) {
		counter += ai_sizeof(arch, aitype->type) * aitype->multiplicity;
		aitype ++;
	}
	return counter;
}

void
ai_write(struct aiarchinfo *arch, struct aistruct *aitype, char *name, uint64_t value, void *start)
{
    uint64_t offset;
    struct aistruct *aielement;

    aielement = ai_find_name(aitype, name);
    offset = ai_offset(arch, aitype, name);
    
    switch (ai_sizeof(arch, aielement->type)){
	case 1:
	    write8(value, start + offset, arch->endianness);
	    break;
	case 2:
	    write16(value, start + offset, arch->endianness);
	    break;
	case 4:
	    write32(value, start + offset, arch->endianness);
	    break;
	case 8:
	    write64(value, start + offset, arch->endianness);
	    break;
	default:
	    printf("Unknown size for ai_write\n");
	    exit(1);
    }
}

uint64_t
ai_read(struct aiarchinfo *arch, struct aistruct *aitype, char *name, void *start)
{
    uint64_t offset;
    struct aistruct *aielement;
    
    aielement = ai_find_name(aitype, name);
    offset = ai_offset(arch, aitype, name);

    switch (ai_sizeof(arch, aielement->type)) {
	case 1:
	    return read8(start+offset, arch->endianness);
	case 2:
	    return read16(start+offset, arch->endianness);
	case 4:
	    return read32(start+offset, arch->endianness);
	case 8:
	    return read64(start+offset, arch->endianness);
	default:
	    printf("Unknown size of ai_read\n");
	    exit(1);
    }

}


#endif
