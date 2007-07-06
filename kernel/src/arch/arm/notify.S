/*********************************************************************
 *
 * Copyright (C) 2003-2004,  National ICT Australia (NICTA)
 *
 * File path:      arch/arm/notify.S
 * Description:    Thread notify function helper
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
 * $Id: notify.S,v 1.3 2004/06/04 03:30:02 cvansch Exp $
 *
 ********************************************************************/

#include INC_ARCH(thread.h)
#include INC_GLUE(syscalls.h)
#include INC_ARCH(asm.h)

.global arm_return_from_notify0
.global arm_return_from_notify1
.global arm_return_from_notify2
.align 4

BEGIN_PROC(arm_return_from_notify2)
    mov     r1,     r11
BEGIN_PROC(arm_return_from_notify1)
    mov     r0,     r5
BEGIN_PROC(arm_return_from_notify0)
    mov     lr,     pc
    mov     pc,     r4
   
    /* Restore switch stack */ 
    ldmfd   sp!,    {r4, r5, r11, pc} 

END_PROC(arm_return_from_notify0);
END_PROC(arm_return_from_notify1);
END_PROC(arm_return_from_notify2);

/* Initial jump to a user thread */
BEGIN_PROC(arm_initial_to_user)
        SET_USER_DACR
        RESTORE_ALL 
        movs    pc,     lr
END_PROC(arm_initial_to_user)
