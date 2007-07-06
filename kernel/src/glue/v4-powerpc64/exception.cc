/****************************************************************************
 *
 * Copyright (C) 2003-2004,  National ICT Australia (NICTA)
 *
 * File path:	glue/v4-powerpc64/exception.cc
 * Description:	PowerPC64 exception handlers.
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
 * $Id: exception.cc,v 1.18 2006/11/17 17:04:18 skoglund Exp $
 *
 ***************************************************************************/

#include <debug.h>
#include <kdb/tracepoints.h>
#ifdef CONFIG_DEBUG
#include <kdb/console.h>
#endif
#include INC_ARCH(msr.h)
#include INC_ARCH(except.h)

#include INC_PLAT(prom.h)

#include INC_API(tcb.h)
#include INC_API(kernelinterface.h)

#include INC_GLUE(syscalls.h)
#include INC_GLUE(pghash.h)
#include INC_GLUE(pgent_inline.h)
#include INC_GLUE(exception.h)

#if CONFIG_PLAT_OFPOWER3
#include INC_ARCH(segment.h)
#endif

DECLARE_TRACEPOINT(except_dsi_cnt);
DECLARE_TRACEPOINT(except_isi_cnt);


#define GENERIC_SAVED_REGISTERS (EXCEPT_IPC_GEN_MR_NUM_ADDRESS+1)

static bool send_exception_ipc( word_t exc_no, word_t exc_code, bool with_address, word_t address )
{
    tcb_t *current = get_current_tcb();
    if( current->get_exception_handler().is_nilthread() )
    {
	printf( "Unable to deliver user exception: no exception handler.\n" );
	return false;
    }

    // Save message registers on the stack
    word_t saved_mr[GENERIC_SAVED_REGISTERS];
    msg_tag_t tag;

    // Save message registers.
    for( int i = 0; i < GENERIC_SAVED_REGISTERS; i++ )
	saved_mr[i] = current->get_mr(i);
    current->set_saved_partner( current->get_partner() );
    current->set_saved_state( current->get_state() );

    // Create the message tag.
    tag.set( 0, with_address ? EXCEPT_IPC_GEN_MR_NUM_ADDRESS :  EXCEPT_IPC_GEN_MR_NUM,
		    EXCEPT_IPC_GEN_LABEL);
    current->set_tag( tag );

    // Create the message.
    current->set_mr( EXCEPT_IPC_GEN_MR_IP, (word_t)current->get_user_ip() );
    current->set_mr( EXCEPT_IPC_GEN_MR_SP, (word_t)current->get_user_sp() );
    current->set_mr( EXCEPT_IPC_GEN_MR_FLAGS, (word_t)current->get_user_flags() );
    current->set_mr( EXCEPT_IPC_GEN_MR_EXCEPTNO, exc_no );
    current->set_mr( EXCEPT_IPC_GEN_MR_ERRORCODE, exc_code );
    current->set_mr( EXCEPT_IPC_GEN_MR_LOCALID, current->get_local_id().get_raw() );
    if (with_address)
	current->set_mr( EXCEPT_IPC_GEN_MR_ERRORADDRESS, address );

    // Deliver the exception IPC.
    tag = current->do_ipc( current->get_exception_handler(),
	    current->get_exception_handler(), timeout_t::never() );

    // Alter the user context if necessary.
    if( !tag.is_error() )
    {
	current->set_user_ip( (addr_t)current->get_mr(EXCEPT_IPC_GEN_MR_IP) );
	current->set_user_sp( (addr_t)current->get_mr(EXCEPT_IPC_GEN_MR_SP) );
	current->set_user_flags( current->get_mr(EXCEPT_IPC_GEN_MR_FLAGS) );
    }
    else
	printf( "Unable to deliver user exception: IPC error.\n" );

    // Clean-up.
    for( int i = 0; i < GENERIC_SAVED_REGISTERS; i++ )
	current->set_mr( i, saved_mr[i] );

    current->set_partner( current->get_saved_partner() );
    current->set_saved_partner( NILTHREAD );
    current->set_state( current->get_saved_state() );
    current->set_saved_state( thread_state_t::aborted );

    return !tag.is_error();
}

