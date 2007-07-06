/*********************************************************************
 *                
 * Copyright (C) 2003,  National ICT Australia (NICTA)
 *                
 * File path:     kdb/arch/powerpc64/dabr.cc
 * Description:   Data Address Break Point Support
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
 * $Id: dabr.cc,v 1.3 2004/06/04 06:49:14 cvansch Exp $
 *                
 ********************************************************************/

#include <debug.h>
#include <kdb/cmd.h>
#include <kdb/kdb.h>
#include <kdb/input.h>

#include INC_ARCH(ppc64_registers.h)

static word_t last_dabr = 0;

/**
 * cmd_powerpc64_dabr_set: set powerpc64 dabr
 */
DECLARE_CMD (cmd_powerpc64_dabr_set, arch, 'b', "dabr", "set data address breakpoint");

CMD(cmd_powerpc64_dabr_set, cg)
{
    word_t val = get_hex ("Address", last_dabr, "data address");

    printf( "Setting break point to %p\n", val );
    val = val & (~0x7ul);
    last_dabr = val;

    val |= get_choice ("With translation", "y/n", 'y') == 'y' ? 0x4 : 0;
    val |= get_choice ("Trap Writes", "y/n", 'y') == 'y' ? 0x2 : 0;
    val |= get_choice ("Trap Reads", "y/n", 'y') == 'y' ? 0x1 : 0;

    ppc64_set_spr( SPR_DABR, val );
    printf( "\n" );

    return CMD_NOQUIT;
}


