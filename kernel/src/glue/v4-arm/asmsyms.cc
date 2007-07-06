/*********************************************************************
 *                
 * Copyright (C) 2002-2003, 2006,  University of New South Wales
 *                
 * File path:     glue/v4-arm/asmsyms.cc
 * Description:   Various asm definitions for arm
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
 * $Id: asmsyms.cc,v 1.2 2006/10/19 14:21:57 reichelt Exp $
 *                
 ********************************************************************/
#include <mkasmsym.h>

#include INC_API(tcb.h)
#include INC_GLUE(space.h)

MKASMSYM( TSTATE_RUNNING, (word_t) thread_state_t::running );
MKASMSYM( TSTATE_WAITING_FOREVER, (word_t) thread_state_t::waiting_forever );
MKASMSYM( TSTATE_POLLING, (word_t) thread_state_t::polling);

MKASMSYM( OFS_SPACE_DOMAIN, offsetof(space_t, pt.pd_split.domain));
MKASMSYM( OFS_SPACE_PID, offsetof(space_t, pt.pd_split.pid));
