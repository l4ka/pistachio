/*********************************************************************
 *                
 * Copyright (C) 2003,  Karlsruhe University
 *                
 * File path:     l4test/powerpc/tests.cc
 * Description:   Architecture dependent tests
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
 * $Id: tests.cc,v 1.6 2003/12/19 18:34:03 joshua Exp $
 *                
 ********************************************************************/

#include <l4/ipc.h>
#include <l4/thread.h>
#include <l4/kdebug.h>
#include <l4io.h>

#include "../l4test.h"
#include "../menu.h"

#define GENERIC_EXC_MR_IP	0
#define GENERIC_EXC_MR_SP	1
#define GENERIC_EXC_MR_FLAGS	2
#define GENERIC_EXC_MR_NO	3
#define GENERIC_EXC_MR_CODE	4
#define GENERIC_EXC_MR_LOCAL_ID	5
#define GENERIC_EXC_MR_MAX	6

#define GENERIC_EXC_LABEL	(L4_Word_t(((-5 << 4) << 16)) >> 16)

#define SC_EXC_MR_IP		9
#define SC_EXC_MR_SP		10
#define SC_EXC_MR_FLAGS		11
#define SC_EXC_MR_MAX		12

#define SC_EXC_LABEL		(L4_Word_t(((-5 << 4) << 16)) >> 16)

#define GENERIC_EXC_SUCCESS	1
#define SYSCALL_EXC_SUCCESS	2

#define TOT_EXCEPTIONS		10000

//#define dprintf(a...)	printf(a)
#define dprintf(a...)

static L4_ThreadId_t controller_tid;
static L4_ThreadId_t handler_tid;
static L4_ThreadId_t subject_tid;
static L4_Word_t which_test = 0;

static bool ipc_error( L4_MsgTag_t tag )
{
    if( !(tag.X.flags & 8) )
	return false;

    L4_Word_t err = L4_ErrorCode();
    if( err & 1 )
	printf( "IPC receive error: %lu, %lx\n", 
		(err >> 1) & 7, err >> 4 );
    else
	printf( "IPC send error: %lu, %lx\n",
		(err >> 1) & 7, err >> 4 );
    return true;
}

static void except_handler_thread( void )
{
    L4_ThreadId_t tid;
    L4_MsgTag_t tag;
    L4_Msg_t msg;

    for(;;)
    {
	tag = L4_Wait( &tid );

	for(;;)
	{
	    if( ipc_error(tag) )
		break;

    	    L4_Store( tag, &msg );

    	    if( (L4_Label(tag) == GENERIC_EXC_LABEL) && 
    		    (L4_UntypedWords(tag) == GENERIC_EXC_MR_MAX) )
    	    {
		// We have received a generic exception message.
		static L4_Word_t tot = 0;
		int response = GENERIC_EXC_SUCCESS;
    		dprintf( "generic exception: ip %lx, sp %lx, flags %lx\n"
			"                   no %lx, code %lx, local ID %lx\n",
    			L4_Get(&msg, GENERIC_EXC_MR_IP),
    			L4_Get(&msg, GENERIC_EXC_MR_SP),
    			L4_Get(&msg, GENERIC_EXC_MR_FLAGS),
    			L4_Get(&msg, GENERIC_EXC_MR_NO),
    			L4_Get(&msg, GENERIC_EXC_MR_CODE),
    			L4_Get(&msg, GENERIC_EXC_MR_LOCAL_ID)
    		      );

		// Increment the instruction pointer and reply
		// to the excepting thread.
    		L4_MsgPutWord( &msg, GENERIC_EXC_MR_IP,
    			L4_Get(&msg, GENERIC_EXC_MR_IP) + 4 );
		L4_Load( &msg );

		tot++;
		if( tot >= TOT_EXCEPTIONS )
		{
		    tot = 0;

		    tag = L4_Reply( tid );
		    if( ipc_error(tag) )
			response = 0;

		    // Tell the controller thread that we finished the test.
		    tag.raw = 0;
		    L4_Set_Label( &tag, response );
		    L4_Clear( &msg );
		    L4_Set_MsgMsgTag( &msg, tag );
		    tid = controller_tid;
		}
    	    }
    	    else if( (L4_Label(tag) == SC_EXC_LABEL) && 
   		    (L4_UntypedWords(tag) == SC_EXC_MR_MAX) )
    	    {
		// We have received a system call emulation exception.
		static L4_Word_t tot = 0;
		int response = SYSCALL_EXC_SUCCESS;
    		dprintf( "syscall exception from %lx: ip %lx, sp %lx, flags %lx\n",
    			L4_GlobalId(tid).raw,
    			L4_Get(&msg, SC_EXC_MR_IP),
    			L4_Get(&msg, SC_EXC_MR_SP),
    			L4_Get(&msg, SC_EXC_MR_FLAGS)
    		      );

		// Reply to the faulting thread.  Don't touch the
		// instruction pointer!!
		L4_Load( &msg );

		tot++;
		if( tot >= TOT_EXCEPTIONS )
		{
		    tot = 0;

		    tag = L4_Reply( tid );
		    if( ipc_error(tag) )
			response = 0;

		    // Tell the controller thread that we finished the test.
		    tag.raw = 0;
		    L4_Set_Label( &tag, response );
		    L4_Clear( &msg );
		    L4_Set_MsgMsgTag( &msg, tag );
		    tid = controller_tid;
		}
    	    }
    	    else
    	    {
    		printf( "unknown exception from %lx! label %lx, untyped %lx\n",
    			tid.raw,
    			L4_Label(tag), L4_UntypedWords(tag) );

		// Tell the controller thread that we received an
		// unexpected result.
		tag.raw = 0;
		L4_Set_Label( &tag, 0 );
		L4_Clear( &msg );
		L4_Set_MsgMsgTag( &msg, tag );
		tid = controller_tid;
    	    }

    	    L4_Load( &msg );
    	    tag = L4_ReplyWait( tid, &tid );
	}
    }
}

