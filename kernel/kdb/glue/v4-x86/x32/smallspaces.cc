/*********************************************************************
 *                
 * Copyright (C) 2003,  Karlsruhe University
 *                
 * File path:     kdb/glue/v4-ia32/smallspaces.cc
 * Description:   Management of small spaces from kernel debugger
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
 * $Id: smallspaces.cc,v 1.3 2003/09/24 19:05:16 skoglund Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include <kdb/kdb.h>
#include <kdb/cmd.h>
#include <kdb/input.h>

#include INC_GLUE(space.h)


extern space_t * small_space_owner[SMALLSPACE_AREA_SIZE >> X86_X32_PDIR_BITS];


DECLARE_CMD_GROUP (x86_x32_smallspaces);

/**
 * Small address spaces managment.
 */
DECLARE_CMD (cmd_smallspaces, arch, 's', "smallspaces",
	     "Manage small addesss spaces");

CMD (cmd_smallspaces, cg)
{
    return x86_x32_smallspaces.interact (cg, "smallspaces");
}


/**
 * Dump small address spaces.
 */
DECLARE_CMD (cmd_smallspaces_dump, x86_x32_smallspaces, 'd', "dump",
	     "Dump small spaces");

CMD (cmd_smallspaces_dump, cg)
{
    const word_t max_idx = SMALLSPACE_AREA_SIZE >> X86_X32_PDIR_BITS;

    printf ("Small address space allocation:\n");

    word_t b = 0, i;
    for (i = 1; i < max_idx; i++)
    {
	if (small_space_owner[b] != small_space_owner[i])
	{
	    printf (small_space_owner[b] ?
		    "%5d-%d MB\t<%p-%p>  space: %p\n" :
		    "%5d-%d MB\t<%p-%p>  (free)\n",
		    b*4, i*4,
		    SMALLSPACE_AREA_START + (b * X86_X32_PDIR_SIZE),
		    SMALLSPACE_AREA_START + (i * X86_X32_PDIR_SIZE),
		    small_space_owner[b]);
	    b = i;
	}
    }
    printf (small_space_owner[b] ?
	    "%5d-%d MB\t<%p-%p>  space: %p\n" :
	    "%5d-%d MB\t<%p-%p>  (free)\n",
	    b*4, i*4,
	    SMALLSPACE_AREA_START + (b * X86_X32_PDIR_SIZE),
	    SMALLSPACE_AREA_START + (i * X86_X32_PDIR_SIZE),
	    small_space_owner[b]);

    return CMD_NOQUIT;
}


/**
 * Turn address space into small space.
 */
DECLARE_CMD (cmd_smallspaces_mksmall, x86_x32_smallspaces, 's', "mksmall",
	     "Make space small");

CMD (cmd_smallspaces_mksmall, cg)
{
    space_t * space = get_space ("Space");

    smallspace_id_t id;
    word_t size, idx;

    for (;;)
    {
	if ((size = get_dec ("Size (in MB)", 16)) == ABORT_MAGIC)
	    return CMD_NOQUIT;

	if ((idx = get_dec ("Index", 0)) == ABORT_MAGIC)
	    return CMD_NOQUIT;

	idx *= size;

	if (((idx + size) >> 2) * X86_X32_PDIR_SIZE <= SMALLSPACE_AREA_SIZE)
	    break;

	printf ("Invalid parameters.\n");
    }

    id.set_small (idx, size);

    if (! space->make_small (id))
	printf ("Unable to create small space!\n");

    return CMD_NOQUIT;
}


/**
 * Turn address space into large space.
 */
DECLARE_CMD (cmd_smallspaces_mklarge, x86_x32_smallspaces, 'l', "mklarge",
	     "Make space large");

CMD (cmd_smallspaces_mklarge, cg)
{
    space_t * space = get_space ("Space");
    space->make_large ();
    return CMD_NOQUIT;
}