INLINE void halt_user_thread( void )
{
    tcb_t *current = get_current_tcb();

    current->set_state( thread_state_t::halted );
    current->switch_to_idle();
}

/* except_return() short circuits the C code return path.
 * We declare the exception handlers as noreturn, to avoid
 * the C prolog (which redundantly spills registers which the assembler
 * path already spills).
 */
#define except_return()			\
do {					\
    asm volatile (			\
	"mtlr %0 ;"			\
	"ld %%r1, 0(%1) ;"		\
	"blr ;"				\
	:				\
	: "r" (__builtin_return_address(0)), \
	  "b" (__builtin_frame_address(0)) \
    );					\
    while(1);				\
} while(0)

extern "C" void ppc64_except_unhandled( word_t vect, powerpc64_irq_context_t *context )
{
    char * vector;
    bool with_address = false, deliver = false;
    word_t address = 0, exc_code = 0;

    switch(vect)
    {
    case 0x0100: vector = "System Reset"; break;
    case 0x0200: vector = "Machine Check"; break;
    case 0x0300: vector = "Data Access";
		 with_address = true;
		 address = context->dar;
		 exc_code = context->dsisr;
		 deliver = true;
		 break;
    case 0x0400: vector = "Instruction Access"; deliver = true; break;
    case 0x0500: vector = "Hardware Int"; break;
    case 0x0600: vector = "Alignment";
		 with_address = true;
		 address = context->dar;
		 exc_code = context->dsisr;
		 deliver = true;
		 break;
    case 0x0700: vector = "Program Check"; deliver = true; break;
    case 0x0800: vector = "FPU Unavailable"; break;
    case 0x0900: vector = "Decrementer"; break;
    case 0x0c00: vector = "System Call"; break;
    case 0x0d00: vector = "Trace"; deliver = true; break;
    case 0x0f00: vector = "Performance"; deliver = true; break;
    case 0x1300: vector = "Instruction Break"; deliver = true; break;
    case 0x1500: vector = "Soft Patch"; deliver = true; break;
    case 0x1600: vector = "Maintenance"; deliver = true; break;
    case 0x2000: vector = "Instrumentation"; deliver = true; break;
    default    : vector = "Unknown"; break;
    }

    if ( !deliver || !send_exception_ipc( vect, exc_code, with_address, address ) )
    {
	printf( "\n-- KD# %s exception --\n", vector );

	if( EXPECT_FALSE(get_kip()->kdebug_entry != NULL) )
	    get_kip()->kdebug_entry(context);

	halt_user_thread();
    }

    except_return();
}

