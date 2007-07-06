/*********************************************************************
 *                
 * Copyright (C) 2002,  Karlsruhe University
 *                
 * File path:     kdb/arch/ia64/sal.cc
 * Description:   SAL table dumping
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
 * $Id: sal.cc,v 1.3 2003/09/24 19:05:06 skoglund Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include <kdb/cmd.h>
#include <kdb/kdb.h>
#include INC_ARCH(sal.h)

DECLARE_CMD (cmd_ia64_dumpsal, arch, 'S', "sal", "dump SAL table");

CMD(cmd_ia64_dumpsal, cg)
{
    // declared in arch/ia64/sal.cc
    extern sal_system_table_t * ia64_sal_table;

    if (ia64_sal_table == NULL || !ia64_sal_table->is_valid())
    {
	printf("SAL table invalid (%p)\n", ia64_sal_table);
	return CMD_NOQUIT;
    }

    printf("OEM id:     \"");
    for (unsigned i = 0; i < 32; i++)
    {
	if (ia64_sal_table->oem_id[i] == 0) break;
	printf("%c", ia64_sal_table->oem_id[i]);
    }

    printf("\"\nProduct id: \"");
    for (unsigned i = 0; i < 32; i++)
    {
	if (ia64_sal_table->product_id[i] == 0) break;
	printf("%c", ia64_sal_table->oem_id[i]);
    }
    printf("\"\n");

    u8_t * entry = (u8_t *) ia64_sal_table + sizeof (*ia64_sal_table);
    for (int i = 0; i < ia64_sal_table->entry_count; i++)
    {
	switch (*entry)
	{
	case 0:
	{
	    printf ("\nEntrypoint Descriptor:\n");
	    sal_entrypoint_desc_t * ed = (sal_entrypoint_desc_t *) entry;
		
	    printf ("  PAL proc at %p\n"
		    "  SAL proc at %p, gp at %p\n",
		    ed->pal_proc, ed->sal_proc, ed->sal_global_data);
	    entry += 48;
	    break;
	}

	case 1:
	{
	    //printf ("Memory Descriptor:\n");
	    entry += 32;
	    break;
	}
		
	case 2:
	{
	    printf ("\nPlatform Features Descriptor:\n");
	    sal_feature_desc_t * fd = (sal_feature_desc_t *) entry;
	    static const char * sal_features[] = {"bus lock", 
						  "IRQ redirection hint", 
						  "IPI redirection hint",
						  "?", "?", "?", "?", "?" };
	    printf("  Platform features: 0x%x [", fd->raw);
	    for (word_t i = 0; i < 8; i++)
		if (fd->raw & (1UL << i)) 
		    printf ("%s%s", sal_features[i], 
			    fd->raw >> (i + 1) ? ", " : "");
	    printf ("]\n");
	    entry += 16;
	    break;
	}

	case 3:
	{
	    printf ("\nTranslation Register Descriptor:\n");
	    sal_trans_reg_desc_t* reg = (sal_trans_reg_desc_t *) entry;
	    
	    printf ("  %s translation register # %d\n"
		    "     Covered area: %p\n"
		    "     Page size:    %d\n",
		    reg->reg_type == 0 ? "Instruction" :
		    reg->reg_type == 1 ? "Data" : "Unknown",
		    reg->reg_num,
		    reg->covered_area,
		    reg->page_size);

	    entry += 32;
	    break;
	}

	case 4:
	{
	    printf ("\nPurge Translation Cache Coherence Descriptor:\n");
	    sal_purge_tc_domain_desc_t * ptc = 
		(sal_purge_tc_domain_desc_t *) entry;
		
	    printf ("  Translation cache coherency domains: %d\n"
		    "  Domain info at %p\n",
		    ptc->num_domains, ptc->domain_info);
	    
	    entry += 16;
	    break;
	}

	case 5:
	{
	    printf ("\nAP Wakeup Descriptor:\n");
	    sal_ap_wakeup_desc_t * wd = (sal_ap_wakeup_desc_t *) entry;
	    printf ("  AP wakeup mechanism: %s (%x), IRQ vector: 0x%2x\n",
		    wd->wakeup_mechanism == 0 ? "external interrupt" :
		    "unknown", wd->wakeup_mechanism, 
		    wd->irq_vector);
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
    return CMD_NOQUIT;
}
