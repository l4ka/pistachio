/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     arch/ia64/sal.cc
 * Description:   IA-64 SAL management
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
 * $Id: sal.cc,v 1.13 2003/09/24 19:05:27 skoglund Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include INC_ARCH(sal.h)
#include INC_ARCH(pal.h)
#include INC_ARCH(tlb.h)
#include INC_ARCH(trmap.h)
#include INC_PLAT(system_table.h)


sal_system_table_t * ia64_sal_table = NULL;

static addr_t __ia64_sal_entry[2];

/**
 * Entry point for SAL procedure calls.
 */
ia64_sal_func_t ia64_sal_entry = (ia64_sal_func_t) &__ia64_sal_entry;



void SECTION (".init")
init_sal (void)
{
    /*
     * Locate the SAL table.
     */

    ia64_sal_table = (sal_system_table_t *)
	efi_config_table.find_table (SAL_SYSTEM_TABLE_GUID);
    ia64_sal_table = phys_to_virt (ia64_sal_table);

    /*
     * Map SAL table if not already mapped.
     */

    if (! dtrmap.is_mapped ((addr_t) ia64_sal_table))
    {
	translation_t tr (1, translation_t::write_back, 1, 1, 0,
			  translation_t::rwx,
			  virt_to_phys (ia64_sal_table), 0);
	dtrmap.add_map (tr, ia64_sal_table, HUGE_PGSIZE, 0);
    }

    /*
     * Sanity check.
     */

    if (! ia64_sal_table->is_valid ())
    {
	printf ("SAL table not valid.\n");
	enter_kdebug ("no sal");
    }

    /*
     * Extract the needed SAL table entries.
     */

    u8_t * entry = (u8_t *) ia64_sal_table + sizeof (*ia64_sal_table);
    for (int i = 0; i < ia64_sal_table->entry_count; i++)
    {
	switch (*entry)
	{
	case 0:
	{
	    sal_entrypoint_desc_t * ed = (sal_entrypoint_desc_t *) entry;

	    TRACE_INIT ("PAL proc at %p\n"
			"SAL proc at %p, gp at %p\n",
			ed->pal_proc, ed->sal_proc, ed->sal_global_data);

	    ia64_pal_entry = ed->pal_proc;
	    __ia64_sal_entry[0] = phys_to_virt ((addr_t) ed->sal_proc);
	    __ia64_sal_entry[1] = phys_to_virt ((addr_t) ed->sal_global_data);


	    /*
	     * Map SAL code and data.
	     */

	    if (! itrmap.is_mapped (__ia64_sal_entry[0]))
	    {
		translation_t tr (1, translation_t::write_back, 1, 1, 0,
				  translation_t::rwx, ed->sal_proc, 0);
		itrmap.add_map (tr, __ia64_sal_entry[0], HUGE_PGSIZE, 0);
	    }
	    if (! dtrmap.is_mapped (__ia64_sal_entry[1]))
	    {
		translation_t tr (1, translation_t::write_back, 1, 1, 0,
				  translation_t::rwx, ed->sal_global_data, 0);
		dtrmap.add_map (tr, __ia64_sal_entry[1], HUGE_PGSIZE, 0);
	    }

	    entry += 48;
	    break;
	}

	case 1:
	    entry += 32;
	    break;

	case 2:
	    entry += 16;
	    break;
	case 3:
	    entry += 32;
	    break;
	case 4:
	    entry += 16;
	    break;
	case 5:
	{
#if defined(CONFIG_SMP)
	    extern word_t ap_wakeup_vector;
	    sal_ap_wakeup_desc_t * wd = (sal_ap_wakeup_desc_t*) entry;
	    TRACE_INIT("AP wakeup mechanism: %s (%x), IRQ vector: 0x%2x\n",
		       wd->wakeup_mechanism == 0 ? "external interrupt" :
		       "unknown", wd->wakeup_mechanism, 
		       wd->irq_vector);
	    ap_wakeup_vector = wd->irq_vector;
#endif
	    entry += 16;
	    break;
	}

	default:
	    /*
	     * Unknown SAL entries will always be located at end.
	     */
	    printf ("Unknown SAL entry: %d\n", *entry);
	    break;
	}
    }

    if (! ia64_pal_entry)
	panic ("PAL proc not found\n");

    if (! __ia64_sal_entry[0])
	panic ("SAL proc not found\n");
}
