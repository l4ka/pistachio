/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     glue/v4-ia64/thread.cc
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
 * $Id: thread.cc,v 1.15 2003/11/03 16:30:24 skoglund Exp $
 *                
 ********************************************************************/
#include INC_API(tcb.h)

#include INC_GLUE(context.h)

extern "C" void activate_context (word_t arg);

void tcb_t::create_startup_stack (void (*func)())
{
    init_stack ();

    ia64_exception_context_t * context =
	(ia64_exception_context_t *) get_stack_top () - 1;
    stack = (word_t *) context;

    // Clear whole context
    for (word_t * t = (word_t *) context; t < get_stack_top (); t++)
	*t = 0;

    context->rsc = 3 << 2;	    // User level RSE, mandatory mode
    context->unat = ~(1UL << ((word_t) &context->r12 & 0x3f));
    context->rnat = ~0UL;

    // Disable all fp traps/exceptions by default and set up status
    // fields according to the runtime conventions.
    word_t def_fpsf = ar_fpsr_t::pc_3 | ar_fpsr_t::rc_nearest;
    context->fpsr.disable_all_traps ();
    context->fpsr.set_sf0 (def_fpsf);
    context->fpsr.set_sf1 (def_fpsf | ar_fpsr_t::td | ar_fpsr_t::wre);
    context->fpsr.set_sf2 (def_fpsf | ar_fpsr_t::td);
    context->fpsr.set_sf3 (def_fpsf | ar_fpsr_t::td);

    // Enable translation
    context->ipsr.dt	= 1;
    context->ipsr.it	= 1;
    context->ipsr.rt	= 1;

    // Disable high floating-point registers
    context->ipsr.dfh	= 1;

    // Enable interrupts
    context->ipsr.ic	= 1;
    context->ipsr.i	= 1;
    context->tpr	= cr_tpr_t::all_enabled ();

    context->ipsr.cpl	= 3;
    context->ipsr.bn	= 1;   

    // Treat exception frame as a switch frame and create an initial
    // reg stack pointer since notify() relies on it.
    ia64_switch_context_t * fake_ctx = (ia64_switch_context_t *) context;
    word_t * tmp = fake_ctx->bspstore;
    fake_ctx->bspstore = get_reg_stack_bottom ();

    // Schedule notify procedure for activating exception context
    notify (activate_context, (word_t) context);

    // Restore exception frame
    fake_ctx->bspstore = tmp;

    // Function to invoke before restoring user frame
    notify (func);
}
