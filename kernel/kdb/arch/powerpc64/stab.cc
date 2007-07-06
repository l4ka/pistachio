/*********************************************************************
 *                
 * Copyright (C) 2003,  National ICT Australia (NICTA)
 *                
 * File path:     kdb/arch/powerpc64/stab.cc
 * Description:   Segment Table management commands
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
 * $Id: stab.cc,v 1.3 2004/06/04 06:49:14 cvansch Exp $
 *                
 ********************************************************************/

#include <debug.h>
#include <kdb/cmd.h>
#include <kdb/kdb.h>

#include INC_API(tcb.h)
#include INC_ARCH(segment.h)


DECLARE_CMD_GROUP (powerpc64_stab);


/**
 * cmd_powerpc64_stab: PowerPC64 Segemnt Table management.
 */
DECLARE_CMD (cmd_powerpc64_stab, arch, 's', "stab", "segment table management");

CMD(cmd_powerpc64_stab, cg)
{
    return powerpc64_stab.interact (cg, "stab");
}

extern tcb_t * kdb_get_tcb();

/**
 * cmd_powerpc64_stab_dump: dump powerpc64 STAB
 */
DECLARE_CMD (cmd_powerpc64_stab_dump, powerpc64_stab, 'd', "dump", "dump segment table");

CMD(cmd_powerpc64_stab_dump, cg)
{
    int i, j;
    tcb_t * tcb = kdb_get_tcb();

    if (tcb)
    {
	space_t *space = tcb->get_space();
	if (!space) space = get_kernel_space();

	ppc64_stab_t *stab = space->get_seg_table();

	ppc64_ste_t *stegA, *stegB;

	printf( "-------- Segment Table Dump --------\n" );
	printf( "space = %p, segment table = %p\n\n", space, stab->get_stab() );

	for( i = 0; i < 16; i ++ )
	{
	    j = (~i)&31;
	    printf( "  ----- Segment Group %2d -----      |", i );
	    printf( "  ----- Segment Group %2d -----\n", j );

	    stegA = &((ppc64_ste_t *)stab->get_stab())[i*8];
	    stegB = &((ppc64_ste_t *)stab->get_stab())[j*8];

	    for( int k = 0; k < 8; k ++ )
	    {
		printf( "%p - %p | ", stegA[k].raw.word0, stegA[k].raw.word1 );
		printf( "%p - %p\n", stegB[k].raw.word0, stegB[k].raw.word1 );
	    }
	}
    }

    printf( "\n" );

    return CMD_NOQUIT;
}


