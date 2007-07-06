/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     kdb/arch/ia64/tlb.cc
 * Description:   TLB and TR management commands
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
 * $Id: tlb.cc,v 1.11 2003/09/24 19:05:06 skoglund Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include <kdb/cmd.h>
#include <kdb/kdb.h>
#include INC_ARCH(pal.h)
#include INC_ARCH(tlb.h)
#include INC_ARCH(trmap.h)
#include INC_ARCH(rr.h)


static void dump_trs (word_t type);

DECLARE_CMD_GROUP (ia64_tlb);


/**
 * IA-64 TLB management.
 */
DECLARE_CMD (cmd_ia64_tlb, arch, 't', "tlb", "TLB and TR management");

CMD(cmd_ia64_tlb, cg)
{
    return ia64_tlb.interact (cg, "tlb");
}


/**
 * Dump IA-64 instruction TRs.
 */
DECLARE_CMD (cmd_ia64_itrs, ia64_tlb, 'i', "itrs", "dump instruction TRs");

CMD(cmd_ia64_itrs, cg)
{
    printf ("Instruction translation registers:\n");
    dump_trs (0);
    return CMD_NOQUIT;
}

/**
 * Dump in-memory ITR map.
 */
DECLARE_CMD (cmd_ia64_itrmap, ia64_tlb, 'I', "itrmap",
	     "dump in-memory ITR map");

CMD(cmd_ia64_itrmap, cg)
{
    printf ("In-memory instruction translation mappings:\n");
    itrmap.dump ();
    return CMD_NOQUIT;
}


/**
 * Dump IA-64 data TRs.
 */
DECLARE_CMD (cmd_ia64_dtrs, ia64_tlb, 'd', "dtrs", "dump data TRs");

CMD(cmd_ia64_dtrs, cg)
{
    printf ("Data translation registers:\n");
    dump_trs (1);
    return CMD_NOQUIT;
}

/**
 * Dump in-memory DTR map.
 */
DECLARE_CMD (cmd_ia64_dtrmap, ia64_tlb, 'D', "dtrmap",
	     "dump in-memory DTR map");

CMD(cmd_ia64_dtrmap, cg)
{
    printf ("In-memory data translation mappings:\n");
    dtrmap.dump ();
    return CMD_NOQUIT;
}


void SECTION(SEC_KDEBUG) dump_trs (word_t type)
{
    pal_vm_summary_t info;
    pal_status_e status;

    /*
     * Get number of implemented registers.
     */

    if ((status = pal_vm_summary (&info)) != PAL_OK)
    {
	printf ("Error: PAL_VM_SUMMARY => %d\n", status);
	return;
    }

    word_t max_entry = (type == 0) ? info.max_itr_entry : info.max_dtr_entry;

    struct {
	translation_t	tr;
	word_t		itir;
	word_t		ifa;
	rr_t		rr;
    } translation;
    pal_vm_tr_read_t	valbits;

    static const char * rights[] = {
	"ro", "rx", "rw", "rwx",
	"ro/rw", "rx/rwx", "rwx/rw", "xp/rx"
    };

    static const char * memattrib[] = {
	"WB ", "~~~", "~~~", "~~~",
	"UC ", "UCe", "WC ", "NaT"
    };

    /*
     * Dump translations.
     */

    printf ("  %9s%16s    %16s  PAD  Priv  PSize  Key    RID    Mem  Access\n",
	    "", "Virt", "Phys");

    for (word_t i = 0; i <= max_entry; i++)
    {
	status = pal_vm_tr_read (i, type, &translation, &valbits);
	if (status != PAL_OK)
	{
	    printf ("Error: PAL_VM_TR_READ => %d\n", status);
	    continue;
	}

	word_t pgsz = (translation.itir & 0xff) >> 2;

	printf ("  %ctr[%d]%s  %p => %p  %c%c%c  %4d  ",
		(type == 0 ? 'i' : 'd'), i, (i < 10 ? " " : ""),
		translation.ifa & ~((1 << 12) - 1),
		translation.tr.phys_addr (),
		translation.tr.is_present ()  ? 'p' : '~',
		translation.tr.is_accessed () ? 'a' : '~',
		translation.tr.is_dirty ()    ? 'd' : '~',
		translation.tr.privilege_level ());
	printf ("%3d%cB  %5x  %5x  %s  %s\n",
		1 << (pgsz - ((pgsz >= 30) ? 30 : (pgsz >= 20) ? 20 : 10)),
		((pgsz >= 30) ? 'G' : (pgsz >= 20) ? 'M' : 'K'),
		(translation.itir >> 8) & 0xfffff,
		translation.rr.region_id (),
		(valbits.memory_attributes_valid ? 
		 memattrib[translation.tr.memattrib ()] : "~~~"),
		(valbits.access_right_valid ? 
		 rights[translation.tr.access_rights ()] : "~"));
    }

}


