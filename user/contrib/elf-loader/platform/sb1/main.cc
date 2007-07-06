/*********************************************************************
 *                
 * Copyright (C) 2002,  University of New South Wales
 *                
 * File path:     elf-loader/src/platform/u4600/main.cc
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
 * $Id: main.cc,v 1.3 2004/05/14 05:16:41 cvansch Exp $
 *                
 ********************************************************************/

#include "elf-loader.h"

#define PHYS_OFFSET 0xffffffff80000000

extern L4_KernelConfigurationPage_t *kip;

#define CKSEG1      0xffffffffa0000000
#define PHYS_TO_CKSEG1(n)     (CKSEG1 | (n))

#define DUART_NUM_PORTS		2
#define DUART_PHYS		0x0010060000
#define DUART_PHYS_SIZE		0x100

#define DUART_REG(chan,r)	(DUART_PHYS_SIZE*(chan) + (r))
#define DUART_REG_PHYS(chan,r)	(DUART_PHYS + DUART_REG(chan,r))

#define DUART_STATUS		0x120
#define DUART_RX_RDY		(1ULL<<0)
#define DUART_RX_FFUL		(1ULL<<1)
#define DUART_TX_RDY		(1ULL<<2)
#define DUART_TX_EMT		(1ULL<<3)
#define DUART_OVRUN_ERR		(1ULL<<4)
#define DUART_PARITY_ERR	(1ULL<<5)
#define DUART_FRM_ERR		(1ULL<<6)
#define DUART_RCVD_BRK		(1ULL<<7)

#define DUART_TX_HOLD		0x170

static void 
duart_out(unsigned long reg, unsigned long val)
{
    *((volatile unsigned long *)PHYS_TO_CKSEG1(DUART_REG_PHYS(0,reg))) = val;
}

static unsigned long 
duart_in(unsigned long reg) 
{
    return *((volatile unsigned long *)PHYS_TO_CKSEG1(DUART_REG_PHYS(0,reg)));
}

extern "C" void putc(char c)
{
    while ((duart_in(DUART_STATUS) & DUART_TX_RDY) == 0);
    duart_out(DUART_TX_HOLD, c);

    if (c == '\n')
	    putc('\r');
}

extern "C" void memset (char * p, char c, int size)
{
    for (;size--;)
	*(p++)=c;
}

extern "C" __attribute__ ((weak)) void *
memcpy (void * dst, const void * src, unsigned int len)
{
    unsigned char *d = (unsigned char *) dst;
    unsigned char *s = (unsigned char *) src;

    while (len-- > 0)
	*d++ = *s++;

    return dst;
}


void start_kernel(L4_Word_t bootaddr, L4_Word_t a0)
{
    void (*func)(unsigned long) = (void (*)(unsigned long)) (bootaddr | PHYS_OFFSET);

    /* XXX - Get this from boot loader */
    kip->MainMem.high = 64UL * 1024 * 1024;
    kip->MemoryInfo.n = 0;
    
    __asm__ __volatile__ (
	".set noreorder;"
	"lui     $4,0x8000	\n\r"
	"lui     $2,0x8000	\n\r"
	"ori     $2,$2,0x8000	\n\r"
	"nop	\n\r"
	"1:	\n\r"
	"cache   0x1,0($4)	\n\r"
	"cache   0x1,32($4)	\n\r"
	"cache   0x1,64($4)	\n\r"
	"cache   0x1,96($4)	\n\r"
	"cache   0x1,128($4)	\n\r"
	"cache   0x1,160($4)	\n\r"
	"cache   0x1,192($4)	\n\r"
	"cache   0x1,224($4)	\n\r"
	"cache   0x1,256($4)	\n\r"
	"cache   0x1,288($4)	\n\r"
	"cache   0x1,320($4)	\n\r"
	"cache   0x1,352($4)	\n\r"
	"cache   0x1,384($4)	\n\r"
	"cache   0x1,416($4)	\n\r"
	"cache   0x1,448($4)	\n\r"
	"cache   0x1,480($4)	\n\r"
	"cache   0x1,512($4)	\n\r"
	"cache   0x1,544($4)	\n\r"
	"cache   0x1,576($4)	\n\r"
	"cache   0x1,608($4)	\n\r"
	"cache   0x1,640($4)	\n\r"
	"cache   0x1,672($4)	\n\r"
	"cache   0x1,704($4)	\n\r"
	"cache   0x1,736($4)	\n\r"
	"cache   0x1,768($4)	\n\r"
	"cache   0x1,800($4)	\n\r"
	"cache   0x1,832($4)	\n\r"
	"cache   0x1,864($4)	\n\r"
	"cache   0x1,896($4)	\n\r"
	"cache   0x1,928($4)	\n\r"
	"cache   0x1,960($4)	\n\r"
	"cache   0x1,992($4)	\n\r"
	"daddiu  $4,$4,1024	\n\r"
	"sltu    $3,$4,$2	\n\r"
	"bnez    $3,1b	\n\r"
	"nop	\n\r"
	".set reorder;"
	::: "$4", "$2", "$3"
    );
    
    func(a0);
}

int main(L4_Word_t a0)
{
    unsigned int temp;
    /* Disable caches */
    __asm__ __volatile__ (
	"mfc0    %0,$16;    \n\t"
	"li      $31,-8;    \n\t"
	"and     %0,%0,$31;  \n\t"
//#if defined (CONFIG_SB1_PASS1_WORKAROUNDS)
	"ori     %0,%0,0x5; \n\t" /* COW hack */
//#else
//	"ori     %0,%0,0x2; \n\t" /* uncached */
//#endif
	"mtc0    %0,$16;    \n\t"
	: "=r" (temp) : : "$31"
    );

    L4_Word_t entry;
    
    if(load_modules(&entry, PHYS_OFFSET)) {
	asm ("break\n\t");
    }

    start_kernel(entry, a0);
    
    asm ("break\n\t");
}
