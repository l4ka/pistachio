/*********************************************************************
 *                
 * Copyright (C) 2002-2008,  Karlsruhe University
 *                
 * File path:     arch/x86/x64/init32.cc
 * Description:   Switch to 64bit long mode
 *                This file is compiled as 32bit Code
 *
 * 
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
 * $Id: init32.cc,v 1.8 2006/11/30 15:32:07 stoess Exp $
 *                
 ********************************************************************/

#include <init.h>


#include INC_ARCH(cpu.h)
#include INC_ARCH(mmu.h)
#include INC_ARCH(segdesc.h)
#include INC_ARCH(x86.h)
#include INC_GLUE_SA(offsets.h)

/* ugly ... */
#define KERNEL_OFFSET_TYPED	X86_64BIT_TYPED(KERNEL_OFFSET)

/* Init Pagemap - We use 2MByt Page Translation, (only :-) 3 levels */
u64_t init_pdir[512] __attribute__((aligned(4096))) SECTION(".init.data");
u64_t init_pdp[512] __attribute__((aligned(4096))) SECTION(".init.data");
u64_t init_pml4[512] __attribute__((aligned(4096))) SECTION(".init.data");

#define INIT32_PDIR_ENTRIES	256
#define INIT32_PDIR_ATTRIBS	(X86_PAGE_VALID | X86_PAGE_WRITABLE | X86_PAGE_SUPER)
#define INIT32_PTAB_ATTRIBS	(X86_PAGE_VALID | X86_PAGE_WRITABLE)


/* Temporary GDT (INVALID, CS), temporary CS */
x86_segdesc_t init32_gdt[2] SECTION(".init.data");
#define INIT32_CS		0x08

/* Temporary debug messaging system  */

#if defined(CONFIG_CONS_KBD) 
#define INIT32_DEBUG_SCREEN	(0xb8000)
INLINE void SECTION(".init.init32")  init32_spin(int pos = 0)
{

    while(1)
    {
         ((u16_t *) INIT32_DEBUG_SCREEN)[pos] += 1;
    }
}

static void SECTION(".init.init32") init32_cons (void) {};


#else

#if !defined(CONFIG_KDB_COMPORT)
#define CONFIG_KDB_COMPORT 0x3f8
#endif

#if !defined(CONFIG_KDB_COMSPEED)
#define CONFIG_KDB_COMSPEED 115200
#endif

#define INIT32_COMPORT		CONFIG_KDB_COMPORT
#define INIT32_RATE		CONFIG_KDB_COMSPEED

void inline SECTION(".init.init32") init32_out(const u16_t port, const u8_t val)
{
    /* GCC can optimize here if constant */
    __asm__ __volatile__("outb	%1, %0\n"
                         :
                         : "dN"(port), "a"(val));
}

static inline u8_t SECTION(".init.init32") init32_in(const u16_t port)
{
    u8_t tmp;
    /* GCC can optimize here if constant */
    __asm__ __volatile__("inb %1, %0\n"
                         : "=a"(tmp)
                         : "dN"(port));
    return tmp;
}

static inline void SECTION(".init.init32") init32_putc(const char c)
{
    while ((init32_in(INIT32_COMPORT+5) & 0x60) == 0);
    init32_out(INIT32_COMPORT,c);
}

static void SECTION(".init.init32")  init32_spin (int pos = 0)
{
    char c= 'A';
    while (1)
    {
	if (c++ == 'Z')
	    c = 'A';
	for (int i=0; i < pos; i++)
	    init32_putc(' ');
	init32_putc(c);
	init32_putc('\r');

    }
}

static void SECTION(".init.init32") init32_cons (void)
{
#define IER	(INIT32_COMPORT+1)
#define EIR	(INIT32_COMPORT+2)
#define LCR	(INIT32_COMPORT+3)
#define MCR	(INIT32_COMPORT+4)
#define LSR	(INIT32_COMPORT+5)
#define MSR	(INIT32_COMPORT+6)
#define DLLO	(INIT32_COMPORT+0)
#define DLHI	(INIT32_COMPORT+1)

    init32_out(LCR, 0x80);		/* select bank 1	*/
# if !defined(CONFIG_CPU_X86_SIMICS)
    //for (volatile int i = 10000000; i--; );
#endif
    init32_out(DLLO, (((115200/INIT32_RATE) >> 0) & 0x00FF));
    init32_out(DLHI, (((115200/INIT32_RATE) >> 8) & 0x00FF));
    init32_out(LCR, 0x03);		/* set 8,N,1		*/
    init32_out(IER, 0x00);		/* disable interrupts	*/
    init32_out(EIR, 0x07);		/* enable FIFOs	*/
    init32_out(IER, 0x01);		/* enable RX interrupts	*/
    init32_in(IER);
    init32_in(EIR);
    init32_in(LCR);
    init32_in(MCR);
    init32_in(LSR);
    init32_in(MSR);
}

#endif


