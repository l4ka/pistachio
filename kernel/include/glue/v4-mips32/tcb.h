/*********************************************************************
 *                
 * Copyright (C) 2006,  Karlsruhe University
 *                
 * File path:     glue/v4-mips32/tcb.h
 * Description:   TCB related functions for Version 4, MIPS32
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
 * $Id: tcb.h,v 1.3 2006/10/20 21:31:55 reichelt Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_MIPS32__TCB_H__
#define __GLUE__V4_MIPS32__TCB_H__

#ifndef __API__V4__TCB_H__
#error not for stand-alone inclusion
#endif


#include INC_API(syscalls.h)		/* for sys_ipc */
#include INC_GLUE(context.h)
#include INC_GLUE(config.h)
#include INC_ARCH(cp0regs.h)

#include <debug.h>

INLINE void tcb_t::set_cpu(cpuid_t cpu) { 
    return;
}

INLINE word_t tcb_t::get_utcb_location() {
    return( myself_local.get_raw() );
}

INLINE void tcb_t::set_utcb_location(word_t utcb_location) {
    return( myself_local.set_raw( utcb_location ) );
}

/**
 * read value of message register
 * @param index number of message register
 */
INLINE word_t tcb_t::get_mr(word_t index) {
    return get_utcb()->mr[index];
}

/**
 * set the value of a message register
 * @param index number of message register
 * @param value value to set
 */
INLINE void tcb_t::set_mr(word_t index, word_t value) {
    get_utcb()->mr[index] = value;
}


/**
 * copies a set of message registers from one UTCB to another
 * @param dest destination TCB
 * @param start MR start index
 * @param count number of MRs to be copied
 */
INLINE void tcb_t::copy_mrs(tcb_t * dest, word_t start, word_t count) {
    utcb_t *my_utcb, *other_utcb;
	
    my_utcb = get_utcb();
    other_utcb = dest->get_utcb();

    for(word_t idx = start; idx < start + count; idx++) { // XXX do asm
        other_utcb->mr[idx] = my_utcb->mr[idx];
        //dest->set_mr(idx, this->get_mr(idx));
    }
}


/**
 * read value of buffer register
 * @param index number of buffer register
 */
INLINE word_t tcb_t::get_br(word_t index) {
    return get_utcb()->br[index];
}

/**
 * set the value of a buffer register
 * @param index number of buffer register
 * @param value value to set
 */
INLINE void tcb_t::set_br(word_t index, word_t value) {
    get_utcb()->br[index] = value;
}


/**
 * allocate the tcb
 * The tcb pointed to by this will be allocated.
 */
INLINE void tcb_t::allocate() {
    this->kernel_stack[0] = 0;
}


/**
 * set the address space a TCB belongs to
 * @param space address space the TCB will be associated with
 */
INLINE void tcb_t::set_space(space_t * space) {
    this->space = space;
}





/**
 * Short circuit a return path from an IPC system call.  The error
 * code TCR and message registers are already set properly.  The
 * function only needs to restore the appropriate user context and
 * return execution to the instruction directly following the IPC
 * system call.
 */

/* XXX FIXME - This does not always get stack right! */
INLINE void tcb_t::return_from_ipc (void) {
	
    mips32_irq_context_t * context = (mips32_irq_context_t *) get_stack_top () - 1;
	
    __asm__ __volatile__ (
        "move	$29, %0				\n"
        "subu	$29, 0x10			\n"		/* this is because mips32_l4sysipc_return's o32 conventions */
        "j	mips32_l4sysipc_return	\n"
        : 
        : "r" (context)
        );
}


/**
 * Short circuit a return path from a user-level interruption or
 * exception.  That is, restore the complete exception context and
 * resume execution at user-level.
 */
INLINE void tcb_t::return_from_user_interruption (void) {
    mips32_irq_context_t * context = (mips32_irq_context_t *) get_stack_top () - 1;
    __asm__ __volatile__ (
        "move	$29, %0			\n"
        "j	mips32_restore_user	\n"
        : 
        : "r" (context)
        );
}


/********************************************************************** 
 *
 *                      thread switch routines
 *
 **********************************************************************/

extern word_t K_STACK_BOTTOM;

