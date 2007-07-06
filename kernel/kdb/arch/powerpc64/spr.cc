/*********************************************************************
 *                
 * Copyright (C) 2003,  National ICT Australia (NICTA)
 *                
 * File path:     kdb/arch/powerpc64/spr.cc
 * Description:   Special Purpose Register Commands
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
 * $Id: spr.cc,v 1.3 2005/01/18 13:31:53 cvansch Exp $
 *                
 ********************************************************************/

#include <debug.h>
#include <kdb/cmd.h>
#include <kdb/kdb.h>

#include INC_ARCH(ppc64_registers.h)


/**
 * cmd_powerpc64_spr_show: dump powerpc64 sprs
 */
DECLARE_CMD (cmd_powerpc64_spr_show, arch, 'r', "spr", "show SPR registers");

CMD(cmd_powerpc64_spr_show, cg)
{
    word_t val;

    printf( "--- Special Purpose Registers ---\n" );

    val = ppc64_get_spr( SPR_DSISR );
    printf( "DSISR (%d)\t= %08lx\n", SPR_DSISR, val );
    val = ppc64_get_spr( SPR_DAR );
    printf( "DAR (%d)\t= %08lx\n", SPR_DAR, val );
    val = ppc64_get_spr( SPR_DEC );
    printf( "DEC (%d)\t= %08lx\n", SPR_DEC, val );
    val = ppc64_get_spr( SPR_SDR1 );
    printf( "SDR1 (%d)\t= %016lx\n", SPR_SDR1, val );
    val = ppc64_get_spr( SPR_SRR0 );
    printf( "SRR0 (%d)\t= %016lx\n", SPR_SRR0, val );
    val = ppc64_get_spr( SPR_SRR1 );
    printf( "SRR1 (%d)\t= %016lx\n", SPR_SRR1, val );
    val = ppc64_get_spr( SPR_SPRG0 );
    printf( "SPRG0 (%d)\t= %016lx\n", SPR_SPRG0, val );
    val = ppc64_get_spr( SPR_SPRG1 );
    printf( "SPRG1 (%d)\t= %016lx\n", SPR_SPRG1, val );
    val = ppc64_get_spr( SPR_SPRG2 );
    printf( "SPRG2 (%d)\t= %016lx\n", SPR_SPRG2, val );
    val = ppc64_get_spr( SPR_SPRG3 );
    printf( "SPRG3 (%d)\t= %016lx\n", SPR_SPRG3, val );
    val = ppc64_get_spr( SPR_ASR );
    printf( "ASR (%d)\t= %016lx\n", SPR_ASR, val );
    val = ppc64_get_spr( SPR_PVR );
    printf( "PVR (%d)\t= %08lx\n", SPR_PVR, val );
    val = ppc64_get_spr( SPR_DABR );
    printf( "DABR (%d)\t= %016lx\n", SPR_DABR, val );

#if CONFIG_CPU_POWERPC64_PPC970
    printf("-- PPC970 Specific --\n");
    val = ppc64_get_spr( SPR_VRSAVE );
    printf( "VRSAVE (%d)\t= %08lx\n", SPR_VRSAVE, val );
    val = ppc64_get_spr( SPR_SCOMMC );
    printf( "SCOMMC (%d)\t= %016lx\n", SPR_SCOMMC, val );
    val = ppc64_get_spr( SPR_SCOMMD );
    printf( "SCOMMD (%d)\t= %016lx\n", SPR_SCOMMD, val );
    val = ppc64_get_spr( SPR_HIOR );
    printf( "HIOR (%d)\t= %016lx\n", SPR_HIOR, val );
    val = ppc64_get_spr( SPR_IMC );
    printf( "IMC (%d)\t= %016lx\n", SPR_IMC, val );
    val = ppc64_get_spr( SPR_HID0 );
    printf( "HID0 (%d)\t= %016lx\n", SPR_HID0, val );
    val = ppc64_get_spr( SPR_HID1 );
    printf( "HID1 (%d)\t= %016lx\n", SPR_HID1, val );
    val = ppc64_get_spr( SPR_HID4 );
    printf( "HID4 (%d)\t= %016lx\n", SPR_HID4, val );
    val = ppc64_get_spr( SPR_HID5 );
    printf( "HID5 (%d)\t= %016lx\n", SPR_HID5, val );
    val = ppc64_get_spr( SPR_DABRX );
    printf( "DABRX (%d)\t= %016lx\n", SPR_DABRX, val );
    val = ppc64_get_spr( SPR_PIR );
    printf( "PIR (%d)\t= %08lx\n", SPR_PIR, val );
#endif
 
    printf( "\n" );

    return CMD_NOQUIT;
}