void print_size (word_t k) __attribute__ ((noinline));
void print_size (word_t k)
{
    if (k >= 60)
	printf ("%dbits ", k);
    else
	printf (" %d%c",
		1UL << (k - ((k>=50) ? 50 : (k>=40) ? 40 : (k>=30) ? 30 :
			     (k>=20) ? 20 : 10)),
		(k>=50) ? 'P' : (k>=40) ? 'T' : (k>=30) ? 'G' :
		(k>=20) ? 'M' : 'K');
}


/**
 * cmd_tlb_info: dump TLB information
 */
DECLARE_CMD (cmd_tlb_info, ia64_tlb, 't', "info", "dump TLB information");

CMD(cmd_tlb_info, cg)
{
    pal_vm_summary_t summary;
    pal_vm_info_t info;
    pal_status_e status;

    if ((status = pal_vm_summary (&summary)) != PAL_OK)
    {
	printf ("Error: PAL_VM_SUMMARY => %d\n", status);
	return CMD_NOQUIT;
    }

    printf ("TLB info:\n");
    pal_vm_info_t::type_e type = pal_vm_info_t::code;

    do {

	for (word_t i = 0; i < summary.num_tc_levels; i++)
	{
	    if ((status = pal_vm_info (i, type, &info)) != PAL_OK)
		continue;

	    printf ("  %cTLB L%d: %4d entries ",
		    type == pal_vm_info_t::code ? 'I' : 'D',
		    i+1,  info.num_entries);

	    if (info.num_ways == 1)
		printf ("(direct)");
	    else
		printf ("(%d-way)", info.num_ways);

	    if (info.unified)
		printf (" UNIFIED");
	    if (info.unified)
		printf (" OPTIMIZED");

	    printf (" (sizes:");
	    for (word_t k = 12; k < 64; k++)
		if (info.page_size_mask & (1UL << k))
		    print_size (k);
	    printf (")\n");
	}

	if (type == pal_vm_info_t::code)
	    type = pal_vm_info_t::data;
	else
	    type = pal_vm_info_t::none;

    } while (type != pal_vm_info_t::none);

    printf ("\nTR info:\n");
    printf ("  Num ITRs: %3d\n  Num DTRs: %3d\n",
	    summary.max_itr_entry+1, summary.max_dtr_entry+1);

    word_t insertable, purgeable;
    if ((status = pal_vm_page_size (&insertable, &purgeable)) != PAL_OK)
    {
	printf ("Error: PAL_VM_PAGE_SIZE => %d\n", (long) status);
	return CMD_NOQUIT;
    }

    printf ("\nPage size info:\n  Insertable:");
    for (word_t k = 12; k < 64; k++)
	if (insertable & (1UL << k))
	    print_size (k);
    printf ("\n  Purgeable: ");
    for (word_t k = 12; k < 64; k++)
	if (purgeable & (1UL << k))
	    print_size (k);
    printf ("\n");
    return CMD_NOQUIT;
}
