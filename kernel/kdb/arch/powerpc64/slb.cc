/*********************************************************************
 *                
 * Copyright (C) 2003,  National ICT Australia (NICTA)
 *                
 * File path:     kdb/arch/powerpc64/slb.cc
 * Description:   SLB management commands
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
 * $Id: slb.cc,v 1.5 2005/01/18 13:31:14 cvansch Exp $
 *                
 ********************************************************************/

#include <debug.h>
#include <kdb/cmd.h>
#include <kdb/kdb.h>

#include INC_ARCH(segment.h)


DECLARE_CMD_GROUP (powerpc64_slb);


/**
 * cmd_powerpc64_slb: PowerPC64 SLB management.
 */
DECLARE_CMD (cmd_powerpc64_slb, arch, 's', "slb", "SLB management");

CMD(cmd_powerpc64_slb, cg)
{
    return powerpc64_slb.interact (cg, "slb");
}

/**
 * cmd_powerpc64_dum: dump powerpc64 SLB
 */
DECLARE_CMD (cmd_powerpc64_dump, powerpc64_slb, 'd', "dump", "dump hardware SLB");

CMD(cmd_powerpc64_dump, cg)
{
    word_t index, asr;
    slbent_t entry;

    asm volatile ( "mfasr   %0;" : "=r" (asr) );
    printf( "-- Virtual Address Space ID: 0x%lx --\n", asr );

    printf( "Index	VSID		    ESID\n" );

    for(index = 0; index < CONFIG_POWERPC64_SLBENTRIES; index++)
    {
	__asm__ __volatile__(
	    "slbmfee  %0,%2;"	    /* Move from SLB entry, ESID */
	    "slbmfev  %1,%2;"	    /* Move from SLB entry, VSID */
            : "=r" (entry.esid), "=r" (entry.vsid)
	    : "r" (index)
	);
	printf( "%2d	%16lx    %16lx\n", index, entry.vsid, entry.esid );
    }

    printf( "\n" );

    return CMD_NOQUIT;
}


/**
 * cmd_powerpc64_trans: translate powerpc64 SLB
 */
DECLARE_CMD (cmd_powerpc64_trans, powerpc64_slb, 't', "translate", "translate hardware SLB");

CMD(cmd_powerpc64_trans, cg)
{
    word_t index, asr;
    slbent_t entry;

    asm volatile ( "mfasr   %0;" : "=r" (asr) );
    printf( "-- Virtual Address Space ID: 0x%lx --\n", asr );

    printf( "Index	VSID		    ESID\n" );

    for(index = 0; index < CONFIG_POWERPC64_SLBENTRIES; index++)
    {
	__asm__ __volatile__(
	    "slbmfee  %0,%2;"	    /* Move from SLB entry, ESID */
	    "slbmfev  %1,%2;"	    /* Move from SLB entry, VSID */
            : "=r" (entry.esid), "=r" (entry.vsid)
	    : "r" (index)
	);

	word_t esid, vsid;

	vsid = entry.vsid.X.vsid;
	esid = entry.esid.X.esid;

	printf( "%2d	%13lx0000000    %9lx0000000	[%s]\n", index, vsid, esid,
			entry.esid.X.v ? "valid" : "invalid" );
    }

    printf( "\n" );

    return CMD_NOQUIT;
}