#define mips32_initial_switch_to(d_stack, d_asid, d_space)							\
__asm__ __volatile__ (												\
    "mtc0   %[asid], "STR(CP0_ENTRYHI)"\n\t"	/* Set new ASID */						\
    "move    $29, %[stack]\n\t"			/* Install the new stack */					\
    "ori    %[stack], 4096-1\n\t"										\
    "addiu  %[stack], 1\n\t"											\
    "sw     %[stack], 0(%[stack_bot])\n\t"									\
														\
    "lw      $31,16($29)\n\r"											\
    "lw      $16,0($29)\n\t"											\
    "lw      $17,4($29)\n\t"											\
    "lw      $30,8($29)\n\t"											\
    "lw      $28,12($29)\n\t"											\
    "addiu   $29,$29,20\n\t"			/* clean stack */						\
														\
    "jr	     $31\n\t"												\
    "0:		\t\t"				/* Return Address */						\
    :														\
	: [stack] "r" (d_stack), [asid]  "r" (d_asid), [space] "r" (d_space), [stack_bot]"r" (&K_STACK_BOTTOM)	\
	: "$1", "$31"												\
);														\
__asm__ __volatile__ ("" ::: "$2", "$3", "$4", "$5", "$6", "$7" );						\
__asm__ __volatile__ ("" ::: "$8", "$9", "$10", "$23", "$24", "$25" );  					\
__asm__ __volatile__ ("" ::: "$11", "$12", "$13", "$14", "$15" );	    					\
__asm__ __volatile__ ("" ::: "$18", "$19", "$20", "$21", "$22" );


/**
 * switch to initial thread
 * @param tcb TCB of initial thread
 *
 * Initializes context of initial thread and switches to it.  The
 * context (e.g., instruction pointer) has been generated by inserting
 * a notify procedure context on the stack.  We simply restore this
 * context.
 */

INLINE void NORETURN initial_switch_to (tcb_t * tcb) {

    ASSERT( tcb->get_space() == 0 || tcb->get_space() == get_kernel_space() );
	
    //printf("\nTHREAD: (2) Inital switch to Thread. IP = 0x%x, TCB = 0x%x, Stack = 0x%x\n", *(((word_t*)tcb->stack)+2) ,tcb, (word_t)tcb->stack );
	
    mips32_initial_switch_to((word_t)tcb->stack, KERNEL_ASID, (word_t)tcb->get_space());

}


extern "C" void touch_tcb( void* tcb );

/**
 * switches to another tcb thereby switching address spaces if needed
 * @param dest tcb to switch to
 */
