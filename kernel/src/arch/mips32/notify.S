/*********************************************************************
 *                
 * Copyright (C) 2006,  Karlsruhe University
 *                
 * File path:     arch/mips32/notify.S
 * Description:   MIPS32 notify return stubs
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
 * $Id: notify.S,v 1.1 2006/02/23 21:07:45 ud3 Exp $
 *                
 ********************************************************************/

#include INC_ARCH(asm.h)
#include INC_GLUE(context.h)
/* #include <asmsyms.h> */

#define BEGIN_NOALIGN(name)			\
    .global name;				\
    .align 2;					\
    .ent name;					\
name:



/******** s0 is the func, s1 is the arg1, s2 is the arg2. ********/ 
BEGIN_PROC(mips32_return_from_notify2)

        move    a1, s1
        /* fall through */
END_PROC(mips32_return_from_notify2)


	
/******** So s0 is the func, s1 is the arg. ********/ 
BEGIN_NOALIGN(mips32_return_from_notify1)

        /* fall through */
END_PROC(mips32_return_from_notify1)


	
/******* s0 is the func. ********/ 
BEGIN_NOALIGN(mips32_return_from_notify0)

        move    a0, s0	/* assembler will do branch delay */
        subu	sp, sp, 0x10
        jal     s8
        nop
        addu	sp, sp, 0x10
        /* XXX: should not return here after switch to user thread ??? */

RESTORE_SWITCH_STACK
        jr	ra
        nop

END_PROC(mips32_return_from_notify0)
