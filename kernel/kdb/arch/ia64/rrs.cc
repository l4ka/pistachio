/*********************************************************************
 *                
 * Copyright (C) 2002,  Karlsruhe University
 *                
 * File path:     kdb/arch/ia64/rrs.cc
 * Description:   Region register access
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
 * $Id: rrs.cc,v 1.5 2003/09/24 19:05:06 skoglund Exp $
 *                
 ********************************************************************/
#include INC_ARCH(rr.h)
#include <kdb/cmd.h>
#include <kdb/kdb.h>
#include <debug.h>


/**
 * cmd_rrs: dump region registers
 */
DECLARE_CMD (cmd_rrs, arch, 'r', "rrs", "dump region registers");

CMD(cmd_rrs, cg)
{
    printf ("Region registers:\n");
    for (int i = 0; i < 8; i++)
    {
	rr_t rr = get_rr (i);
	word_t size = rr.page_size ();
	char ss;

	if (size >= GB (1))
	    ss = 'G', size >>= 30;
	else if (size >= MB (1))
	    ss = 'M', size >>= 20;
	else
	    ss = 'K', size >>= 10;
	
	printf ("  rr[%d] = rid: 0x%5x,  page size: %3d%cB,  VHPT %s\n",
		i, rr.region_id (), size, ss,
		rr.is_vhpt_enabled () ? "enabled" : "disabled");
    }

    return CMD_NOQUIT;
}