static void subject_thread( void )
{
    L4_MsgTag_t tag;
    L4_Msg_t msg;

    dprintf( "controller tid: %lx, handler tid: %lx\n",
	    L4_GlobalId(controller_tid).raw, handler_tid.raw );

    L4_Set_ExceptionHandler( L4_GlobalId(handler_tid) );

    if( which_test == GENERIC_EXC_SUCCESS )
    {
	// Generate a generic exception.
	for( int i=0; i < TOT_EXCEPTIONS; i++ )
	    __asm__ __volatile__ ("mr %%r0, 0 ; trap" ::: "r0");
	dprintf( "trap complete\n" );
    }
    else
    {
	// Generate a legacy system call exception.
	for( int i=0; i < TOT_EXCEPTIONS; i++ )
	    __asm__ __volatile__ (
	    	    "sc"
	    	    : : : "r0", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10",
		          "r11", "r12", "cr0", "cr1", "cr5", "cr6", "cr7",
		          "lr", "ctr", "xer"
		    );
       	dprintf( "syscall complete\n" );
    }

    // Tell the controller thread that we finished the exception test.
    tag.raw = 0;
    L4_Set_Label( &tag, which_test );
    L4_Clear( &msg );
    L4_Set_MsgMsgTag( &msg, tag );
    L4_Load( &msg );
    L4_Send( controller_tid );
}

static void exception_tests( L4_Word_t test_choice, const char *test_msg )
{
    L4_ThreadId_t tid;

    //L4_KDB_Enter( "starting" );

    // Start tests and initialize global values.
    which_test = test_choice;
    controller_tid = L4_Myself();
    tid = create_thread( except_handler_thread, false, -1 );
    handler_tid = L4_GlobalId(tid);
    tid = create_thread( subject_thread, false, -1 );
    subject_tid = L4_GlobalId(tid);

    dprintf( "handler tid %lx, subject tid %lx, controller tid %lx\n",
	    handler_tid.raw, subject_tid.raw, L4_Myself().raw );

    /*  Wait for results.
     */
    L4_ThreadId_t next_tid;
    L4_MsgTag_t tag;

    tag = L4_Wait( &tid );
    tid = L4_GlobalId( tid );
    if( ((tid != subject_tid) && (tid != handler_tid))
	    || ipc_error(tag) || (L4_Label(tag) != which_test) )
    {
	print_result( test_msg, false );
	goto clean;
    }
    else if( tid == handler_tid )
	next_tid = subject_tid;
    else if( tid == subject_tid )
	next_tid = handler_tid;

    tag = L4_Wait( &tid );
    tid = L4_GlobalId( tid );
    if( (tid != next_tid)
	    || ipc_error(tag) || (L4_Label(tag) != which_test) )
    {
	print_result( test_msg, false );
	goto clean;
    }

    print_result( test_msg, true );

clean:
    dprintf( "shutting down ...\n" );
    kill_thread( handler_tid );
    kill_thread( subject_tid );
}

