/*********************************************************************
 *                
 * Copyright (C) 2007,  Karlsruhe University
 *                
 * File path:     glue/v4-ia32/debug.h
 * Description:   
 *                
 * @LICENSE@
 *                
 * $Id:$
 *                
 ********************************************************************/
#ifndef __GLUE__IA32__DEBUG_H__
#define __GLUE__IA32__DEBUG_H__

#include INC_GLUEX(x86,debug.h)

INLINE void x86_dump_frame (x86_exceptionframe_t * frame)
{
   printf("fault addr: %8x\tstack: %8x\terror code: %x frame: %p\n",
	   frame->eip, frame->esp, frame->error, frame);

    printf("eax: %8x\tebx: %8x\n", frame->eax, frame->ebx);
    printf("ecx: %8x\tedx: %8x\n", frame->ecx, frame->edx);
    printf("esi: %8x\tedi: %8x\n", frame->esi, frame->edi);
    printf("ebp: %8x\tefl: %8x [", frame->ebp, frame->eflags);
    dump_flags(frame->eflags);printf("]\n");
    printf("cs:      %4x\tss:      %4x\n",
	   frame->cs & 0xffff, frame->ss & 0xffff);
    printf("ds:      %4x\tes:      %4x\n",
	   frame->ds & 0xffff, frame->es & 0xffff);
}


#define do_enter_kdebug(frame, exc)					\
    debug_param_t param = {exc, get_current_space() ?			\
			   get_current_space() : get_kernel_space(),	\
			   get_current_tcb(), frame};			\
    word_t dummy;							\
    asm volatile							\
    ("pushl %%ebp	\n"						\
     "mov %%esp, %%ebp	\n"						\
     "mov %0, %%esp	\n"						\
     "pushl %%ebp	\n"						\
     "pushl %1		\n"						\
     "call *%2		\n"						\
     "addl $4, %%esp	\n"						\
     "popl %%esp	\n"						\
     "popl %%ebp	\n"						\
     : "=r"(dummy), "=r"(dummy), "=r"(dummy)				\
     : "0"(&kdb_stack[KDB_STACK_SIZE]),					\
       "1"(&param),							\
       "2"(get_kip()->kdebug_entry)					\
     : "eax", "ecx", "edx", "memory");			

#endif /* !__GLUE__IA32__DEBUG_H__ */
