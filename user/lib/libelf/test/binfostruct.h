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
#ifndef BINFOSTRUCT_H
#define BINFOSTRUCT_H

#include "aistructs.h"

struct aistruct dite_bootinfo[] = {
    {"magic", AI_WORD, 1},
    {"version", AI_WORD, 1},
    {"size", AI_WORD, 1},
    {"first_entry", AI_WORD, 1},
    {"num_entries", AI_WORD, 1},
    {"__reserved", AI_WORD, 3},
    {NULL, 0, 0}
};

struct aistruct dite_bootrec[] = {
    {"type", AI_WORD, 1},
    {"version", AI_WORD, 1},
    {"offset_next", AI_WORD, 1},
    {NULL, 0, 0}
};

struct aistruct dite_simpleexec[] = {
    {"type", AI_WORD, 1},
    {"version", AI_WORD, 1},
    {"offset_next", AI_WORD, 1},
    {"text_pstart", AI_WORD, 1},
    {"text_vstart", AI_WORD, 1},
	{"text_size", AI_WORD, 1},
    {"data_pstart", AI_WORD, 1},
    {"data_vstart", AI_WORD, 1},
    {"data_size", AI_WORD, 1},
    {"bss_pstart", AI_WORD, 1},
    {"bss_vstart", AI_WORD, 1},
    {"bss_size", AI_WORD, 1},
    {"initial_ip", AI_WORD, 1},
    {"flags", AI_WORD, 1},
    {"label", AI_WORD, 1},
    {"cmdline_offset", AI_WORD, 1},
    {NULL, 0, 0}
};

struct aistruct dite_module[] = {
	{"type", AI_WORD, 1},
	{"version", AI_WORD, 1},
	{"offset_next", AI_WORD, 1},
	{"start", AI_WORD, 1},
	{"size", AI_WORD, 1},
	{"cmdline_offset", AI_WORD, 1},
	{NULL, 0, 0}
};

struct aistruct dite_binfo_simpleelf[] = {
	{"type", AI_WORD, 1},
	{"version", AI_WORD, 1},
	{"offset_next", AI_WORD, 1},
	{"elf_header_offset", AI_WORD, 1},
	{"cmdline_offset", AI_WORD, 1},
	{NULL, 0, 0}
};

#endif