/* Data storage interrupt (Data TLB miss) */
extern "C" void dsi_handler( word_t dar, word_t dsisr, powerpc64_irq_context_t *context )
{
    tcb_t *tcb = get_current_tcb();

    bool is_kernel = ppc64_is_kernel_mode(context->srr1);

    TRACEPOINT( except_dsi_cnt,
		printf( "[%p%s] Data exception @ %p from %p\n", tcb,
		is_kernel ? " (kernel)" : "", dar, context->srr0 ) );

#ifdef CONFIG_DEBUG
    // Do we have a DABR hit?
    if( EXPECT_FALSE( EXCEPT_IS_DSI_DABR_MATCH(dsisr) ) )
    {
	printf( "Data Address Break Point @ %p\n", dar );
	get_kip()->kdebug_entry( context );
	except_return();
    }
#endif

    // If fault is in the kernel area, just map in a kernel page
    if ( EXPECT_FALSE( is_kernel && space_t::is_kernel_area ((addr_t)dar) ))
    {
	//TRACEF( "kernel fault\n" );
	pgent_t pg;
        space_t *space = get_kernel_space();
#ifdef CONFIG_POWERPC64_LARGE_PAGES
	pgent_t::pgsize_e size = pgent_t::size_16m;
#else
	pgent_t::pgsize_e size = pgent_t::size_4k;
#endif

	/* Create a dummy page table entry */
	pg.set_entry( space, size, virt_to_phys((addr_t)dar),
		      7, pgent_t::l4default, true );

	/* Insert the kernel mapping, bolted */
	get_pghash()->insert_mapping( space, (addr_t)dar, &pg, size, true );

	//TRACEF( "kernel fault done\n" );
	except_return();
    }

    space_t *space = tcb->get_space();
    if (!space) space = get_kernel_space();

    // Use kernel space if we have kernel fault in TCB area or
    // when no space is set (e.g., running on the idle thread)
    if ( EXPECT_FALSE( is_kernel && space->is_tcb_area ((addr_t)dar) )
			    || space == NULL)
    {
        space = get_kernel_space();
	//TRACEF( "kernel space\n" );
    }

    ASSERT( !(is_kernel && space->is_cpu_area ((addr_t)dar)) );

    // Do we have a page hash miss?
    if( EXPECT_TRUE( EXCEPT_IS_DSI_MISS(dsisr) ) )
    {

	// Is the page hash miss in the copy area?
	if( EXPECT_FALSE(space->is_copy_area((addr_t)dar)) )
	{
	    enter_kdebug( "Page table needs to be fixed for copy area" );
	    // Resolve the fault using the partner's address space!
	    tcb_t *partner = space->get_tcb( tcb->get_partner() );
	    if( partner )
	    {
		addr_t real_fault = tcb->copy_area_real_address( (addr_t)dar );
		if( partner->get_space()->handle_hash_miss(real_fault) )
	    	    except_return();
	    }
	}

	// Normal page hash miss.
	if( EXPECT_TRUE(space->handle_hash_miss((addr_t)dar)) )
	{
	    //TRACEF("found - returning\n");
	    except_return();
	}
    }
    else if( EXCEPT_IS_DSI_FAULT(dsisr) )
    {
	// Page found, but access denied
	if( EXPECT_TRUE(space->handle_protection_fault( (addr_t)dar, true )) )
	{
	    //TRACEF("handled - returning\n");
	    except_return();
	}
    } else {
	if ( !send_exception_ipc( 0x300, dsisr, true, dar) )
	{
	    printf( "\n-- KD# data address exception --\n" );
	    if( EXPECT_FALSE(get_kip()->kdebug_entry != NULL) )
		get_kip()->kdebug_entry(context);
	}
    }

    //TRACEF("handle pagefault\n");
    space->handle_pagefault( (addr_t)dar, (addr_t)context->srr0, 
		    EXCEPT_IS_DSI_WRITE(dsisr) ?  space_t::write : space_t::read,
		    ppc64_is_kernel_mode(context->srr1) );

    //TRACEF("handled - returning\n");
    except_return();
}

extern "C" void program_check_handler( word_t vect, powerpc64_irq_context_t *context )
{
#ifdef CONFIG_DEBUG
    if ( *(u32_t *)context->srr0 == KDEBUG_EXCEPT_INSTR )
    {
	switch( context->r0 ) {
	case L4_TRAP64_KDEBUG:
	    printf( "-- DEBUG: %s --\n", (char *)(context->srr0+8) );
	    kdebug_entry( context );
	    break;
	case L4_TRAP64_KPUTC:
	    putc( (char)context->r3 );
	    break;
	case L4_TRAP64_KGETC:
	    context->r3 = getc(true);
	    break;
	case L4_TRAP64_KGETC_NB:
	    context->r3 = getc(false);
	    break;
	default:
	    goto exception;
	}
	context->srr0 += 4;	/* Skip the trap instruction */
	except_return();
    }
#endif

    if ( *(u32_t *)context->srr0 == KIP_EXCEPT_INSTR )
    {
	//TRACEF( "KernelInterface() at %p (%p)\n", context->srr0, get_current_tcb() );

	context->srr0 += 4;
        space_t * space = get_current_space ();

	context->r3 = (u64_t) space->get_kip_page_area ().get_base ();
	context->r4 = get_kip ()->api_version;
	context->r5 = get_kip ()->api_flags;
	context->r6 = (NULL != get_kip()->kernel_desc_ptr) ?
		    *(word_t *)((word_t)get_kip() + get_kip()->kernel_desc_ptr) : 0;

	except_return();
    }

exception:
    if ( !send_exception_ipc( vect, 0, 0, 0) )
    {
	printf( "\n-- KD# Program check --\n" );
	if( EXPECT_FALSE(get_kip()->kdebug_entry != NULL) )
	    get_kip()->kdebug_entry(context);

	halt_user_thread();
    }

    except_return();
}