/**
 * 
 * Initializes paged 64bit mode
 * 
 * Precondition: paged or non paged protected mode
d */

extern "C" void SECTION(".init.init32") init_paging( u32_t is_ap )
{
   
    init32_cons();
    /* 
     * Test if long mode is available
     * as long as no real console is available:
     * spin(x) signalizes error at character position x
     */ 
	
    if (!x86_mmu_t::has_long_mode())
	init32_spin(1);


    for (int i=0; i<512; i++){
	init_pml4[i] = init_pdp[i] = init_pdir[i] = 0;
    }

    
    /* Create a pagetable hierarchy */
    
    /* ugly...
     * otherwise we override our own pdir entries or we make entries
     * outside the pdir
     */
    
    if  (X86_X64_PDIR_IDX(KERNEL_OFFSET_TYPED) != 0 && 
	 (X86_X64_PDIR_IDX(KERNEL_OFFSET_TYPED) < INIT32_PDIR_ENTRIES ||
	  X86_X64_PDIR_IDX(KERNEL_OFFSET_TYPED) + INIT32_PDIR_ENTRIES > 512ULL))
	init32_spin(2);
    
    for (int i=0; i< INIT32_PDIR_ENTRIES; i++){
    	/* the pdir (used twice!) maps 1 GByte */
	init_pdir[i] = init_pdir[i + X86_X64_PDIR_IDX(KERNEL_OFFSET_TYPED)] =
	    ((i << X86_SUPERPAGE_BITS) | (INIT32_PDIR_ATTRIBS & X86_SUPERPAGE_FLAGS_MASK));
    }

    /* 
     * the pdp maps 512 GByte
     * we use it twice for high and and low mapping, since it's only a 
     * preliminary pagetable hierarchy
     */
   
    
    init_pdp[0] = init_pdp[X86_X64_PDP_IDX(KERNEL_OFFSET_TYPED) ] =\
	(u64_t) ( ( (u32_t) (init_pdir)) | INIT32_PTAB_ATTRIBS);

    
    /* 
     * the pml4 needs 2 entries: 
     * one for the first 512 GByte, one for the last 512 GByte
     */
   
    init_pml4[0] = init_pml4[X86_X64_PML4_IDX(KERNEL_OFFSET_TYPED) ] =\
	(u64_t) ( ( (u32_t) (init_pdp)) | INIT32_PTAB_ATTRIBS);


    /* Disable Paging (Vol. 2, 14.6.1) */
    x86_mmu_t::disable_paging();
	 
    /* Enable PAE mode - required before  long mode */
    x86_mmu_t::enable_pae_mode();
     
    /* Enable long mode (not active unless paging is enabled) */
    x86_mmu_t::enable_long_mode();
	 
    /* Set pagemap base pointer (CR3) */
    x86_mmu_t::set_active_pagetable((u64_t) ((u32_t)init_pml4));
	 
    /* Enable paged mode */
    x86_mmu_t::enable_paging();
	 
    /* Success ?  */
    if (!(x86_mmu_t::long_mode_active()))
	init32_spin(3);
	 
    /* Set up temporary GDT (true long mode needs 64bit Code Segment) */
    init32_gdt[0].set_seg((u32_t)0, x86_segdesc_t::inv, 0, x86_segdesc_t::m_comp);
    init32_gdt[1].set_seg((u32_t)0, x86_segdesc_t::code, 0, x86_segdesc_t::m_long);
	 
    /* Install temporary GDT */
    x86_descreg_t gdtr((word_t) init32_gdt, sizeof(init32_gdt));
    gdtr.setdescreg(x86_descreg_t::gdtr);
	 
    /*
     * Search startup_system (see linker script)
     */ 
    u8_t *startup64;
    
    asm("	call 1f			\n\t"     /* retrieve ip of next instruction */
	"1:	popl %0 		\n\t"     /* save in addr  */
	: "=r" (startup64)
	);
    
    startup64 += 1023;
    
    bool found = false;
    for (int n=0; n<4096; n++)
    {
	if ( *++startup64 != 0x90)
	{
	    found = true;
	    break;
	}
    }
    
    if (found == false)
	init32_spin(4);

    /* 
     * Load new (64-bit) code segment, 
     * use it as data segment, too 
     * and leave this type-casting hell... 
     */
    
    asm("	mov  %0, %%ds		\n\t"	
	"	pushw %1		\n\t"     /* push segment selector, used by ljmp */
	"	lea %2, %%eax   	\n\t"     /* load startup_system */	
	"	pushl %%eax		\n\t"     /* load startup_system */	
	"       movl %3, %%edi          \n\t"     /* pass AP info to startup_system */
	"	ljmp *(%%esp)		\n\t"     /* jump to startup_system and load new CS */
	: /* No Output */ 
	: 
	"r" (INIT32_CS),
	"i" (INIT32_CS),
	"m" (*startup64),
	"r" (is_ap)
	);
    
    init32_spin(5);
}
