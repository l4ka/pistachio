/*********************************************************************
 *                
 * Copyright (C) 2007,  Karlsruhe University
 *                
 * File path:     glue/v4-amd64/debug.h
 * Description:   
 *                
 * @LICENSE@
 *                
 * $Id:$
 *                
 ********************************************************************/

#ifndef __GLUE__AMD64__DEBUG_H__
#define __GLUE__AMD64__DEBUG_H__

#include INC_GLUEX(x86,debug.h)

INLINE void x86_dump_frame (x86_exceptionframe_t * frame)
{
   printf("fault addr: %8x\tstack: %8x\terror code: %x frame: %p\n",
	   frame->rip, frame->rsp, frame->error, frame);

    printf("eax: %16x\tebx: %16x\n", frame->rax, frame->rbx);
    printf("ecx: %16x\tedx: %16x\n", frame->rcx, frame->rdx);
    printf("esi: %16x\tedi: %16x\n", frame->rsi, frame->rdi);
    printf("ebp: %16x\tefl: %16x [", frame->rbp, frame->rflags);
    dump_flags(frame->rflags);printf("]\n");
    printf("cs:      %4x\tss:      %4x\n",
	   frame->cs & 0xffff, frame->ss & 0xffff);
}


#define do_enter_kdebug(frame, exc)					\
    debug_param_t param = {exc, get_current_space() ?			\
			   get_current_space() : get_kernel_space(),	\
			   get_current_tcb(), frame};			\
    word_t dummy;							\
    asm volatile							\
    ("pushq %%rbp	\n"						\
     "mov %%rsp, %%rbp	\n"						\
     "mov %0, %%rsp	\n"						\
     "pushq %%rbp	\n"						\
     "pushq %1		\n"						\
     "callq *%2		\n"						\
     "addq $8, %%rsp	\n"						\
     "popq %%rsp	\n"						\
     "popq %%rsp	\n"						\
     : "=r"(dummy), "=r"(dummy), "=r"(dummy)				\
     : "0"(&kdb_stack[KDB_STACK_SIZE]),					\
       "1"(&param),							\
       "2"(get_kip()->kdebug_entry)					\
     : "rax", "rcx", "rdx", "memory");			


#endif /* !__GLUE__AMD64__DEBUG_H__ */