/* Instruction storage interrupt (Instruction TLB miss) */
extern "C" void isi_handler( powerpc64_irq_context_t *context )
{
    word_t srr0 = context->srr0;
    word_t srr1 = context->srr1;

    tcb_t *tcb = get_current_tcb();
    space_t *space = tcb->get_space();
    if (!space) space = get_kernel_space();

    bool is_kernel = ppc64_is_kernel_mode(srr1);

    TRACEPOINT( except_isi_cnt, 
		printf( "[%p%s] Instruction fault @ %p\n", tcb,
		is_kernel ? " (kernel)" : "", srr0 ) );

    // If fault is in the kernel area, just map in a kernel page
    if ( EXPECT_FALSE( is_kernel && space->is_kernel_area ((addr_t)srr0) ))
    {
	TRACEF( "kernel execute @ %p\n", srr0 );
	pgent_t pg;
        space = get_kernel_space();
#ifdef CONFIG_POWERPC64_LARGE_PAGES
	pgent_t::pgsize_e size = pgent_t::size_16m;
#else
	pgent_t::pgsize_e size = pgent_t::size_4k;
#endif

	/* Create a dummy page table entry */
	pg.set_entry( space, size, virt_to_phys((addr_t)srr0),
		      7, pgent_t::l4default, true );

	/* Insert the kernel mapping, bolted */
	get_pghash()->insert_mapping( space, (addr_t)srr0, &pg, size, true );

	//TRACEF( "kernel fault done\n" );
	except_return();
    }

    // Use kernel space if we have kernel fault in TCB area or
    // when no space is set (e.g., running on the idle thread)
    if ( EXPECT_FALSE( is_kernel && space->is_tcb_area ((addr_t)srr0) )
			    || space == NULL)
    {
        space = get_kernel_space();
	//TRACEF( "kernel space\n" );
    }

    if ( EXPECT_FALSE( is_kernel ))
    {
	if( srr0 < USER_AREA_END )
	    TRACEF( "kernel execution in user area: %p\n", srr0 );
	else if( (srr0 < KERNEL_AREA_START) || (srr0 >= KERNEL_AREA_END) )
	    TRACEF( "kernel execution in data area: %p\n", srr0 );
    }

    if( EXPECT_TRUE( EXCEPT_IS_ISI_MISS(srr1) ) ) 
    {
	if( EXPECT_TRUE(space->handle_hash_miss((addr_t)srr0)) ) 
	{
	    //TRACEF("found - returning\n");
	    except_return();
	}
    }
    else if( EXCEPT_IS_FAULT(srr1) )
    {
	// Page found, but access denied
	if( EXPECT_TRUE(space->handle_protection_fault( (addr_t)srr0, false )) )
	{
	    //TRACEF("handled - returning\n");
	    except_return();
	}
    } else {
	printf( "-- KD# Unknown instruction fault --\n");
	if( EXPECT_FALSE(get_kip()->kdebug_entry != NULL) )
	    get_kip()->kdebug_entry(context);
    }

    //TRACEF("handle pagefault\n");
    space->handle_pagefault( (addr_t)srr0, (addr_t)srr0, 
	    space_t::execute, ppc64_is_kernel_mode(srr1) );

    // Try to reload the hash table after pagefault
    space->handle_hash_miss((addr_t)srr0);

    //TRACEF("done pagefault\n");
    except_return();
}

/* FIXME - check if the kernel debugger is waiting for this */
extern "C" void ppc64_except_trace( word_t vect, powerpc64_irq_context_t *context )
{
    printf( "--KD# Trace Point: IP=%p, SRR1=%p --\n", context->srr0, context->srr1 );
    if( EXPECT_FALSE(get_kip()->kdebug_entry != NULL) )
	get_kip()->kdebug_entry(context);
    except_return();
}

extern "C" void fpu_unavailable_handler( word_t vect, powerpc64_irq_context_t *context )
{
    tcb_t *current = get_current_tcb();

    ASSERT(!ppc64_is_kernel_mode(context->srr1));

    current->resources.powerpc64_fpu_unavail_exception( current );

    except_return();
}

#define SYSCALL_SAVED_REGISTERS (EXCEPT_IPC_SYS_MR_NUM+1)

