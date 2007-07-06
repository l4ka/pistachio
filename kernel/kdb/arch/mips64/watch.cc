/*********************************************************************
 *                
 * Copyright (C) 2002,  University of New South Wales
 *                
 * File path:     kdb/arch/mips64/watch.cc
 * Description:   MIPS WATCH point
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
 * $Id: watch.cc,v 1.3 2004/04/05 06:22:15 cvansch Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include <kdb/cmd.h>
#include <kdb/kdb.h>
#include <kdb/input.h>
#include INC_ARCH(mipsregs.h)


DECLARE_CMD_GROUP (watchpoint);


/**
 * cmd_mips64_watch: Mips64 WATCH management.
 */
DECLARE_CMD (cmd_mips64_watch, arch, 'w', "watch", "MIPS Watchpoint");

CMD(cmd_mips64_watch, cg)
{
    return watchpoint.interact (cg, "watch");
}

/**
 * cmd_mips64_wshow: show current
 */
DECLARE_CMD (cmd_mips64_wset, watchpoint, 's', "set", "set watch point");

CMD(cmd_mips64_wset, cg)
{
    word_t addr = get_hex ("Phys Address", 0x0, NULL);

    char r = get_choice ("Watch read access", "y/n", 'y');
    char w = get_choice ("Watch write access", "y/n", 'y');

    u32_t watchlo = (u32_t)addr & (~3);
    u32_t watchhi = (addr>>32)&0xF;
    if (r == 'y')
	watchlo |= 2;
    if (w == 'y')
	watchlo |= 1;

    write_32bit_cp0_register(CP0_WATCHLO, watchlo);
    write_32bit_cp0_register(CP0_WATCHHI, watchhi);

    return CMD_NOQUIT;
}

/**
 * cmd_mips64_wshow: show current
 */
DECLARE_CMD (cmd_mips64_wshow, watchpoint, 'w', "show", "show watch status");

CMD(cmd_mips64_wshow, cg)
{
    u32_t watchlo = read_32bit_cp0_register(CP0_WATCHLO);
    u32_t watchhi = read_32bit_cp0_register(CP0_WATCHHI);

    printf("watch status\n");
    printf("  Watch is %s\n", watchlo & 3 ? "enabled" : "disabled");

    if (watchlo & 3)
    {
	printf("Watching physical address: %p for", watchlo&(~3) | (((word_t)watchhi)<<32));
	switch (watchlo & 3) {
	case 1: printf("write access\n"); break;
	case 2: printf("read access\n"); break;
	case 3: printf("any access\n"); break;
	default: ;
	}
    }

    return CMD_NOQUIT;
}

