/*********************************************************************
 *                
 * Copyright (C) 2002-2005,  Karlsruhe University
 *                
 * File path:     glue/v4-ia64/context.S
 * Description:   Context management
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
 * $Id: context.S,v 1.22 2005/10/19 16:22:48 skoglund Exp $
 *                
 ********************************************************************/
#include <tcb_layout.h>
#include <asmsyms.h>

#include INC_ARCH(asm.h)
#include INC_GLUE(registers.h)
#include INC_GLUE(context_handle.h)


/**
 * Trampoline to invoke a notify procedure.  Stack pointer will upon
 * entry contian the switch context to return to, and r_rp will contain the
 * notify procedure to invoke.  We just call the procedure and restore
 * the previous switch context.
 */
BEG_PROC (ia64_notify_trampoline)
	.regstk 0,0,0,0

sp1		= r10
sp2		= r11
r_bsp		= r14
r_ip		= r15
r_rp		= r16		// must match r_rp in thread_switch()
r_cfm		= r17		// must match r_cfm in thread_switch()
r_pfs		= r18		// must match r_pfs in thread_switch()
r_rnat		= r19
r_unat		= r20
r_pr		= r21
r_psr		= r22

	// Turn all local+input registers into outputs
	dep	r_cfm = 0,r_cfm,7,7
	movl	r10 = 1f
	;;
	mov	rp = r10
	mov	ar.pfs = r_cfm
	;;
	br.ret.sptk.few rp

1:	// Perform last phase of thread switch
	mov	ar.pfs = r_pfs
	mov	rp = r_rp
	mov	ar.rsc = 3

	// Invoke notify procedure
	add	sp = -16, sp
	;;
	br.call.sptk.many rp = rp
	add	sp = 16, sp
	;;

	// Create a separate return trampoline label to be used for
	// only "restoring" initial context of a newly started thread.
	.global ia64_notify_return_trampoline
ia64_notify_return_trampoline:

	add	sp1 = SWITCH_CTX_PFS_OFFSET, sp
	add	sp2 = SWITCH_CTX_PFS_OFFSET+8, sp
	flushrs				// flush dirty registers
	mov	ar.rsc = 0		// turn off RSE
	;;

	// Load context from new frame
	ld8	r_pfs = [sp1], 16
	ld8	r_cfm = [sp2], 16
	;;
	ld8	r_ip = [sp1], 16
	ld8	r_bsp = [sp2], 16
	mov	ar.pfs = r_cfm
	;;
	ld8	r_rnat = [sp1], 16
	ld8	r_unat = [sp2], 16
	mov	ar.bspstore = r_bsp
	;;
	ld8	r_pr = [sp1]
	ld8	r_psr = [sp2], 16
	mov	ar.rnat = r_rnat
	mov	ar.unat = r_unat
	;;
	ld8	r_rp = [sp2]
	mov	rp = r_ip
	add	sp = SIZEOF_SWITCH_CONTEXT,sp
	mov	pr = r_pr, 0x1ffff
	mov	psr.l = r_psr
	;;
	srlz.d

	// Return to the last phase of the context switch
	br.ret.sptk.many rp

END_PROC (ia64_notify_trampoline)


BEG_PROC (save_context)
arg_ip		= r31
arg_num		= r30

	SAVE_EXCEPTION_CONTEXT (arg_num, ;; mov rp = arg_ip ;;)
	br.sptk.few rp

END_PROC (save_context)

BEG_PROC (load_context)
	LOAD_EXCEPTION_CONTEXT ()
	srlz.d
	;;
	rfi
END_PROC (load_context)


BEG_PROC (save_context_extint)
arg_ip		= r31
arg_ivr		= r30

	// We temporarily store arg_ivr in b1 while saving the 
	// exception context because all other general registers are
	// used for storing the context.

	SAVE_EXCEPTION_CONTEXT (12, ;; mov rp = arg_ip ;; mov b1 = arg_ivr )
	mov	ret0 = b1

	br.sptk.few rp
END_PROC (save_context_extint)



/**
 * Restore a user exception context and activate it using rfi.
 *
 * @param context	pointer to exception context
 */
BEG_PROC (activate_context)
	rsm	psr.i
	;;
	alloc	r14 = ar.pfs,1,0,0,0
	mov	sp = in0
	;;
	mov	r_KERNEL_STACK_COUNTER = 2
	;;
	LOAD_EXCEPTION_CONTEXT ()
	srlz.d
	;;
	rfi

END_PROC (activate_context)




/**
 * clobber_caller_saved_registers: clobbers all caller saved registers
 */
BEG_PROC (clobber_caller_saved_registers)
	br.ret.sptk.many rp
END_PROC (clobber_caller_saved_registers)


