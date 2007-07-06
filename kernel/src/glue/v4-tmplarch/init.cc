/*********************************************************************
 *                
 * Copyright (C) 2002,  Karlsruhe University
 *                
 * File path:     glue/v4-tmplarch/init.cc
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
 * $Id: init.cc,v 1.2 2003/09/24 19:05:43 skoglund Exp $
 *                
 ********************************************************************/

#include INC_API(kernelinterface.h)
#include INC_API(schedule.h)

/**
 * Entry point from ASM into C kernel
 * Precondition: paging is initialized with init_paging
 */
extern "C" void SECTION(".init") startup_system()
{
    /* feed the kernel memory allocator */
#warning PORTME
    //init_bootmem();
    
    /* initialize kernel interface page */
    get_kip()->init();
    
    /* initialize mapping database */
    //init_mdb ();
    
    /* initialize kernel debugger if any */
    if (get_kip()->kdebug_init)
	get_kip()->kdebug_init();

    /* configure IRQ hardware - global part */
    //get_interrupt_ctrl()->init_arch();
    /* configure IRQ hardware - local part */
    //get_interrupt_ctrl()->init_cpu();
    
    /* initialize the kernel's timer source */
    //get_timer()->init_global();
    //get_timer()->init_cpu();


    /* initialize the scheduler */
    get_current_scheduler()->init();
    /* get the thing going - we should never return */
    get_current_scheduler()->start();
    
    /* make sure we don't fall off the edge */
    spin_forever(1);
}
