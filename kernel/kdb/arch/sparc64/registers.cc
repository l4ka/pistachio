/*********************************************************************
 *                
 * Copyright (C) 2003, University of New South Wales
 *                
 * File path:    kdb/arch/sparc64/registers.cc
 * Description:   
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
 * $Id: registers.cc,v 1.3 2004/05/21 02:34:54 philipd Exp $
 *                
 ********************************************************************/

#include <debug.h>
#include <kdb/kdb.h>
#include <kdb/input.h>

#include INC_ARCH(registers.h)

extern void putc(char);

DECLARE_CMD(cmd_dump_version, arch, 'v',
	    "version register", "Dump the version register");
DECLARE_CMD(cmd_dump_pstate, arch, 's',
	    "processor state registers", "Dump the processors state registers");
DECLARE_CMD(cmd_dump_tstate, arch, 't',
	    "trap state registers", "Dump the trap state registers, current");
DECLARE_CMD(cmd_dump_tstate_all, arch, 'T',
	    "trap state registers, all TLs",
	    "Dump the trap state registers, all TLs");
DECLARE_CMD(cmd_dump_wstate, arch, 'w',
	    "register-window state registers",
	    "Dump the register-window state registers");
//DECLARE_CMD(?, arch, 'f',
//	    "floating-point registers", "Dump the floating-point registers");
//DECLARE_CMD(?, arch, 'g',
//	    "global registers", "Dump the global registers");
//DECLARE_CMD(?, arch, 'i',
//	    "windowed registers", "Dump the windowed integer registers"); 

CMD(cmd_dump_version, cg)
{
    ver_t ver;

    ver.get();
    ver.print();

    return CMD_NOQUIT;

} // CMD(cmd_dump_version, cg)

CMD(cmd_dump_pstate, cg)
{
    y_reg_t  y;
    ccr_t    ccr;
    asi_t    asi;
    //fprs_t   fprs;
    //tick_t   tick;
    pstate_t pstate;
    pil_t    pil;
    tl_t     tl;

    y.get();
    ccr.get();
    asi.get();
    pstate.get();
    pil.get();
    tl.get();

    pstate.print(), putc(' '), pil.print(), putc(' '), tl.print(), putc(' '),
    ccr.print(), putc(' '), asi.print(), putc(' '), y.print(), putc('\n');

    return CMD_NOQUIT;

} // CMD(cmd_dump_state, cg)

CMD(cmd_dump_tstate, cg)
{
    tpc_t    tpc;
    tnpc_t   tnpc;
    tt_t     tt;
    tstate_t tstate;

    tpc.get();
    tnpc.get();
    tt.get();
    tstate.get();
    tpc.print(), putc(' '), tnpc.print(), putc(' '), tt.print(),
    putc(' '), tstate.print();

    return CMD_NOQUIT;

} // CMD(cmd_dump_tstate, cg)

CMD(cmd_dump_tstate_all, cg)
{
    tl_t     tl_old;
    tl_t     tl_new;
    tpc_t    tpc;
    tnpc_t   tnpc;
    tt_t     tt;
    tstate_t tstate;
    ver_t    ver;

    tl_old.get();                                     /* Save the current TL.   */
    ver.get();                                        /* We need rev.maxtl.     */
    for(u8_t i = 1; i <= ver.ver.maxtl; i++) {        /* For every TL.          */
	tl_new.tl = i;                                  /* Set TL we want.        */
	tl_new.set();
	tpc.get();
	tnpc.get();
	tt.get();
	tstate.get();                                   /* Get the trap state.    */
	tl_new.print(), putc(' '), tpc.print(),         /* Print the results.     */
	putc('\t'), tnpc.print(), putc('\t'),
	tt.print(), putc(' '), tstate.print();
    }
    tl_old.set();                                     /* Restore original TL.   */

    return CMD_NOQUIT;

} // CMD(cmd_dump_tstate_all, cg)

CMD(cmd_dump_wstate, cg)
{
    reg_win_t reg_win;

    reg_win.get();
    reg_win.print();

    return CMD_NOQUIT;

} // CMD(cmd_dump_wstate, cg)
