/*********************************************************************
 *                
 * Copyright (C) 2002,  University of New South Wales
 *                
 * File path:     glue/v4-alpha/asmsyms.cc
 * Description:   Various asm definitions for alpha
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
 * $Id: asmsyms.cc,v 1.2 2003/09/24 19:05:32 skoglund Exp $
 *                
 ********************************************************************/
#include <mkasmsym.h>

#include INC_ARCH(thread.h)

#include INC_API(tcb.h)
#include INC_GLUE(config.h)

/* Alpha defines */
MKASMSYM (ALPHA_CONTEXT_SIZE, sizeof(alpha_context_t));
MKASMSYM (ALPHA_SWITCH_STACK_SIZE, sizeof(alpha_switch_stack_t));
MKASMSYM (UTCB_MR_OFFSET, (word_t) ((utcb_t *) 0)->mr);
MKASMSYM (ASM_KTCB_MASK, KTCB_MASK);
MKASMSYM (ASM_KTCB_BITS, KTCB_BITS);
MKASMSYM (ASM_KTCB_SIZE, KTCB_SIZE);
MKASMSYM (ASM_KTCB_AREA_START, KTCB_AREA_START);
MKASMSYM (SWITCH_STACK_RA, offsetof(alpha_switch_stack_t, ra));
