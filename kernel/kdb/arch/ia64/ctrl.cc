/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     kdb/arch/ia64/ctrl.cc
 * Description:   Control register access
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
 * $Id: ctrl.cc,v 1.8 2003/09/24 19:05:06 skoglund Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include <kdb/cmd.h>
#include <kdb/kdb.h>

#include INC_ARCH(psr.h)
#include INC_ARCH(cr.h)
#include INC_ARCH(ar.h)


void ia64_dump_psr (psr_t psr);


/**
 * cmd_ctrl_regs: dump control registers
 */
DECLARE_CMD (cmd_ctrl_regs, arch, 'R', "ctrlregs", "dump control registers");

CMD (cmd_ctrl_regs, cg)
{
    psr_t	psr  = get_psr ();
    psr_t	ipsr = cr_get_ipsr ();
    cr_dcr_t	dcr  = cr_get_dcr ();
    cr_pta_t	pta  = cr_get_pta ();
    cr_isr_t	isr  = cr_get_isr ();
    cr_itir_t	itir = cr_get_itir ();
    cr_ifs_t	ifs  = cr_get_ifs ();
    cr_lid_t	lid  = cr_get_lid ();
    cr_tpr_t	tpr  = cr_get_tpr ();
    cr_ivec_t   itv  = cr_get_itv ();
    cr_ivec_t   pmv  = cr_get_pmv ();
    cr_ivec_t   cmcv = cr_get_cmcv ();
    cr_ivec_t   lrr0 = cr_get_lrr0 ();
    cr_ivec_t   lrr1 = cr_get_lrr1 ();

    static char * delmode[] = {
	"INT", "dm=1", "PMI", "dm=3", "NMI", "INIT", "dm=6", "ExtINT"
    };

    printf ("Processor Status Register:\n");
    printf ("  psr:     %p ", psr); ia64_dump_psr (psr); printf ("\n");

    printf ("\nControl Registers:\n");
    printf ("  cr.ipsr: %p ", ipsr); ia64_dump_psr (ipsr); printf ("\n");
    printf ("  cr.dcr:  %p [%s%s%s%s%s%s%s%s%s%s]\n",
	    dcr.raw,
	    dcr.pp ? "pp " : "", dcr.be ? "be " : "",
	    dcr.lc ? "lc " : "", dcr.dm ? "dm " : "",
	    dcr.dp ? "dp " : "", dcr.dk ? "dk " : "",
	    dcr.dr ? "dr " : "", dcr.da ? "da " : "",
	    dcr.dd ? "dd " : "", dcr.raw ? "\b" : "");
    printf ("  ar.itc:  %p\n", ar_get_itc ());
    printf ("  cr.itm:  %p\n", cr_get_itm ());
    printf ("  cr.iva:  %p\n", cr_get_iva ());
    printf ("  cr.pta:  %p [%s, format: %s, size: %dKB, base: %p]\n",
	    pta.raw, pta.is_vhpt_enabled () ? "enabled" : "disabled",
	    pta.format () == cr_pta_t::fmt_short ? "short" : "long",
	    pta.size () >> 10, pta.base ());
    printf ("  cr.isr:  %p [code: %4x, vec: %2x, slot: %d, acc: %c%c%c "
	    "%s%s%s%s%s%s%s\b]\n",
	    isr.raw, isr.code, isr.vector, isr.instruction_slot,
	    isr.rwx & 4 ? 'r' : '~', isr.rwx & 2 ? 'w' : '~',
	    isr.rwx & 1 ? 'x' : '~',
	    isr.non_access ? "na " : "",
	    isr.speculative_load ? "sp " : "",
	    isr.register_stack ? "rs " : "",
	    isr.incomplete_reg_frame ? "ir " : "",
	    isr.nested_interruption ? "ni " : "",
	    isr.supervisor_override ? "so " : "",
	    isr.exception_deferral ? "ed " : "");
    printf ("  cr.iip:  %p\n", cr_get_iip ());
    printf ("  cr.ifa:  %p\n", cr_get_ifa ());
    printf ("  cr.itir: %p [size: %d%cB, key: %5x]\n",
	    itir.raw, itir.ps >= 30 ? itir.page_size () >> 30 :
	    itir.ps >= 20 ? itir.page_size () >> 20 : itir.page_size () >> 10,
	    itir.ps >= 30 ? 'G' : itir.ps >= 20 ? 'M' : 'K',
	    itir.protection_key ());
    printf ("  cr.iipa: %p\n", cr_get_iipa ()); 
    printf ("  cr.ifs:  %p [%svalid, size: %d (%d+%d), rot: %d, "
	    "rrb (gr: %d, fr: %d, pr: %d)]\n",
	    ifs.raw, ifs.valid ? "" : "in",
	    ifs.framesize (), ifs.locals (), ifs.outputs (), ifs.sor,
	    ifs.rrb_gr, ifs.rrb_fr, ifs.rrb_pr);
    printf ("  cr.iim:  %p\n", cr_get_iim ());
    printf ("  cr.iha:  %p\n", cr_get_iha ());
    printf ("  cr.lid:  %p [eid: %02x, id: %02x]\n", lid.raw, lid.eid, lid.id);
    printf ("  cr.tpr:  %p [mic: %d, mask %s]\n",
	    tpr.raw, tpr.mic, tpr.mmi ? "all" : "mic");
    printf ("  cr.irr:  %p %p %p %p\n",
	    cr_get_irr (3), cr_get_irr (2), cr_get_irr (1), cr_get_irr (0));
    printf ("  cr.itv:  %p [vector: %d, %smasked]\n",
	    itv.raw, itv.vector, itv.m ? "" : "not ");
    printf ("  cr.pmv:  %p [vector: %d, %smasked]\n",
	    pmv.raw, pmv.vector, pmv.m ? "" : "not ");
    printf ("  cr.cmcv: %p [vector: %d, %smasked]\n",
	    cmcv.raw, cmcv.vector, cmcv.m ? "" : "not ");
    printf ("  cr.lrr0: %p [vector: %d, %s, active %s, %s triggered, "
	    "%smasked]\n",
	    lrr0.raw, lrr0.vector, delmode[lrr0.dm],
	    lrr0.ipp == 0 ? "high" : "low",
	    lrr0.dm == 0 ? "edge" : "level",
	    lrr0.m ? "" : "not ");
    printf ("  cr.lrr1: %p [vector: %d, %s, active %s, %s triggered, "
	    "%smasked]\n",
	    lrr1.raw, lrr1.vector, delmode[lrr1.dm],
	    lrr1.ipp == 0 ? "high" : "low",
	    lrr1.dm == 0 ? "edge" : "level",
	    lrr1.m ? "" : "not ");

    printf ("\nApplication Registers:\n");
    word_t ar[8];
    asm volatile (
	"	mov %0 = ar.k0		\n"
	"	mov %1 = ar.k1		\n"
	"	mov %2 = ar.k2		\n"
	"	mov %3 = ar.k3		\n"
	"	mov %4 = ar.k4		\n"
	"	mov %5 = ar.k5		\n"
	"	mov %6 = ar.k6		\n"
	"	mov %7 = ar.k7		\n"
	:
	"=r"(ar[0]), "=r"(ar[1]), "=r"(ar[2]), "=r"(ar[3]),
	"=r"(ar[4]), "=r"(ar[5]), "=r"(ar[6]), "=r"(ar[7]));

    for (word_t i = 0; i < 8; i++)
	printf ("  ar.k%d:   %p\n", i, ar[i]);

    return CMD_NOQUIT;
}

