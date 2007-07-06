/*********************************************************************
 *
 * Copyright (C) 2003,  University of New South Wales
 *
 * File path:     contrib/elf-loader/platform/pleb/main.cc
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
 * $Id: main.cc,v 1.6 2004/06/04 07:53:18 htuch Exp $
 *
 ********************************************************************/

#include <l4io.h>
#include <elf-loader.h>

#define PHYS_OFFSET 0x00000000

extern L4_KernelConfigurationPage_t *kip;

extern "C" void print_byte(char c);

extern "C" void putc(int c)
{
    print_byte(c);

    if (c == '\n')
        print_byte('\r');
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

void start_kernel(L4_Word_t bootaddr)
{
    void (*func)(unsigned long) = (void (*)(unsigned long)) (bootaddr - 0x2FB00000);

    /* XXX - Get this from boot loader FIXME
    kip->MainMem.high = 16UL * 1024 * 1024; */
    kip->MemoryInfo.n = 0;

    printf("Jumping to kernel @ %p\n", func);

    func(0);
}

int main(void)
{
    L4_Word_t entry;

    if (load_modules(&entry, PHYS_OFFSET)) {
        putc('!');
        for (;;)
            ;
    }

    start_kernel(entry);

    putc('!');
    putc('!');
    for (;;)
        ;
}

