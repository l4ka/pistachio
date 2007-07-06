/*********************************************************************
 *                
 * Copyright (C) 2002,  University of New South Wales
 *                
 * File path:     elf-loader/src/platform/srm/main.cc
 * Description:   Main file for elf loader 
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
 * $Id: main.cc,v 1.2 2003/09/24 19:06:19 skoglund Exp $
 *                
 ********************************************************************/

#include "alpha/console.h"
#include "alpha/hwrpb.h"

#include "elf-loader.h"

extern "C" int printf(const char *s, ...);

//#if CONFIG_ALPHA_ADDRESS_BITS == 43
#define PHYS_OFFSET 0xfffffc0000000000
//#else
//#define PHYS_OFFSET 0xffff800000000000
//#endif /* CONFIG_ALPHA_ADDRESS_BITS */


extern void init_console(void);
extern void init_pal(void);

extern "C" void halt(void);

extern "C" void imb(void);

#define pcb_va ((struct pcb_struct *) 0x20000000)
void start_kernel(L4_Word_t bootaddr)
{
    void (*func)(unsigned long) = (void (*)(unsigned long)) bootaddr;
    
    imb();

    printf("elf-loader:\tJumping to kernel\n");
    func(pcb_va->ptbr);
}

int main(void)
{
    L4_Word_t entry;

    init_console();
       
    printf("elf-loader:\tStarting.\n");
    
    init_pal();
    
    if(load_modules(&entry, PHYS_OFFSET)) {
	printf("elf-loader:\tSomething went wrong, halting\n");
	halt();
    }
    
    printf("elf-loader:\tentry is 0x%lx\n", entry);
    
    start_kernel(entry);
    
    halt();
}
