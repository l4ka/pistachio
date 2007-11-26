/*********************************************************************
 *                
 * Copyright (C) 2007,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/traphandler.h
 * Description:   
 *                
 * @LICENSE@
 *                
 * $Id:$
 *                
 ********************************************************************/
#ifndef __GLUE__V4_X86__TRAPHANDLER_H__
#define __GLUE__V4_X86__TRAPHANDLER_H__

/* debugging exceptions */
extern "C" void exc_debug(void);
extern "C" void exc_nmi(void);
extern "C" void exc_breakpoint(void);

/* gp, pagefault, kip */
extern "C" void exc_gp(void);
extern "C" void exc_pagefault(void);
extern "C" void exc_invalid_opcode(void);

/* fpu  */
extern "C" void exc_nomath_coproc(void);

/* catcher for all other interrupts and exceptions */
typedef void (*func_exc)(void);
extern u64_t exc_catch_all[IDT_SIZE] UNIT("x86.exc_all");
extern "C" void exc_catch_common_wrapper(void);
extern "C" void exc_catch_common(void);

/* exception handling */
class x86_exc_reg_t
{
    static const word_t mr2reg[NUM_EXC_REGS][2];
public:
    static const word_t mr(word_t num) { return mr2reg[num][0]; };
    static const word_t reg(word_t num) { return mr2reg[num][1]; };
};

bool send_exception_ipc(x86_exceptionframe_t * frame, word_t exception);



#endif /* !__GLUE__V4_X86__TRAPHANDLER_H__ */