static void exception_unwind_test( L4_Word_t test_choice, const char *test_msg)
{
    L4_ThreadId_t tid;

    // Start tests and initialize global values.
    which_test = test_choice;
    controller_tid = L4_Myself();
    handler_tid = L4_Myself();
    tid = create_thread( subject_thread, false, -1 );
    subject_tid = L4_GlobalId(tid);

    /*  Wait for results.
     */
    L4_MsgTag_t tag;
    L4_Word_t label =
	(which_test == GENERIC_EXC_SUCCESS) ?  GENERIC_EXC_LABEL:SC_EXC_LABEL;

    tag = L4_Wait( &tid );
    tid = L4_GlobalId( tid );
    if( (tid != subject_tid) 
	    || ipc_error(tag) || (L4_Label(tag) != label) )
    {
	print_result( test_msg, false );
	goto clean;
    }

    kill_thread( subject_tid );
    print_result( test_msg, true );
    return;

clean:
    dprintf( "shutting down ...\n" );
    kill_thread( subject_tid );
}


static void unhandled_exception_thread( void )
{
    __asm__ __volatile__ (
	    "mr %%r0, 0 ;\n\t"
	    ".globl __trigger ;\n\t"
	    "__trigger:\n\t"
	    "trap" ::: "r0");

    dprintf( "resumed after an unhandled exception\n" );

    // Tell the controller that we have finished.
    L4_Msg_t msg;
    L4_Clear( &msg );
    L4_Set_Label( &msg.tag, 0 );
    L4_Load( &msg );
    L4_Send( controller_tid );
}

static void unhandled_exception_test( void )
{
    controller_tid = L4_Myself();

    L4_ThreadId_t tid = create_thread( unhandled_exception_thread, false, -1 );
    subject_tid = L4_GlobalId(tid);

    L4_ThreadSwitch( subject_tid );

    L4_Word_t control, sp, ip, flags, handle;
    L4_ThreadId_t pager;
    tid = L4_ExchangeRegisters( subject_tid, control, sp, ip, flags, handle,
	    pager, &control, &sp, &ip, &flags, &handle, &pager );

    if( L4_IsNilThread(tid) )
    {
	print_result( "Unhandled exception test", false );
	goto clean;
    }
    else
    {
	extern char __trigger[];
	L4_Word_t trigger_ip = L4_Word_t(__trigger);
	print_result( "Unhandled exception test", (trigger_ip == ip) );
    }

    // Restart the halted thread, after the faulting instruction.
    ip += 4;
    L4_Start( L4_LocalId(subject_tid), sp, ip );

    // Wait for the subject thread to finish.
    L4_MsgTag_t tag;
    tag = L4_Wait( &tid );
    tid = L4_GlobalId( tid );
    print_result( "Unhandled exception resume", 
	    (tid == subject_tid) && (L4_Label(tag) == 0) );

clean:
    dprintf( "shutting down ...\n" );
    kill_thread( subject_tid );
}

static void generic_exc_test( void )
{
    exception_tests( GENERIC_EXC_SUCCESS, "Generic exception test" );
}

static void generic_unwind_test( void )
{
    exception_unwind_test( GENERIC_EXC_SUCCESS, "Generic exception unwind" );
}

static void syscall_exc_test( void )
{
    exception_tests( SYSCALL_EXC_SUCCESS, "Legacy system call exception test");
}

static void syscall_unwind_test( void )
{
    exception_unwind_test( SYSCALL_EXC_SUCCESS, 
	    "Legacy system call exception unwind");
}

static void all_tests( void )
{
    generic_exc_test();
    syscall_exc_test();
    generic_unwind_test();
    syscall_unwind_test();
    unhandled_exception_test();
}

static struct menuitem menu_items[] =
{
    { NULL,		"return" },
    { generic_exc_test,	"Generic exception test" },
    { syscall_exc_test,	"Legacy system call exception test" },
    { generic_unwind_test,	"Generic exception unwind" },
    { syscall_unwind_test,	"Legacy system call exception unwind" },
    { unhandled_exception_test,	"Unhandled exception test" },
    { all_tests,	"All PowerPC tests" },
};

static struct menu menu = 
{
    "PowerPC Menu",
    0,
    NUM_ITEMS( menu_items ),
    menu_items
};

void arch_test(void)
{
    menu_input( &menu );
}
