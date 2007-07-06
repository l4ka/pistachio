/*********************************************************************
 *                
 * Copyright (C) 2003-2004, University of New South Wales
 *                
 * File path:    glue/v4-sparc64/ultrasparc/interrupt.S
 * Description:
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
 * $Id: interrupt.S,v 1.1 2004/02/12 06:14:54 philipd Exp $
 *                
 ********************************************************************/

#include INC_ARCH(asm.h)
#include INC_ARCH(ttable.h)

TRAP_C_GLUE(timer_interrupt)

UNIMPLEMENTED_TTABLE_ENTRY(interrupt_level_1,O)
UNIMPLEMENTED_TTABLE_ENTRY(interrupt_level_2,O)
UNIMPLEMENTED_TTABLE_ENTRY(interrupt_level_3,O)
UNIMPLEMENTED_TTABLE_ENTRY(interrupt_level_4,O)
UNIMPLEMENTED_TTABLE_ENTRY(interrupt_level_5,O)
UNIMPLEMENTED_TTABLE_ENTRY(interrupt_level_6,O)
UNIMPLEMENTED_TTABLE_ENTRY(interrupt_level_7,O)
UNIMPLEMENTED_TTABLE_ENTRY(interrupt_level_8,O)
UNIMPLEMENTED_TTABLE_ENTRY(interrupt_level_9,O)
UNIMPLEMENTED_TTABLE_ENTRY(interrupt_level_10,O)
UNIMPLEMENTED_TTABLE_ENTRY(interrupt_level_11,O)
UNIMPLEMENTED_TTABLE_ENTRY(interrupt_level_12,O)
UNIMPLEMENTED_TTABLE_ENTRY(interrupt_level_13,O)

BEGIN_TTABLE_ENTRY(interrupt_level_14,O)
	/* XXX check that it's really a timer interrupt and not a level 14
	 * SOFTINT */
	OTRAP_C_CALL(timer_interrupt)
	nop

UNIMPLEMENTED_TTABLE_ENTRY(interrupt_level_15,O)

UNIMPLEMENTED_TTABLE_ENTRY(interrupt_vector, O)