INLINE void tcb_t::switch_to( tcb_t * dest ) {

    word_t new_asid, wired, entryhi;
    space_t* space;	

    entryhi = ( (word_t)dest & 0xffffe000 ) | KERNEL_ASID;
    wired = ((word_t)dest >= KTCB_AREA_START) ? 1 : 0;
	
    ASSERT( (word_t)(dest->stack) < (word_t)dest + 4096  );		
    ASSERT( (void*)(dest->stack) > (void*)((word_t)dest + 500) ); // XXX stupid stack overflow test
	
    space = dest->get_space();

    if( space == NULL ) {
        space = get_kernel_space();
        new_asid = KERNEL_ASID;
    }
    else {
        if( !space->get_asid()->is_valid() ) {
            // get new asid and preemt another one if necessary
            new_asid = space->get_asid()->get();
            get_tlb()->flush( (word_t)new_asid );
        }
        else {
            new_asid = space->get_asid()->value();
        }
    }

    if( this->resource_bits )
        resources.save( this );

    /**
     * So the most indented part following is my personal wire
     * action. Its sole purpose is to keep the currently running
     * threads' kernel stack (in the TCB) hard wired in the TLB. This
     * turned out to be useful cause if it's not, the following might
     * happen: On an exception, the kernel tries to save some context
     * on the (faulting threads) stack. If it's not (TLB-)mapped
     * though, a subsequent (Tlb-miss) exception would try the same
     * thing and cause another (Tlb-miss) excpetion and so on.
     * 
     * No doubt there are other (and probably way better) solutions
     * around that (see e.g. Mips64 port). It's just: This one is
     * simple and seems to work out (so far) and I havn't gotten
     * around to change it.
     **/ 
	
    __asm__ __volatile__ (
        /* I'm the old thread :) */
        "addiu  $29,$29,-20				\n\t"
        "la     $31,0f					\n\t"	/* ra */
        "sw     $16,0($29)				\n\t"	/* save s0 */
        "sw     $17,4($29)				\n\t"	/* save s1 */
        "sw     $30,8($29)				\n\t"	/* save s8 */
        "sw     $28,12($29)				\n\t"	/* save gp */
        "sw     $31,16($29)				\n\t"	/* save ra */

        "sw	    $29, 0(%[old_stack])		\n\t"	/* Store current stack in old_stack */

        "beqz %[wired], 4f				\n\t"	/* nothing to hard-wire cause TCB is not in virt mem */

        "1:						\n\t"	/* check whether dest-TCB is in TLB (most likely) */
        "mtc0 %[entryhi], "STR(CP0_ENTRYHI)"		\n\t"
        "nop;nop;nop					\n\t"
        "tlbp						\n\t"
        "nop;nop					\n\t"

        "mfc0 $8, "STR(CP0_INDEX)"			\n\t"	/* find out where exactly it is: */
        "nop;nop;nop					\n\t"
        "beqz $8, 3f					\n\t"	/*   seems to be at the proper location already */
        "bgtz $8, 2f					\n\t"	/*   seems to be at the wrong location */
        "move $4, %[new_stack]				\n\t"	/*   seems to be not in the TLB -> put it in there  */
        "jal  touch_tcb					\n\t"	/*     this is ugly. bother. probably never happens */
        "b 1b						\n\t"
        "2:						\n\t"	/* swap TLB[0] and TLB[index] and avoid trashing */
        "tlbr						\n\t"
        "nop;nop;nop					\n\t"
        "mfc0 $10, "STR(CP0_ENTRYHI)"			\n\t"	/* get TLB[index] */
        "mfc0 $11, "STR(CP0_ENTRYLO0)"			\n\t"
        "mfc0 $12, "STR(CP0_ENTRYLO1)"			\n\t"
        "mtc0 $0, "STR(CP0_INDEX)"			\n\t"
        "nop;nop					\n\t"
        "tlbr						\n\t"
        "mfc0 $13, "STR(CP0_ENTRYHI)"			\n\t"	/* get TLB[0] */
        "mfc0 $14, "STR(CP0_ENTRYLO0)"			\n\t"
        "mfc0 $15, "STR(CP0_ENTRYLO1)"			\n\t"
        "mtc0 %[invalid], "STR(CP0_ENTRYHI)"		\n\t"	/* invalidate TLB[0] and avoid a temp duplicate entry */
        "mtc0 $0, "STR(CP0_ENTRYLO0)"			\n\t"	/*   XXX not sure whether this is necessary */
        "mtc0 $0, "STR(CP0_ENTRYLO1)"			\n\t"
        "nop;nop					\n\t"
        "tlbwi; nop;nop;nop				\n\t"
        "mtc0 $8, "STR(CP0_INDEX)"			\n\t"	/* write TLB[index] */
        "mtc0 $13, "STR(CP0_ENTRYHI)"			\n\t"
        "mtc0 $14, "STR(CP0_ENTRYLO0)"			\n\t"
        "mtc0 $15, "STR(CP0_ENTRYLO1)"			\n\t"
        "nop;nop					\n\t"
        "tlbwi; nop;nop;nop				\n\t"
        "mtc0 $0, "STR(CP0_INDEX)"			\n\t"	/* write TLB[0] */
        "mtc0 $10, "STR(CP0_ENTRYHI)"			\n\t"
        "mtc0 $11, "STR(CP0_ENTRYLO0)"			\n\t"
        "mtc0 $12, "STR(CP0_ENTRYLO1)"			\n\t"
        "nop;nop					\n\t"
        "tlbwi; nop;nop;nop				\n\t"
        "3:						\n\t"
        "mtc0 %[wired], "STR(CP0_WIRED)"		\n\t"
        "b 5f						\n\t"
        "4:						\n\t"
        "mtc0 $0, "STR(CP0_WIRED)"			\n\t"
        "nop						\n\t"
        "5:						\n\t"
        "nop;nop					\n\t"
        "mtc0   %[asid], "STR(CP0_ENTRYHI)"		\n\t"	/* Set new ASID */
        "move   $29, %[new_stack]			\n\t"	/* Install the new stack */
        "or	    %[new_stack], 4096-1		\n\t"
        "addiu  %[new_stack], 1				\n\t"
        "sw     %[new_stack], 0(%[stack_bot])		\n\t"	/* I'm the new thread :) */
        "lw     $31,16($29)				\n\t"	/* load ra */
        "lw     $16,0($29)				\n\t"	/* load s0 */
        "lw     $17,4($29)				\n\t"	/* load s1 */
        "lw     $30,8($29)				\n\t"   /* load s8 */
        "lw     $28,12($29)				\n\t"	/* load gp */
        "addiu  $29,$29,20				\n\t"
        "jr	    $31					\n\t"
        "0:						\n\t"	/* Return Address */

        :: [old_stack] "r" ((word_t)&this->stack), [new_stack] "r" ((word_t)dest->stack), [asid] "r"(new_asid),
        [stack_bot]"r" (&K_STACK_BOTTOM), [wired] "r"(wired), [entryhi] "r"(entryhi), [invalid] "r"(get_tlb()->get_invalid() )
        : "$1", "$4","$8", "$10", "$11", "$12", "$13", "$14", "$15", "$16", "$31"
	);

    __asm__ __volatile__ ("" ::: "$2", "$3", "$4", "$5", "$6", "$7", "$8", "$9", "$10", "$23", "$24", "$25", "$11", "$12",
                          "$13", "$14", "$15", "$18", "$19", "$20", "$21", "$22" );

    if( this->resource_bits )
        resources.load(this);
}



