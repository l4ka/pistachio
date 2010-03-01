/*********************************************************************
 *                
 * Copyright (C) 2006-2010,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/ipc.h
 * Description:   
 *                
 * @LICENSE@
 *                
 * $Id:$
 *                
 ********************************************************************/
#ifndef __GLUE__V4_X86__IPC_H__
#define __GLUE__V4_X86__IPC_H__

#include INC_ARCH(trapgate.h)
#include <kdb/tracepoints.h>

class arch_ctrlxfer_item_t
{ 
public:
    enum id_e {
	id_gpregs   	=   0,
 	id_fpuregs  	=   1,
#if defined(CONFIG_X_X86_HVM)
 	id_cregs    	=   2,
 	id_dregs    	=   3,
 	id_csregs   	=   4,
 	id_ssregs   	=   5,
 	id_dsregs   	=   6,
 	id_esregs   	=   7,
 	id_fsregs   	=   8,
 	id_gsregs   	=   9,
 	id_trregs   	=   10,
 	id_ldtrregs 	=   11,
 	id_idtrregs 	=   12,
 	id_gdtrregs 	=   13,
	id_nonregexc  	=   14,
	id_execctrl 	=   15,
	id_otherregs	=   16,
#endif
 	id_max, 
    };

    enum gpreg_e	{ gpreg_eip, gpreg_eflags, gpreg_edi, gpreg_esi, gpreg_ebp,
			  gpreg_esp, gpreg_ebx, gpreg_edx, gpreg_ecx, gpreg_eax };
#if defined(CONFIG_X_X86_HVM)
    enum crreg_e    	{ creg_cr0, creg_cr0_rd, creg_cr0_msk, creg_cr2, creg_cr3, creg_cr4, creg_cr4_rd, creg_cr4_msk};
    enum dreg_e    	{ dreg_dr0, dreg_dr1, dreg_dr2, dreg_dr3, dreg_dr6, dreg_dr7 };
    enum csreg_e   	{ csreg_sel, csreg_base, csreg_limit, csreg_attr };
    enum ssreg_e   	{ ssreg_sel, ssreg_base, ssreg_limit, ssreg_attr };
    enum dsreg_e   	{ dsreg_sel, dsreg_base, dsreg_limit, dsreg_attr };
    enum esreg_e   	{ esreg_sel, esreg_base, esreg_limit, esreg_attr };
    enum fsreg_e   	{ fsreg_sel, fsreg_base, fsreg_limit, fsreg_attr };
    enum gsreg_e   	{ gsreg_sel, gsreg_base, gsreg_limit, gsreg_attr };
    enum trreg_e   	{ trreg_sel, trreg_base, trreg_limit, trreg_attr };
    enum ldtrreg_e 	{ ldtrreg_sel, ldtrreg_base, ldtrreg_limit, ldtrreg_attr };
    enum idtrreg_e 	{ idtrreg_base, idtrreg_limit };
    enum gdtreg_e  	{ gdtrreg_base, gdtrreg_limit };
    enum nonregexc_e	{ activity_state, interruptibility_state, pending_debug_exc, entry_info, 
			  entry_eec, entry_ilen, exit_info, exit_eec, idt_info, idt_eec };
    enum execctrl_e	{ pin_exec_ctrl, cpu_exec_ctrl, exc_bitmap };
    enum otherreg_e	{ sysenter_cs_msr, sysenter_eip_msr, sysenter_esp_msr, debugctl_msr_low,
			  debugctl_msr_high, rdtsc_ofs_low, rdtsc_ofs_high, vapic_address, tpr_threshold };
#endif
};

x86_exceptionframe_t *get_user_frame(tcb_t *tcb);






#endif /* !__GLUE__V4_X86__IPC_H__ */
