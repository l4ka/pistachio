/*********************************************************************
 *                
 * Copyright (C) 2008-2010,  Karlsruhe University
 *                
 * File path:     kdb/glue/v4-x86/ipc.cc
 * Description:   
 *                
 * @LICENSE@
 *                
 * $Id:$
 *                
 ********************************************************************/
#include <debug.h>
#include <kdb/tracepoints.h>
#include <linear_ptab.h>
#include INC_API(tcb.h)
#include INC_API(space.h)
#include INC_API(schedule.h)
#include INC_ARCH(traps.h)
#include INC_ARCH(trapgate.h)
#include INC_ARCH_SA(segdesc.h)
#include INC_GLUE(idt.h)

#if defined(CONFIG_X_X86_HVM)
#include INC_GLUE(hvm.h)
#endif

const char* ctrlxfer_item_idname[ctrlxfer_item_t::id_max] = 
{
    "gpregs", "fpuregs", 
#if defined(CONFIG_X_X86_HVM)
    "cregs", "dregs", "csregs", "ssregs",
    "dsregs", "esregs", "fsregs", "gsregs",
    "trregs", "ldtrregs", "idtrregs", 
    "gdtrregs", "nonregexc", "execctl", "other",

#endif
};

const char* ctrlxfer_item_hwregname[ctrlxfer_item_t::id_max][16] = 
{
    {  "eip", "efl", "edi", "esi", "ebp", "esp", "ebx", "edx", "ecx", "eax" },
    {  NULL },
#if defined(CONFIG_X_X86_HVM)
    { "cr0", "cr0_rd", "cr0_msk", "cr2", "cr3", "cr4", "cr4_rd", "cr4_msk"},
    { "dr0", "dr1", "dr2", "dr3", "dr6", "dr7", },
    { "cs", "cs_base", "cs_limit", "cs_attr", },
    { "ss", "ss_base", "ss_limit", "ss_attr", },
    { "ds", "ds_base", "ds_limit", "ds_attr", },
    { "es", "es_base", "es_limit", "es_attr", },
    { "fs", "fs_base", "fs_limit", "fs_attr", },
    { "gs", "gs_base", "gs_limit", "gs_attr", },
    { "tr", "tr_base", "tr_limit", "tr_attr", },
    { "ldtr", "ldtr_base", "ldtr_limit", "ldtr_attr", },
    { "idtr_base", "idtr_limit", },
    { "gdtr_base", "gdtr_limit", },
    { "as", "ias", "pend_dbg", "enti", "ente", "entl", "exi", "exe", "idi", "ide" },   
    { "pinexec", "cpuexec", "excbmp"},
    { "syse_cs", "syse_eip", "syse_esp", "dbgl", "dbgh",  "tscl", "tsch", "vapic", "tprth" }
#endif
};

const char* ctrlxfer_item_t::get_idname(const word_t id)
{ 
    return ctrlxfer_item_idname[id]; 
}
    
const char* ctrlxfer_item_t::get_hwregname(const word_t id, const word_t reg)
{ 
    return ctrlxfer_item_hwregname[id][reg]; 
}

word_t arch_ktcb_t::get_ctrlxfer_reg(word_t id, word_t reg)
{	    
    word_t value;
    
    tcb_t *tcb = (tcb_t *) ((word_t) this & KTCB_MASK); 
    x86_exceptionframe_t* frame = get_user_frame(tcb);	    
    
    switch (id)
    {
    case ctrlxfer_item_t::id_gpregs:
        // GP regs
        value = frame->regs[ctrlxfer_item_t::hwregs[ctrlxfer_item_t::id_gpregs][reg]];
        break;
    case ctrlxfer_item_t::id_fpuregs:
        UNIMPLEMENTED();
        break;
#if defined(CONFIG_X_X86_HVM)
    case ctrlxfer_item_t::id_cregs ... ctrlxfer_item_t::id_otherregs:
        return get_x86_hvm_ctrlxfer_reg(id, reg);
#endif /* defined(CONFIG_X_X86_HVM) */
    default:
        value = 0;
        printf("unsupported ctrlxfer item read id %d reg %d\n", id, reg);
        enter_kdebug("ctrlxfer read");
        break;
    }
    return value;
}



