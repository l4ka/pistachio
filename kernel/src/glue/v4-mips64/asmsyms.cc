/*********************************************************************
 *                
 * Copyright (C) 2002-2003,  University of New South Wales
 *                
 * File path:     glue/v4-mips64/asmsyms.cc
 * Description:   Various asm definitions for mips64
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
 * $Id: asmsyms.cc,v 1.4 2003/09/24 19:05:36 skoglund Exp $
 *                
 ********************************************************************/
#include <mkasmsym.h>

#include INC_API(tcb.h)
#include INC_GLUE(space.h)
#include INC_GLUE(config.h)
#include INC_GLUE(context.h)

/* Mips64 defines */
MKASMSYM (MIPS64_SWITCH_STACK_SIZE, sizeof(mips64_switch_stack_t));
MKASMSYM (UTCB_MR_OFFSET, (word_t) ((utcb_t *) 0)->mr);
MKASMSYM (ASM_KTCB_MASK, KTCB_MASK);
MKASMSYM (ASM_KTCB_BITS, KTCB_BITS);
MKASMSYM (ASM_KTCB_SIZE, KTCB_SIZE);
MKASMSYM (ASM_KTCB_AREA_START, KTCB_AREA_START);
//MKASMSYM (SWITCH_STACK_RA, offsetof(alpha_switch_stack_t, ra));
//
MKASMSYM( TSTATE_RUNNING, (word_t) thread_state_t::running );
MKASMSYM( TSTATE_WAITING_FOREVER, (word_t) thread_state_t::waiting_forever );
MKASMSYM( TSTATE_POLLING, (word_t) thread_state_t::polling);

MKASMSYM (SPACE_ASID_OFFSET, (word_t) &((space_t *) 0)->asid);
