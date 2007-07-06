/*********************************************************************
 *                
 * Copyright (C) 2002,  University of New South Wales
 *                
 * File path:     kdb/arch/mips64/cp0.cc
 * Description:   MIPS-64 CPUID
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
 * $Id: cp0.cc,v 1.2 2003/09/24 19:05:08 skoglund Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include <kdb/kdb.h>

#include INC_ARCH(mips_cpu.h)
#include INC_ARCH(cpu.h)

/**
 * cmd_dpuid: dump CPU ID
 */
DECLARE_CMD (cmd_cp0, arch, '0', "dumpcp0", "dump cp0 registers");

typedef struct
{
    char    *name;
    int	    num;
    int	    word64;
} cp0_reg;

/* Coprocessor 0 register names */
#define INDEX		0
#define RANDOM		1
#define ENTRYLO0	2
#define ENTRYLO1	3
#define CONTEXT		4
#define PAGEMASK	5
#define WIRED		6
#define BADVADDR	8
#define COUNT		9
#define ENTRYHI		10
#define COMPARE		11
#define STATUS		12
#define CAUSE		13
#define EPC		14
#define PRID		15
#define CONFIG		16
#define LLADDR		17
#define WATCHLO		18
#define WATCHHI		19
#define XCONTEXT	20
#define FRAMEMASK	21
#define DIAGNOSTIC	22
#define PERFORMANCE	25
#define ECC		26
#define CACHEERR	27
#define TAGLO		28
#define TAGHI		29
#define ERROREPC	30

static cp0_reg regs[] =
{
    { "BadVaddr",  BADVADDR, 1 },
    { "CacheErr",  CACHEERR, 0 },
    { "Cause",     CAUSE, 0 }, 
    { "Compare",   COMPARE, 0 },
    { "Config",    CONFIG, 0 },
    { "Context",   CONTEXT, 1 },
    { "Count",     COUNT, 0 },
    { "ECC",       ECC, 0 },
    { "EntryHi",   ENTRYHI, 1 },
    { "EntryLo0",  ENTRYLO0, 1 },
    { "EntryLo1",  ENTRYLO1, 1 },
    { "EPC",       EPC, 1 },
    { "ErrorEPC",  ERROREPC, 1 },
    { "Index",     INDEX, 0 },
    { "LLaddr",    LLADDR, 0 },
    { "PageMask",  PAGEMASK, 0 },
    { "PerfCount", PERFORMANCE, 0 },
    { "PRID",      PRID, 0 },
    { "Random",    RANDOM, 0 },
    { "Status",    STATUS, 0 },
    { "TagHi0",    TAGHI, 0 },
    { "TagLo0",    TAGLO, 0 },
    { "WatchHi",   WATCHHI, 0 },
    { "WatchLo",   WATCHLO, 0 },
    { "Wired",     WIRED, 0 },
    { "XContext",  XCONTEXT, 1 },
    { 0, 0, 0 }
};

word_t read_cp0_reg(word_t i)
{
    word_t ret;

    switch (i) {
    case INDEX:		ret = read_32bit_cp0_register($0 ); break;
    case RANDOM:	ret = read_32bit_cp0_register($1 ); break;
    case ENTRYLO0:	ret = read_64bit_cp0_register($2 ); break;
    case ENTRYLO1:	ret = read_64bit_cp0_register($3 ); break;
    case CONTEXT:	ret = read_64bit_cp0_register($4 ); break;
    case PAGEMASK:	ret = read_32bit_cp0_register($5 ); break;
    case WIRED:		ret = read_32bit_cp0_register($6 ); break;
    case BADVADDR:	ret = read_64bit_cp0_register($8 ); break;
    case COUNT:		ret = read_32bit_cp0_register($9 ); break;
    case ENTRYHI:	ret = read_64bit_cp0_register($10); break;
    case COMPARE:	ret = read_32bit_cp0_register($11); break;
    case STATUS:	ret = read_32bit_cp0_register($12); break;
    case CAUSE:		ret = read_32bit_cp0_register($13); break;
    case EPC:		ret = read_64bit_cp0_register($14); break;
    case PRID:		ret = read_32bit_cp0_register($15); break;
    case CONFIG:	ret = read_32bit_cp0_register($16); break;
    case LLADDR:	ret = read_32bit_cp0_register($17); break;
    case WATCHLO:	ret = read_32bit_cp0_register($18); break;
    case WATCHHI:	ret = read_32bit_cp0_register($19); break;
    case XCONTEXT:	ret = read_64bit_cp0_register($20); break;
    case FRAMEMASK:	ret = read_64bit_cp0_register($21); break;
    case DIAGNOSTIC:	ret = read_64bit_cp0_register($22); break;
    case PERFORMANCE:	ret = read_32bit_cp0_register($25); break;
    case ECC:		ret = read_32bit_cp0_register($26); break;
    case CACHEERR:	ret = read_32bit_cp0_register($27); break;
    case TAGLO:		ret = read_32bit_cp0_register($28); break;
    case TAGHI:		ret = read_32bit_cp0_register($29); break;
    case ERROREPC:	ret = read_64bit_cp0_register($30); break;
    default: ret = 0x0000deadbeef0000UL;
    }
    return ret;
}

CMD (cmd_cp0, cg)
{
    word_t x;

    printf("Dumping Contents of CP0 Registers\n");
    x = 0;
    while (regs[x].name) {
	if (regs[x].word64)
	    printf("  %12s [%2d]  0x%016lx\n", regs[x].name,
			    regs[x].num, read_cp0_reg(regs[x].num));
	else
	    printf("  %12s [%2d]          0x%08lx\n", regs[x].name,
			    regs[x].num, read_cp0_reg(regs[x].num));
	x++;
    } 
    return CMD_NOQUIT;
}