void tcb_t::dump_ctrlxfer_state(bool extended)
{
    if (extended)
    {
	word_t max = 4;
#if defined(CONFIG_X_X86_HVM)
	if (get_arch()->is_hvm_enabled())
	    max += arch_ktcb_t::fault_max;
#endif
	
	printf("\nfault masks:\n");
	for (word_t fault=0; fault < max; fault++)
	{
	    if (fault % 2 == 0) printf("\t");
	    printf("%s ", fault_ctrlxfer[fault+0].string());
	    if (fault % 2 == 1) printf("\n");
	}
	printf("\n");
    }
    printf("ctrlxfer state:");
    
    
    for (word_t id = 0; id < 2; id++)
    {
	printf("\n\t%9s:", ctrlxfer_item_t::get_idname(id));
	for (word_t reg = 0; reg < ctrlxfer_item_t::num_hwregs[id]; reg++)
	{
	    if (reg && reg % 3 == 0) printf("\n\t\t  ");
	    printf("%10s: %wx  ", ctrlxfer_item_t::get_hwregname(id, reg), arch.get_ctrlxfer_reg(id, reg));
	}
    }
#if defined(CONFIG_X_X86_HVM)
    if (get_arch()->is_hvm_enabled())
    {
        for (word_t id = 3; id < ctrlxfer_item_t::id_max; id++)
	{
	    printf("\n\t%9s:", ctrlxfer_item_t::get_idname(id));
	    for (word_t reg = 0,num = 0; reg < ctrlxfer_item_t::num_hwregs[id]; reg++,num++)
	    {
		const char *regname = ctrlxfer_item_t::get_hwregname(id, reg);
		if (regname)
		{
		    if (num && num % 3 == 0) printf("\n\t\t  ");
		    printf("%10s: %0wx  ", regname, arch.get_ctrlxfer_reg(id, reg));
		}
	    }
	}
	if (extended)
	    get_arch()->dump_hvm();
    }
#endif
    printf("\n");
}

#if defined(CONFIG_X_X86_HVM)
void arch_hvm_ktcb_t::dump_hvm ()
{
    printf("\nvcpu state: %x\n\t", this);
    tcb_t *tcb = addr_to_tcb(this);
    
    // Check if this really is a VCPU.
    if (!load_vmcs())
	return;
    
    
    space_t *space = tcb->get_space();
    u64_t r;
   
    x86_segdesc_t *vgdt = (x86_segdesc_t *) 
	tcb->arch.get_ctrlxfer_reg(ctrlxfer_item_t::id_gdtrregs, 
				   ctrlxfer_item_t::gdtrreg_base);

    
    printf("\n\tvGDT-dump: gdt at gva %x ", vgdt);
    if (! space->get_hvm_space()->lookup_gphys_addr ((addr_t) vgdt, (addr_t *) &vgdt))
	printf("gpa [###]\n");
    else
    {
	printf("gpa %x\n", vgdt);
	
	for (int i = 0; i < GDT_SIZE; i++)
	{
	    if (readmem (space, vgdt + i, &r))
	    {
		x86_segdesc_t *ent = (x86_segdesc_t *)&r; 
		
		printf("\t\tGDT[%2d] = %p:%p", i, ent->x.raw[0], ent->x.raw[1]);

		printf("\t <%p,%p> ",
		       ent->x.d.base_low + (ent->x.d.base_high << 24),
		       ent->x.d.base_low + (ent->x.d.base_high << 24) +
		       (ent->x.d.g ? 0xfff |
			(ent->x.d.limit_low + (ent->x.d.limit_high << 16)) << 12 :
			(ent->x.d.limit_low + (ent->x.d.limit_high << 16))));
		printf("\tdpl=%d %d-bit ", ent->x.d.dpl, ent->x.d.d ? 32 : 16);
		if ( ent->x.d.type & 0x8 )
		    printf("\tcode %cC %cR ",
			   ent->x.d.type & 0x4 ? ' ' : '!',
			   ent->x.d.type & 0x2 ? ' ' : '!');
		else
		    printf("\tdata E%c R%c ",
			   ent->x.d.type & 0x4 ? 'D' : 'U',
			   ent->x.d.type & 0x2 ? 'W' : 'O');
		printf("\t%cP %cA",
		       ent->x.d.p ? ' ' : '!',
		       ent->x.d.type & 0x1 ? ' ' : '!');
	
		printf("\n");
	    }
	    else 
	    {
		printf("\t\t[###]");
		break;
	    }

	}
    }
    
    x86_idtdesc_t *vidt = (x86_idtdesc_t *) 
	tcb->arch.get_ctrlxfer_reg(ctrlxfer_item_t::id_idtrregs, 
				   ctrlxfer_item_t::idtrreg_base);

    printf("\n\tvIDT-dump: idt at gva %x ", vidt);
    if (! space->get_hvm_space()->lookup_gphys_addr ((addr_t) vidt, (addr_t *) &vidt))
	printf("gpa [###]\n");
    else
    {
	printf("gpa %x\n", vidt);
    for (word_t i = 0; i < IDT_SIZE; i++)
    {
	if (readmem (space, vidt + i, &r))
	{
	    x86_idtdesc_t *ent = (x86_idtdesc_t *) &r;
	    if (ent->x.d.p)
		printf("\t\t%2x -> %4x:%x, dpl=%d, %s (%x:%x)\n", i,
		       ent->x.d.sel,
		       ent->x.d.offset_low | (ent->x.d.offset_high << 16),
		       ent->x.d.dpl,
		       ((const char*[]){0,0,0,0,0,0,"INT ","TRAP"})[ent->x.d.type],
		       ent->x.raw[0], ent->x.raw[1]);
	}
	else 
	{
	    printf("\t\t[###]");
	    break;
	}
    }
    }

   
}
#endif /* defined(CONFIG_X_X86_HVM) */
