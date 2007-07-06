/*********************************************************************
 *                
 * Copyright (C) 2006,  Karlsruhe University
 *                
 * File path:     glue/v4-mips32/exception.cc
 * Description:   Exception handling for MIPS32
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
 * $Id: exception.cc,v 1.1 2006/02/23 21:07:46 ud3 Exp $
 *                
 ********************************************************************/

#include <debug.h>
#include <kdb/console.h>

#include INC_API(tcb.h)
#include INC_API(schedule.h)
#include INC_API(kernelinterface.h)
#include INC_API(syscalls.h)

#include INC_GLUE(context.h)
#include INC_GLUE(syscalls.h)
#include INC_GLUE(exception.h)
#include INC_GLUE(config.h)

#include INC_ARCH(cp0regs.h)


bool mips32_illegal(mips32_irq_context_t *context, word_t* code);
bool mips32_break(mips32_irq_context_t *context);
static bool send_exception_ipc( word_t exc_no, word_t exc_code );
static bool send_syscall_ipc( mips32_irq_context_t *context );

void halt_user_thread( void );


extern "C" void mips32_exception_handler( mips32_irq_context_t *context ) {

    char * exception = NULL;
    bool result = false;
    word_t exc_no = CAUSE_EXCCODE_NUM(context->cause);
    word_t exc_code = 0;

    switch (exc_no) {
		
    case 4:  
        exception = "Address error (load/execute)";
        exc_code = read_32bit_cp0_register(CP0_BADVADDR); 
        break;
		
    case 5:  
        exception = "Address error (store)";
        exc_code = read_32bit_cp0_register(CP0_BADVADDR); 
        break;
		
    case 6:	 
        exception = "Bus error (instruction fetch)";	
        break;
		
    case 7:	 
        exception = "Bus error (data load/store)";	
        break;
		
    case 9:  
        exception = "Breakpoint";		
        result = mips32_break(context); 
        break;
		
    case 10: 
        exception = "Illegal instruction";	
        result = mips32_illegal(context, &exc_code);
        break;
        //case 11: exception = "Coprocessor unavailable"; result = mips32_cpu_unavail(context, &exc_code); break;
		
    case 12: 
        exception = "Arithmetic overflow";		
        break;
		
    case 13: 
        exception = "Trap exception"; 
        context->epc += 4; 
        break;
		
    case 14: 
        exception = "Virtual coherency exception (instruction)"; 
        break;
        //case 23: exception = "Watchpoint";		result = mips32_watch(context, &exc_code); break;
		
    case 31: 
        exception = "Virtual coherency exception (data)"; 
        break;
		
    default: 
        exception = "Unknown Exception"; 
        break;
    }

    if( result == false ) {

        if (!send_exception_ipc(exc_no, exc_code)) {
			
            if( EXPECT_FALSE(get_kip()->kdebug_entry != NULL) ) {
                if( exception ) {
                    printf( TXT_BRIGHT "--- KD# %s [%s] ---\n" TXT_NORMAL, "Unhandled Exception", exception );
                }
                else {
                    printf( TXT_BRIGHT "--- KD# %s [%d] ---\n" TXT_NORMAL, "Unhandled Exception", exc_no );
                }
                get_kip()->kdebug_entry( context );
            }

            halt_user_thread();
        }
    }
}


// invoked on invalid syscall (identifier) in syscall.S
extern "C" void syscall_exception(mips32_irq_context_t *context) {

    if( !send_syscall_ipc(context) ) {
        if (EXPECT_FALSE(get_kip()->kdebug_entry != NULL)) {
            printf( TXT_BRIGHT "--- KD# %s ---\n" TXT_NORMAL, "Unhandled User SYSCALL" );
            get_kip()->kdebug_entry(context);
        }
        halt_user_thread();
    }
}