void ia64_dump_psr (psr_t psr)
{
    printf ("[%s%s%s%s%s%s%s%s%s%s",
	    psr.be	? "be " : "",
	    psr.up	? "up " : "",
	    psr.ac	? "ac " : "",
	    psr.mfl	? "mfl " : "",
	    psr.mfh	? "mfh " : "",
	    psr.ic	? "ic " : "",
	    psr.i	? "i " : "",
	    psr.pk	? "pk " : "",
	    psr.dt	? "dt " : "",
	    psr.dfl	? "dfl " : "");
    printf ("%s%s%s%s%s%s%s%s%s%s",
	    psr.dfh	? "dfh " : "",
	    psr.sp	? "sp " : "",
	    psr.pp	? "pp " : "",
	    psr.di	? "di " : "",
	    psr.si	? "si " : "",
	    psr.db	? "db " : "",
	    psr.lp	? "lp " : "",
	    psr.tb	? "tb " : "",
	    psr.rt	? "rt " : "",
	    psr.is	? "is " : "");
    printf ("%s%s%s%s%s%s%s%s%s%s",
	    psr.mc	? "mc " : "",
	    psr.it	? "it " : "",
	    psr.id	? "id " : "",
	    psr.da	? "da " : "",
	    psr.dd	? "dd " : "",
	    psr.ss	? "ss " : "",
	    psr.ed	? "ed " : "",
	    psr.bn	? "bn " : "",
	    psr.ia	? "ia " : "",
	    psr.raw & ~((3UL << 32) | (3UL << 41)) ? "\b" : "");
    printf (" cpl=%d ri=%d]",
	    psr.cpl, psr.ri);
}