INLINE word_t *tcb_t::get_stack_top(void) {
    return (word_t *)((char*)this + KTCB_SIZE);
}



/**
 * intialize stack for given thread
 */
INLINE void tcb_t::init_stack() {
	
    mips32_irq_context_t * context = (mips32_irq_context_t*)get_stack_top() - 1;

    this->stack = (word_t *) context;/* Update new stack position */

    for (word_t* t = (word_t *) context; t < get_stack_top(); t++) {
        *t = 0;
    }

}




/**********************************************************************
 *
 *                        notification functions
 *
 **********************************************************************/

/**
 * create stack frame to invoke notify procedure
 * @param func notify procedure to invoke
 *
 * Create a stack frame in TCB so that next thread switch will invoke
 * the indicated notify procedure.
 */
INLINE void tcb_t::notify (void (*func)()) {
	
    //printf("\nTHREAD: (1) Create new Thread ( tcb_t::notify ). IP = 0x%x, TCB = 0x%x, UserStack = 0x%x\n", func, this, this->stack);
    mips32_switch_stack_t* stack = reinterpret_cast< mips32_switch_stack_t*>(this->stack);
    stack--;
    stack->s8 = (word_t) func;
    stack->ra = (word_t) mips32_return_from_notify0;
    this->stack = reinterpret_cast<word_t*>(stack);
}

/**
 * create stack frame to invoke notify procedure
 * @param func notify procedure to invoke
 * @param arg1 1st argument to notify procedure
 *
 * Create a stack frame in TCB so that next thread switch will invoke
 * the indicated notify procedure.
 */
INLINE void tcb_t::notify (void (*func)(word_t), word_t arg1) {
    //printf("\nTHREAD: (1.1) Create new Thread ( tcb_t::notify ). IP = 0x%x, TCB = 0x%x, Stack = 0x%x\n", func, this, this->stack);

    mips32_switch_stack_t* stack = reinterpret_cast< mips32_switch_stack_t*>(this->stack);
    stack--;
    stack->s8 = (word_t) func;
    stack->s0 = arg1;
    stack->ra = (word_t) mips32_return_from_notify1;
    this->stack = reinterpret_cast<word_t*>(stack);
}

INLINE void tcb_t::notify (void (*func)(word_t, word_t), word_t arg1, word_t arg2) {
    //printf("\nTHREAD: (1.2) Create new Thread ( tcb_t::notify ). IP = 0x%x, TCB = 0x%x, UserStack = 0x%x\n", func, this, this->stack);
    mips32_switch_stack_t* stack = reinterpret_cast< mips32_switch_stack_t*>(this->stack);
    stack--;
    stack->s8 = (word_t) func;
    stack->s0 = arg1;
    stack->s1 = arg2;
    stack->ra = (word_t) mips32_return_from_notify2;
    this->stack = reinterpret_cast<word_t*>(stack);
}




