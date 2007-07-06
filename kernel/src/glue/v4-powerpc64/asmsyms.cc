/****************************************************************************
 *
 * Copyright (C) 2003-2004,  National ICT Australia (NICTA)
 *
 * File path:	glue/v4-powerpc64/asmsyms.cc
 * Description:	Various C++ constants converted into assembler compatible 
 * 		symbols.
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
 * $Id: asmsyms.cc,v 1.5 2004/06/04 06:38:41 cvansch Exp $
 *
 ***************************************************************************/

#include <mkasmsym.h>

#include INC_API(threadstate.h)
#include INC_API(tcb.h)

#if 0
MKASMSYM( TSTATE_RUNNING, (word_t) thread_state_t::running );
MKASMSYM( TSTATE_WAITING_FOREVER, (word_t) thread_state_t::waiting_forever );

MKASMSYM( OFS_TSWITCH_FRAME_IP, offsetof(tswitch_frame_t, ip) );
MKASMSYM( OFS_TSWITCH_FRAME_R30, offsetof(tswitch_frame_t, r30) );
MKASMSYM( OFS_TSWITCH_FRAME_R31, offsetof(tswitch_frame_t, r31) );
MKASMSYM( TSWITCH_FRAME_SIZE, sizeof(tswitch_frame_t) );

#endif
extern utcb_t *UTCB;
MKASMSYM( OFS_UTCB_MR, (((word_t) &UTCB->mr[0]) - ((word_t) UTCB)));
MKASMSYM( OFS_UTCB_EXCEPTION_HANDLER, (((word_t) &UTCB->exception_handler) - ((word_t) UTCB)));
MKASMSYM( OFS_UTCB_MY_GLOBAL_ID, (((word_t) &UTCB->my_global_id) - ((word_t) UTCB)));

#if (CONFIG_PLAT_OFPOWER4 || CONFIG_PLAT_OFPOWER3)
#include INC_ARCH(rtas.h)

extern rtas_t *RTAS;
MKASMSYM( RTAS_ENTRY, (((word_t) &RTAS->entry) - ((word_t) RTAS)));
MKASMSYM( RTAS_BASE, (((word_t) &RTAS->base) - ((word_t) RTAS)));
#endif