static bool send_syscall_ipc( powerpc64_irq_context_t *context )
{
    tcb_t *current = get_current_tcb();
    if( current->get_exception_handler().is_nilthread() )
    {
	printf( "Unable to deliver user exception: no exception handler.\n" );
	return false;
    }

    // Save message registers on the stack
    word_t saved_mr[SYSCALL_SAVED_REGISTERS];
    msg_tag_t tag;

    // Save message registers.
    for( int i = 0; i < SYSCALL_SAVED_REGISTERS; i++ )
	saved_mr[i] = current->get_mr(i);
    current->set_saved_partner( current->get_partner() );
    current->set_saved_state( current->get_state() );

    // Create the message tag.
    tag.set( 0, EXCEPT_IPC_SYS_MR_NUM, EXCEPT_IPC_SYS_LABEL);
    current->set_tag( tag );

    // Create the message.
    current->set_mr( EXCEPT_IPC_SYS_MR_R3, context->r3 );
    current->set_mr( EXCEPT_IPC_SYS_MR_R4, context->r4 );
    current->set_mr( EXCEPT_IPC_SYS_MR_R5, context->r5 );
    current->set_mr( EXCEPT_IPC_SYS_MR_R6, context->r6 );
    current->set_mr( EXCEPT_IPC_SYS_MR_R7, context->r7 );
    current->set_mr( EXCEPT_IPC_SYS_MR_R8, context->r8 );
    current->set_mr( EXCEPT_IPC_SYS_MR_R9, context->r9 );
    current->set_mr( EXCEPT_IPC_SYS_MR_R10, context->r10 );
    current->set_mr( EXCEPT_IPC_SYS_MR_R0, context->r0 );

    current->set_mr( EXCEPT_IPC_SYS_MR_IP, (word_t)current->get_user_ip() );
    current->set_mr( EXCEPT_IPC_SYS_MR_SP, (word_t)current->get_user_sp() );
    current->set_mr( EXCEPT_IPC_SYS_MR_FLAGS, (word_t)current->get_user_flags() );

    // Deliver the exception IPC.
    tag = current->do_ipc( current->get_exception_handler(),
	    current->get_exception_handler(), timeout_t::never() );

    // Alter the user context if necessary.
    if( !tag.is_error() )
    {
	current->set_user_ip( (addr_t)current->get_mr( EXCEPT_IPC_SYS_MR_IP ) );
	current->set_user_sp( (addr_t)current->get_mr( EXCEPT_IPC_SYS_MR_SP ) );
	current->set_user_flags( current->get_mr(EXCEPT_IPC_SYS_MR_FLAGS) );
    }
    else
	printf( "Unable to deliver user exception: IPC error.\n" );

    // Results
    context->r3 = current->get_mr( EXCEPT_IPC_SYS_MR_R3 );
    context->r4 = current->get_mr( EXCEPT_IPC_SYS_MR_R4 );
    context->r5 = current->get_mr( EXCEPT_IPC_SYS_MR_R5 );
    context->r6 = current->get_mr( EXCEPT_IPC_SYS_MR_R6 );
    context->r7 = current->get_mr( EXCEPT_IPC_SYS_MR_R7 );
    context->r8 = current->get_mr( EXCEPT_IPC_SYS_MR_R8 );
    context->r9 = current->get_mr( EXCEPT_IPC_SYS_MR_R0 );
    context->r10 = current->get_mr( EXCEPT_IPC_SYS_MR_R10 );
    context->r0 = current->get_mr( EXCEPT_IPC_SYS_MR_R0 );

    // Clean-up.
    for( int i = 0; i < SYSCALL_SAVED_REGISTERS; i++ )
	current->set_mr( i, saved_mr[i] );

    current->set_partner( current->get_saved_partner() );
    current->set_saved_partner( NILTHREAD );
    current->set_state( current->get_saved_state() );
    current->set_saved_state( thread_state_t::aborted );

    return !tag.is_error();
}

extern "C" void ppc64_except_syscall( word_t vect, powerpc64_irq_context_t *context )
{
    if ( !send_syscall_ipc( context ) )
    {
	printf("-- KD# Unhandled user system call --\n");
	if( EXPECT_FALSE(get_kip()->kdebug_entry != NULL) )
	    get_kip()->kdebug_entry(context);

	halt_user_thread();
    }
    except_return();
}

