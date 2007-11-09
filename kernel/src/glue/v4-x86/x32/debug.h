/*********************************************************************
 *                
 * Copyright (C) 2007,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/x32/debug.h
 * Description:   
 *                
 * @LICENSE@
 *                
 * $Id:$
 *                
 ********************************************************************/
#ifndef __GLUE__V4_X86__X32__DEBUG_H__
#define __GLUE__V4_X86__X32__DEBUG_H__

#if defined(CONFIG_DEBUG)

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

#else

#define do_enter_kdebug(x...)	do { } while (true)

#endif /* defined(CONFIG_DEBUG) */

#endif /* !__GLUE__V4_X86__X32__DEBUG_H__ */