#define GENERIC_SAVED_REGISTERS (EXCEPT_IPC_GEN_MR_NUM + 1)
static bool send_exception_ipc( word_t exc_no, word_t exc_code ) {
	
    // XXX has never been tested
	
    tcb_t *current = get_current_tcb();
    
    if( current->get_exception_handler().is_nilthread() ) {
        return false;
    }

    // Save message registers on the stack
    word_t saved_mr[GENERIC_SAVED_REGISTERS];
    msg_tag_t tag;

    // Save message registers.
    for( int i = 0; i < GENERIC_SAVED_REGISTERS; i++ ) {
        saved_mr[i] = current->get_mr(i);
    }
    current->set_saved_partner( current->get_partner() );
    current->set_saved_state( current->get_state() );

    // Create the message tag.
    tag.set( 0, EXCEPT_IPC_GEN_MR_NUM, EXCEPT_IPC_GEN_LABEL);
    current->set_tag( tag );

    // Create the message.
    current->set_mr( EXCEPT_IPC_GEN_MR_IP, (word_t)current->get_user_ip() );
    current->set_mr( EXCEPT_IPC_GEN_MR_SP, (word_t)current->get_user_sp() );
    current->set_mr( EXCEPT_IPC_GEN_MR_FLAGS, (word_t)current->get_user_flags() );
    current->set_mr( EXCEPT_IPC_GEN_MR_EXCEPTNO, exc_no );
    current->set_mr( EXCEPT_IPC_GEN_MR_ERRORCODE, exc_code );
    current->set_mr( EXCEPT_IPC_GEN_MR_LOCALID, current->get_local_id().get_raw() );

    //// For fast path, we need to indicate that we are doing exception ipc
    //current->resources.set_exception_ipc( current );

    // Deliver the exception IPC.
    tag = current->do_ipc( current->get_exception_handler(),
                           current->get_exception_handler(), timeout_t::never() );

    //current->resources.clear_exception_ipc( current );

    // Alter the user context if necessary.
    if( !tag.is_error() ) {
        current->set_user_ip( (addr_t)current->get_mr(EXCEPT_IPC_GEN_MR_IP) );
        current->set_user_sp( (addr_t)current->get_mr(EXCEPT_IPC_GEN_MR_SP) );
        current->set_user_flags( current->get_mr(EXCEPT_IPC_GEN_MR_FLAGS) );
    }
    else {
        printf( "Unable to deliver user exception: IPC error.\n" );
    }

    // Clean-up.
    for( int i = 0; i < GENERIC_SAVED_REGISTERS; i++ ) {
        current->set_mr( i, saved_mr[i] );
    }

    current->set_partner( current->get_saved_partner() );
    current->set_saved_partner( NILTHREAD );
    current->set_state( current->get_saved_state() );
    current->set_saved_state( thread_state_t::aborted );

    return !tag.is_error();
}


#define SYSCALL_SAVED_REGISTERS (EXCEPT_IPC_SYS_MR_NUM+1)
static bool send_syscall_ipc( mips32_irq_context_t *context ) {

    // XXX has never been tested
	
    tcb_t *current = get_current_tcb();
    if( current->get_exception_handler().is_nilthread() ) {
        printf( "Unable to deliver user exception: no exception handler.\n" );
        return false;
    }

    // Save message registers on the stack
    word_t saved_mr[SYSCALL_SAVED_REGISTERS];
    msg_tag_t tag;

    // Save message registers.
    for( int i = 0; i < SYSCALL_SAVED_REGISTERS; i++ ) {
        saved_mr[i] = current->get_mr(i);
    }
    current->set_saved_partner( current->get_partner() );
    current->set_saved_state( current->get_state() );

    // Create the message tag.
    tag.set( 0, EXCEPT_IPC_SYS_MR_NUM, EXCEPT_IPC_SYS_LABEL);
    current->set_tag( tag );

    // Create the message.
    current->set_mr( EXCEPT_IPC_SYS_MR_V0, context->v0 );
    current->set_mr( EXCEPT_IPC_SYS_MR_V1, context->v1 );
    current->set_mr( EXCEPT_IPC_SYS_MR_A0, context->a0 );
    current->set_mr( EXCEPT_IPC_SYS_MR_A1, context->a1 );
    current->set_mr( EXCEPT_IPC_SYS_MR_A2, context->a2 );
    current->set_mr( EXCEPT_IPC_SYS_MR_A3, context->a3 );
    current->set_mr( EXCEPT_IPC_SYS_MR_A4, context->t0 );
    current->set_mr( EXCEPT_IPC_SYS_MR_A5, context->t1 );
    current->set_mr( EXCEPT_IPC_SYS_MR_A6, context->t2 );
    current->set_mr( EXCEPT_IPC_SYS_MR_A7, context->t3 );

    current->set_mr( EXCEPT_IPC_SYS_MR_IP, (word_t)current->get_user_ip() );
    current->set_mr( EXCEPT_IPC_SYS_MR_SP, (word_t)current->get_user_sp() );
    current->set_mr( EXCEPT_IPC_SYS_MR_FLAGS, (word_t)current->get_user_flags() );

    //// For fast path, we need to indicate that we are doing exception ipc
    //current->resources.set_exception_ipc( current );

    // Deliver the exception IPC.
    tag = current->do_ipc( current->get_exception_handler(),
                           current->get_exception_handler(), timeout_t::never() );

    //current->resources.clear_exception_ipc( current );

    // Alter the user context if necessary.
    if( !tag.is_error() ) {
        current->set_user_ip( (addr_t)current->get_mr( EXCEPT_IPC_SYS_MR_IP ) );
        current->set_user_sp( (addr_t)current->get_mr( EXCEPT_IPC_SYS_MR_SP ) );
        current->set_user_flags( current->get_mr(EXCEPT_IPC_SYS_MR_FLAGS) );
    }
    else {
        printf( "Unable to deliver user exception: IPC error.\n" );
    }

    // Results
    context->v0 = current->get_mr( EXCEPT_IPC_SYS_MR_V0 );
    context->v1 = current->get_mr( EXCEPT_IPC_SYS_MR_V1 );
    context->a0 = current->get_mr( EXCEPT_IPC_SYS_MR_A0 );
    context->a1 = current->get_mr( EXCEPT_IPC_SYS_MR_A1 );
    context->a2 = current->get_mr( EXCEPT_IPC_SYS_MR_A2 );
    context->a3 = current->get_mr( EXCEPT_IPC_SYS_MR_A3 );
    context->t0 = current->get_mr( EXCEPT_IPC_SYS_MR_A4 );
    context->t1 = current->get_mr( EXCEPT_IPC_SYS_MR_A5 );
    context->t2 = current->get_mr( EXCEPT_IPC_SYS_MR_A6 );
    context->t3 = current->get_mr( EXCEPT_IPC_SYS_MR_A7 );

    // Clean-up.
    for( int i = 0; i < SYSCALL_SAVED_REGISTERS; i++ ) {
        current->set_mr( i, saved_mr[i] );
    }

    current->set_partner( current->get_saved_partner() );
    current->set_saved_partner( NILTHREAD );
    current->set_state( current->get_saved_state() );
    current->set_saved_state( thread_state_t::aborted );

    return !tag.is_error();
}


