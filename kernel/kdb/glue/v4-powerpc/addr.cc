/****************************************************************************
 *
 * Copyright (C) 2002, Karlsruhe University
 *
 * File path:	glue/v4-powerpc/addr.cc
 * Description:	Dump address space layout info.
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
 * $Id: addr.cc,v 1.3 2003/11/17 13:56:51 joshua Exp $
 *
 ***************************************************************************/

#include <debug.h>
#include <kdb/kdb.h>

#include INC_GLUE(config.h)
#include INC_GLUE(space.h)

DECLARE_CMD (cmd_addr_space, arch, 'a', "addrspace", "Print address space layout");


CMD(cmd_addr_space, cg)
{
    printf( "kernel area: 0x%p --> 0x%p\n",
	    KERNEL_AREA_START, KERNEL_AREA_END );
    printf( "             kernel offset: 0x%08x\n", KERNEL_OFFSET );

    printf( "ktcb area:   0x%p --> 0x%p\n", 
	KTCB_AREA_START, KTCB_AREA_END );
    printf( "             ktcb size:  %d\n", KTCB_SIZE );
    printf( "             total ktcb: %d\n", 1 << L4_GLOBAL_THREADNO_BITS );

    printf( "device area: 0x%p --> 0x%p\n",
	    DEVICE_AREA_START, DEVICE_AREA_END );
    printf( "pghash area: 0x%p --> 0x%p\n",
	    PGHASH_AREA_START, PGHASH_AREA_END );
    printf( "cpu area:    0x%p --> 0x%p\n",
	    CPU_AREA_START, CPU_AREA_END );
    printf( "copy area:   0x%p --> 0x%p\n",
	    COPY_AREA_START, COPY_AREA_END );

    printf( "root utcb:   0x%p\n", ROOT_UTCB_START );
    printf( "root kip:    0x%p\n", ROOT_KIP_START );

    printf( "\nsizeof(space_t): %d\n", sizeof(space_t) );

    space_t *space = (space_t *)0;
    printf( "space_t user offset: 0x%p\n", &space->x.user );
    printf( "space_t kernel offset: 0x%p\n", &space->x.kernel );
    printf( "space_t device offset: 0x%p\n", &space->x.device );
    printf( "space_t phhash offset (where we store space_t members): 0x%p\n", 
	    &space->x.kip_area );
    printf( "space_t cpu offset: 0x%p\n", &space->x.cpu );
    printf( "space_t tcb offset: 0x%p\n", &space->x.tcb );

    return CMD_NOQUIT;
}

