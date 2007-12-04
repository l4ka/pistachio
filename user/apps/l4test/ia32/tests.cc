/*********************************************************************
 *                
 * Copyright (C) 2007,  Karlsruhe University
 *                
 * File path:     l4test/ia32/tests.cc
 * Description:   
 *                
 * @LICENSE@
 *                
 * $Id:$
 *                
 ********************************************************************/
#include <l4/kip.h>
#include <l4/ipc.h>
#include <l4/schedule.h>
#include <l4/kdebug.h>
#include <l4/ia32/arch.h>

#include <l4io.h>
#include "../l4test.h"
#include "../assert.h"

#define START_ADDR(func)	((L4_Word_t) func)
#define NOUTCB	((void*)-1)

L4_Word_t exc_stack[2048] __attribute__ ((aligned (16)));
L4_Word_t zero = 0;

void exc (void)
{
    L4_Set_ExceptionHandler(L4_Pager());
    printf("Testing exception handling\n");
    printf("%x", (int) (1/zero));
    
}

void exc2 (void)
{
    printf("IPC exception handling ok.\n");
    L4_Send(L4_Pager());
    printf("Testing ExRegs ctrlxfer read\n");
    printf("%x", (int) (1/zero));
}



void arch_test(void)

{
    L4_KernelInterfacePage_t * kip =
	(L4_KernelInterfacePage_t *) L4_KernelInterface ();
    
    L4_Word_t utcb_size = L4_UtcbSize (kip); 
    L4_MsgTag_t tag;
    L4_Msg_t exc_msg;
    L4_Word_t dummy, old_control;
    L4_ThreadId_t tid;
    L4_ThreadId_t exc_tid = L4_GlobalId (L4_ThreadNo (L4_Myself()) + 1, 2);
    L4_Word_t exc_utcb = L4_MyLocalId().raw;
    L4_Word_t eip, esp;
    
    exc_utcb = (exc_utcb & ~(utcb_size - 1)) + utcb_size;

    // Touch the memory to make sure we never get pagefaults
    extern L4_Word_t _end, _start;
    for (L4_Word_t * x = (&_start); x < &_end; x++)
    {
	volatile L4_Word_t q;
	q = *(volatile L4_Word_t*) x;
    }

    exc_tid = create_thread();

    
    L4_Start (exc_tid, (L4_Word_t) exc_stack + sizeof(exc_stack) - 32,
	      START_ADDR (exc));

    /* Test Exception IPC */

    
    tag = L4_Receive (exc_tid);
    L4_Store (tag, &exc_msg);
    printf ("ExcHandler got msg from %p (%p, %p, %p, %p, %p)\n",
	    (void *) tid.raw, (void *) tag.raw,
	    (void *) L4_Get (&exc_msg, 0), (void *) L4_Get (&exc_msg, 1),
	    (void *) L4_Get (&exc_msg, 2), (void *) L4_Get (&exc_msg, 3));;
    

    kill_thread( exc_tid );

}