bool mips32_illegal(mips32_irq_context_t *context, word_t* code) {

    if (context->at == MAGIC_KIP_REQUEST) {
        space_t * space = get_current_space ();

        context->t0 = (word_t) space->get_kip_page_area ().get_base();
        context->t1 = get_kip ()->api_version;
        context->t2 = get_kip ()->api_flags;
        context->t3 = ( NULL != get_kip()->kernel_desc_ptr) ? *(word_t *)((word_t)get_kip() + get_kip()->kernel_desc_ptr ) : 0;
    }
    else {
        printf("Error: Illegal Instruction. epc =  0x%x, status = 0x%x, kernel_stack = 0x%x, ra = 0x%x\n",
               context->epc, context->status, context->sp, context->ra );
        ASSERT( !"NOTREACHED" );

    }
	
    //ASSERT(!(context->cause & CAUSE_BD));

    context->epc += 4;
    return true;	    // Succesfully handled
}


bool mips32_break(mips32_irq_context_t *context) {

    switch( context->at ) {
		
    case L4_TRAP_KPUTC:	    
        putc( (char)context->a0 );	
        break;
		
    case L4_TRAP_KGETC:
        context->v0 = getc(true);	
        break;
		
    case L4_TRAP_KGETC_NB:  
        context->v0 = getc(false);	
        break;
		
    case L4_TRAP_KDEBUG: {
        pgent_t * pg;
        pgent_t::pgsize_e pgsize;
        space_t * space = get_current_tcb ()->get_space();
        if (space == NULL) {
            space = get_kernel_space();
        }
			
        if( EXPECT_TRUE(get_kip()->kdebug_entry == NULL) ) {
            return false;
        }

        printf( TXT_BRIGHT "--- KD# " );
		    
        if( context->status & ST_UM ) {
            printf( "<User> " );
        }

        if( (!(context->status & ST_UM) ) || (space->lookup_mapping((addr_t)context->v0, &pg, &pgsize)) ) {
            printf( "%s ", (char*)context->v0 );
        }

        printf( "---\n" TXT_NORMAL );

        get_kip()->kdebug_entry(context);
        break;
    }
			
    default:
        return( false );
    }

    //ASSERT(!(context->cause & CAUSE_BD));

    context->epc += 4;

    return true;	    // Successfully handled
}


void halt_user_thread( void ) {

    tcb_t *current = get_current_tcb();
    current->set_state( thread_state_t::halted );
    current->switch_to_idle();
}