/**********************************************************************
 * 
 *            access functions for ex-regs'able registers
 *
 **********************************************************************/

/**
 * read the user-level instruction pointer
 * @return	the user-level stack pointer
 */
INLINE addr_t tcb_t::get_user_ip()
{
    mips32_irq_context_t * context = (mips32_irq_context_t *) get_stack_top () - 1;
    return (addr_t) (context)->epc;
}

/**
 * read the user-level stack pointer
 * @return	the user-level stack pointer
 */
INLINE addr_t tcb_t::get_user_sp()
{
    mips32_irq_context_t* context = (mips32_irq_context_t *) get_stack_top () - 1;
    return (addr_t) (context)->sp;
}

/**
 * set the user-level instruction pointer
 * @param ip	new user-level instruction pointer
 */
INLINE void tcb_t::set_user_ip(addr_t ip)
{
    mips32_irq_context_t * context = (mips32_irq_context_t *) get_stack_top () - 1;
    context->epc = (word_t)ip;
}

/**
 * set the user-level stack pointer
 * @param sp	new user-level stack pointer
 */
INLINE void tcb_t::set_user_sp(addr_t sp)
{
    mips32_irq_context_t * context = (mips32_irq_context_t *) get_stack_top () - 1;
    context->sp = (word_t)sp;
}


/**
 * read the user-level flags (one word)
 * @return	the user-level flags
 */
INLINE word_t tcb_t::get_user_flags (void)
{
    mips32_irq_context_t * context = (mips32_irq_context_t *) get_stack_top () - 1;
    return context->status;
}

/**
 * set the user-level flags
 * @param flags	new user-level flags
 */
INLINE void tcb_t::set_user_flags (const word_t flags)
{
    UNIMPLEMENTED();
}

/**********************************************************************
 *
 *                  copy-area related functions
 *
 **********************************************************************/

/**
 * Enable copy area for current thread.
 *
 * @param dst		destination TCB for IPC copy operation
 * @param s		source address
 * @param d		destination address
 */
INLINE void tcb_t::adjust_for_copy_area (tcb_t * dst, addr_t * s, addr_t * d) {
    resources.enable_copy_area( this, s, dst, d );
}

/**
 * Release copy area(s) for current thread.
 */
INLINE void tcb_t::release_copy_area (void) {
    resources.release_copy_area( this );
}

/**
 * Retrieve the real address associated with a copy area address.
 *
 * @param addr		address within copy area
 *
 * @return address translated into a regular user-level address
 */
INLINE addr_t tcb_t::copy_area_real_address (addr_t addr) {
    return( resources.copy_area_real_address( addr ) );
}

/**********************************************************************
 *
 *                        global tcb functions
 *
 **********************************************************************/

/**
 * Locate current TCB by using current stack pointer and return it.
 */
INLINE tcb_t * get_current_tcb (void)
{
    register word_t stack_var asm("$29");
    return( (tcb_t *)(stack_var & KTCB_MASK) );
};


/**
 * invoke an IPC from within the kernel
 *
 * @param to_tid destination thread id
 * @param from_tid from specifier
 * @param timeout IPC timeout
 * @return IPC message tag (MR0)
 */
INLINE msg_tag_t tcb_t::do_ipc (threadid_t to_tid, threadid_t from_tid, timeout_t timeout) {

    msg_tag_t tag;

    sys_ipc( to_tid, from_tid, timeout );
    tag.raw = get_mr( 0 );

    return tag;
}

INLINE tcb_t * addr_to_tcb (addr_t addr){
    return (tcb_t *) ((word_t) addr & KTCB_MASK);
}

/**********************************************************************
 *
 *                  architecture-specific functions
 *
 **********************************************************************/

/**
 * initialize architecture-dependent root server properties based on
 * values passed via KIP
 * @param space the address space this server will run in   
 * @param ip the initial instruction pointer           
 * @param sp the initial stack pointer
 */
INLINE void tcb_t::arch_init_root_server (space_t * space, word_t ip, word_t sp)
{ 
}

#endif /* !__GLUE__V4_MIPS32__TCB_H__ */
