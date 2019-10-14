#if 0
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
 *        Project:  EDIT --- Extended DIT
 *        Created:  12/07/2000 17:16:13 by Simon Winwood (sjw)
 *  Last Modified:  27/03/2001 16:58:02 by  (sjw)
 *   Version info:  $Revision: 1.9 $ 
 *    Description:
 *          Endianess abstraction layer, deals with 8, 16, 32, 64.
 *
 *       Comments:
 *
 *        License:
 *           Copyright (C) 2000, 2001 Simon Winwood, All Rights Reserved.
 * 
 *           To be licensed under the OzPLB license.
 */

#include <stdio.h>
#include <stdlib.h>
#include "aistructs.h" /* endianness */

/* Get byte b within x */
#define EXTB(x, b) ((uint8_t) ((x) >> ((b) * 8)) & 0xff)

/* Move a uint8_t into the position determined by b */
#define INSB(x, b) (((uint64_t) (x)) << ((b) * 8))

void write8(uint8_t data, void *iptr, enum ai_endianness endianness)
{   
    uint8_t *ptr = iptr;
       
    *ptr = data;
}

void write16(uint16_t data, void *iptr, enum ai_endianness endianness)
{
    uint8_t *ptr = iptr;

    if(endianness == AI_E_LSB) {
	*ptr++ = EXTB(data, 0);
	*ptr = EXTB(data, 1);
    } else {
	*ptr++ = EXTB(data, 1);
	*ptr = EXTB(data, 0);
    }
}

void write32(uint32_t data, void *iptr, enum ai_endianness endianness)
{
    uint8_t *ptr = iptr;
	
    if(endianness == AI_E_LSB) {
	*ptr++ = EXTB(data, 0);
	*ptr++ = EXTB(data, 1);
	*ptr++ = EXTB(data, 2);
	*ptr = EXTB(data, 3);
    } else {
	*ptr++ = EXTB(data, 3);
	*ptr++ = EXTB(data, 2);
	*ptr++ = EXTB(data, 1);
	*ptr = EXTB(data, 0);
    }
}

void write64(uint64_t data, void *iptr, enum ai_endianness endianness)
{
    uint8_t *ptr = iptr;
	
    if(endianness == AI_E_LSB) {
	*ptr++ = EXTB(data, 0);
	*ptr++ = EXTB(data, 1);
	*ptr++ = EXTB(data, 2);
	*ptr++ = EXTB(data, 3);
	*ptr++ = EXTB(data, 4);
	*ptr++ = EXTB(data, 5);
	*ptr++ = EXTB(data, 6);
	*ptr = EXTB(data, 7);
    } else {
	*ptr++ = EXTB(data, 7);
	*ptr++ = EXTB(data, 6);
	*ptr++ = EXTB(data, 5);
	*ptr++ = EXTB(data, 4);
	*ptr++ = EXTB(data, 3);
	*ptr++ = EXTB(data, 2);
	*ptr++ = EXTB(data, 1);
	*ptr = EXTB(data, 0);
    }
}

uint8_t read8( void *iptr, enum ai_endianness endianness)
{
    uint8_t *ptr = iptr;

    return *ptr;
}

uint16_t read16( void *iptr, enum ai_endianness endianness)
{
    uint8_t *ptr = iptr;

    if(endianness == AI_E_LSB) {
	return *ptr | *(ptr + 1) << 8;
    } else {
	return *ptr << 8 | *(ptr + 1);
    }
}

uint32_t read32( void *iptr, enum ai_endianness endianness)
{
    uint8_t *ptr = iptr;

    if(endianness == AI_E_LSB) {
	return *ptr | *(ptr + 1) << 8 | *(ptr + 2) << 16 | *(ptr + 3) << 24;
    } else {
	return *ptr << 24 | *(ptr + 1) << 16 | *(ptr + 2) << 8 | *(ptr + 3);
    }    
}

uint64_t read64(void *iptr, enum ai_endianness endianness)
{
    uint8_t *ptr = iptr;

    if(endianness == AI_E_LSB) {
	return INSB(*ptr, 0) | INSB(*(ptr + 1), 1) | INSB(*(ptr + 2), 2) | INSB(*(ptr + 3), 3) |
	    INSB(*(ptr + 4), 4) | INSB(*(ptr + 5), 5) | INSB(*(ptr + 6), 6) | INSB(*(ptr + 7), 7);
    } else {
	return INSB(*ptr, 7) | INSB(*(ptr + 1), 6) | INSB(*(ptr + 2), 5) | INSB(*(ptr + 3), 4) |
	    INSB(*(ptr + 4), 3) | INSB(*(ptr + 5), 2) | INSB(*(ptr + 6), 1) | INSB(*(ptr + 7), 0);
    }
}


#endif
