/*********************************************************************
 *                
 * Copyright (C) 2002-2004,  University of New South Wales
 *                
 * File path:     glue/v4-sparc64/asmsyms.cc
 * Description:   Various asm definitions for sparc64
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
 * $Id: asmsyms.cc,v 1.4 2004/06/28 06:51:35 philipd Exp $
 *                
 ********************************************************************/
#include <mkasmsym.h>

#include INC_API(tcb.h)
#include INC_ARCH(frame.h)
#include INC_GLUE(config.h)

MKASMSYM(TRAP_FRAME_SIZE, sizeof(trap_frame_t));
MKASMSYM(TRAP_FRAME_G1, offsetof(trap_frame_t, g1));
MKASMSYM(TRAP_FRAME_G2, offsetof(trap_frame_t, g2));
MKASMSYM(TRAP_FRAME_G3, offsetof(trap_frame_t, g3));
MKASMSYM(TRAP_FRAME_G4, offsetof(trap_frame_t, g4));
MKASMSYM(TRAP_FRAME_G5, offsetof(trap_frame_t, g5));
MKASMSYM(TRAP_FRAME_O0, offsetof(trap_frame_t, o0));
MKASMSYM(TRAP_FRAME_O1, offsetof(trap_frame_t, o1));
MKASMSYM(TRAP_FRAME_O2, offsetof(trap_frame_t, o2));
MKASMSYM(TRAP_FRAME_O3, offsetof(trap_frame_t, o3));
MKASMSYM(TRAP_FRAME_O4, offsetof(trap_frame_t, o4));
MKASMSYM(TRAP_FRAME_O5, offsetof(trap_frame_t, o5));
MKASMSYM(TRAP_FRAME_O6, offsetof(trap_frame_t, o6));
MKASMSYM(TRAP_FRAME_O7, offsetof(trap_frame_t, o7));
MKASMSYM(TRAP_FRAME_I6, offsetof(trap_frame_t, i6));
MKASMSYM(TRAP_FRAME_I7, offsetof(trap_frame_t, i7));
MKASMSYM(TRAP_FRAME_ARG6, offsetof(trap_frame_t, args[6]));
MKASMSYM(TRAP_FRAME_ARG7, offsetof(trap_frame_t, args[7]));

MKASMSYM(WINDOW_FRAME_SIZE, sizeof(window_frame_t));
MKASMSYM(WINDOW_FRAME_L0, offsetof(window_frame_t, l0));
MKASMSYM(WINDOW_FRAME_L1, offsetof(window_frame_t, l1));
MKASMSYM(WINDOW_FRAME_L2, offsetof(window_frame_t, l2));
MKASMSYM(WINDOW_FRAME_L3, offsetof(window_frame_t, l3));
MKASMSYM(WINDOW_FRAME_L4, offsetof(window_frame_t, l4));
MKASMSYM(WINDOW_FRAME_L5, offsetof(window_frame_t, l5));
MKASMSYM(WINDOW_FRAME_L6, offsetof(window_frame_t, l6));
MKASMSYM(WINDOW_FRAME_L7, offsetof(window_frame_t, l7));
MKASMSYM(WINDOW_FRAME_I0, offsetof(window_frame_t, i0));
MKASMSYM(WINDOW_FRAME_I1, offsetof(window_frame_t, i1));
MKASMSYM(WINDOW_FRAME_I2, offsetof(window_frame_t, i2));
MKASMSYM(WINDOW_FRAME_I3, offsetof(window_frame_t, i3));
MKASMSYM(WINDOW_FRAME_I4, offsetof(window_frame_t, i4));
MKASMSYM(WINDOW_FRAME_I5, offsetof(window_frame_t, i5));
MKASMSYM(WINDOW_FRAME_I6, offsetof(window_frame_t, i6));
MKASMSYM(WINDOW_FRAME_I7, offsetof(window_frame_t, i7));

MKASMSYM(SWITCH_FRAME_SIZE, sizeof(switch_frame_t));
MKASMSYM(SWITCH_FRAME_O7, offsetof(switch_frame_t, o7));
MKASMSYM(SWITCH_FRAME_I6, offsetof(switch_frame_t, i6));
MKASMSYM(SWITCH_FRAME_I7, offsetof(switch_frame_t, i7));
MKASMSYM(SWITCH_FRAME_O0, offsetof(switch_frame_t, o0));
MKASMSYM(SWITCH_FRAME_O1, offsetof(switch_frame_t, o1));
MKASMSYM(SWITCH_FRAME_O2, offsetof(switch_frame_t, o2));
MKASMSYM(SWITCH_FRAME_Y, offsetof(switch_frame_t, y));

MKASMSYM(TCB_PINNED_STACK_TOP, offsetof(tcb_t, arch.pinned_stack_top));
MKASMSYM(TCB_TSTATE, offsetof(tcb_t, arch.tstate));
MKASMSYM(TCB_TPC, offsetof(tcb_t, arch.tpc));
MKASMSYM(TCB_TNPC, offsetof(tcb_t, arch.tnpc));
MKASMSYM(TCB_TL, offsetof(tcb_t, arch.tl));
MKASMSYM(TCB_PIL, offsetof(tcb_t, arch.pil));
MKASMSYM(TCB_SAVED_WINDOWS, offsetof(tcb_t, arch.saved_windows));
MKASMSYM(TCB_SAVED_CWP, offsetof(tcb_t, arch.saved_cwp));
MKASMSYM(TCB_MYSELF_LOCAL, offsetof(tcb_t, myself_local));

MKASMSYM(THREAD_STATE_RUNNING, thread_state_t::running);
MKASMSYM(THREAD_STATE_WAITING_FOREVER, thread_state_t::waiting_forever);

MKASMSYM(SPACE_PGDIR_CONTEXT, offsetof(space_t, pgdir_context));

MKASMSYM(UTCB_MR_BASE, offsetof(utcb_t, mr[0]));
MKASMSYM(UTCB_WINDOW_SAVE_AREA, offsetof(utcb_t, reg_win[0]));
