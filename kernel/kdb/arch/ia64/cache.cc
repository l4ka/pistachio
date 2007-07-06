/*********************************************************************
 *                
 * Copyright (C) 2002,  Karlsruhe University
 *                
 * File path:     kdb/arch/ia64/cache.cc
 * Description:   IA-64 cache info/managemnt commands
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
 * $Id: cache.cc,v 1.5 2003/09/24 19:05:06 skoglund Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include <kdb/kdb.h>
#include <kdb/cmd.h>
#include INC_ARCH(pal.h)



DECLARE_CMD_GROUP (ia64_cache);


/**
 * cmd_ia64_cache: IA-64 cache management
 */
DECLARE_CMD (cmd_ia64_cache, arch, 'c', "cache", "cache management");

CMD(cmd_ia64_cache, cg)
{
    return ia64_cache.interact (cg, "cache");
}


/**
 * cmd_ia64_cache_info: dump cache information
 */
DECLARE_CMD (cmd_ia64_cache_info, ia64_cache, 'i', "info", "dump cache info");

CMD(cmd_ia64_cache_info, cg)
{
    pal_cache_summary_t summary;
    pal_cache_info_t info;
    pal_status_e status;

    if ((status = pal_cache_summary (&summary)) != PAL_OK)
    {
	printf ("Error: PAL_CACHE_SUMMARY => %d\n", status);
	return CMD_NOQUIT;
    }

    printf ("Cache info:\n");
    pal_cache_info_t::type_e type = pal_cache_info_t::code;

    const static char *hints[] = {
	"Temp L1", "Non-Temp L1", "bit2", "Non-Temp all levels",
	"bit4", "bit5", "bit6", "bit7"
    };


    do {

	for (word_t i = 0; i < summary.cache_levels; i++)
	{
	    if ((status = pal_cache_info (i, type, &info)) != PAL_OK)
		continue;

	    printf ("  L%d %c-Cache: ",
		    i+1, info.unified ? 'U' :
		    type == pal_cache_info_t::code ? 'I' : 'D');

	    printf ("%3d%cB ",
		    info.cache_size > GB (1) ? info.cache_size >> 30 :
		    info.cache_size > MB (1) ? info.cache_size >> 20 :
		    info.cache_size >> 10,
		    info.cache_size > GB (1) ? 'G' :
		    info.cache_size > MB (1) ? 'M' : 'K');

	    if (info.associativity == 0)
		printf ("fully assoc, ");
	    else if (info.associativity == 1)
		printf ("direct, ");
	    else
		printf ("%d-way, ", info.associativity);

	    if (info.attributes == 0)
		printf ("write-through, ");
	    else if (info.attributes == 1)
		printf ("write-back, ");

	    printf ("line size: %d, ", (1UL << info.line_size));
	    printf ("stride: %d, ", (1UL << info.stride));
	    printf ("tag bits: %d-%d, ", info.tag_ls_bit, info.tag_ms_bit);
	    printf ("alias boundary: %d", info.alias_boundary);

	    printf ("\n                    ");

	    printf ("[ld.latancy: %d, ld.hints:", info.load_latency);
	    for (word_t k = 0; k < 8; k++)
		if (info.load_hints & (1 << k))
		    printf (" %s,", hints[k]);
	    if (info.load_hints == 0)
		printf (" none ");
	    printf ("\b]  ");

	    if (info.store_latency != 0xff)
	    {
		printf ("[st.latancy: %d, st.hints:", info.store_latency);
		for (word_t k = 0; k < 8; k++)
		    if (info.store_hints & (1 << k))
			printf (" %s,", hints[k]);
		if (info.store_hints == 0)
		    printf (" none ");
		printf ("\b]");
	    }
	    printf ("\n");
	}

	if (type == pal_cache_info_t::code)
	    type = pal_cache_info_t::data;
	else
	    type = pal_cache_info_t::none;

    } while (type != pal_cache_info_t::none);


    return CMD_NOQUIT;
}

