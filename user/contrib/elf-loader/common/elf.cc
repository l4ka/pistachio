/*********************************************************************
 *                
 * Copyright (C) 2002,  University of New South Wales
 *                
 * File path:     elf-loader/generic/elf64.cc
 * Description:   Generic elf support 
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
 * $Id: elf.cc,v 1.2 2003/09/24 19:06:11 skoglund Exp $
 *                
 ********************************************************************/

/* Kernel includes */

#include <l4/kcp.h>
#include <l4io.h>
#include <string.h>
#include "elf.h"
#include "elf-loader.h"
#include "arch.h"

#define PAGE_MASK    (~(PAGE_SIZE-1))

struct elf_phdr {
    L4_Word_t phys;
    L4_Word_t file_size;
    L4_Word_t mem_size;
    L4_Word_t offset;
};

typedef int (*elf_next_phdr_func)(L4_Word_t addr, int *index, elf_phdr *phdr);

L4_Word_t wrap_up( L4_Word_t val, L4_Word_t size )
{
    if( val % size )
	val = (val + size) & ~(size-1);
    return val;
}
 
/* This evilness is so I don't cut-n-paste code */
#define ELF_FUNCTIONS(size)											\
int elf##size##_next_phdr_func(L4_Word_t addr, int *index, elf_phdr *phdr)						\
{														\
    Elf##size##_Ehdr *ehdr = (Elf##size##_Ehdr *) addr;								\
														\
    if(*index == -1)												\
	*index = ehdr->e_phnum;											\
														\
    if(*index == 0)												\
	return -1;												\
														\
    *index -= 1;												\
														\
    Elf##size##_Phdr *elfphdr = (Elf##size##_Phdr *)(addr + ehdr->e_phoff + *index * ehdr->e_phentsize);	\
    while(elfphdr->p_type != PT_LOAD) {										\
	*index -= 1;												\
	if(*index == -1)											\
	    return -1;												\
														\
	elfphdr = (Elf##size##_Phdr *)(addr + ehdr->e_phoff + *index * ehdr->e_phentsize);			\
    }														\
														\
    phdr->phys = elfphdr->p_paddr;										\
    phdr->file_size = elfphdr->p_filesz;									\
    phdr->mem_size = elfphdr->p_memsz;										\
    phdr->offset = elfphdr->p_offset;										\
														\
    return 0;													\
}														\
														\
int check_elf##size (L4_Word_t addr, L4_Word_t *entry, elf_next_phdr_func *func)					\
{														\
    char *e_ident = (char *) addr;										\
														\
    if(e_ident[EI_MAG0] != ELFMAG0 ||										\
       e_ident[EI_MAG1] != ELFMAG1 ||										\
       e_ident[EI_MAG2] != ELFMAG2 ||										\
       e_ident[EI_MAG3] != ELFMAG3)										\
        return 1;												\
														\
    if(e_ident[EI_CLASS] != ELFCLASS##size)									\
        return 1;												\
														\
    /* Possibly allow for old elf formats */									\
    if(e_ident[EI_VERSION] != EV_CURRENT)									\
        return 1;												\
														\
    Elf##size##_Ehdr *ehdr = (Elf##size##_Ehdr *) addr;								\
														\
    *entry = ehdr->e_entry;											\
    *func = &elf##size##_next_phdr_func;									\
														\
    return 0;													\
}

ELF_FUNCTIONS(32)
ELF_FUNCTIONS(64)

int install_module(const char *name, char *mod_start, char *mod_end, L4_KernelRootServer_t *server, L4_Word_t offset)
{
    L4_Word_t addr = (L4_Word_t) mod_start;
    L4_Word_t entry, start = (L4_Word_t) mod_start, end = (L4_Word_t) mod_end;
    elf_next_phdr_func func;

    /* Clear server */
    if(server)
    {
    	server->ip = 0;
	server->sp = 0;
	server->low = 0;
	server->high = 0;
    }

    if(start == end ) {
	printf("install_module:\tSkipping %s\n", name);
	return 0;	/* Empty module. */
    }

    printf("install_module:\tInstalling %s (0x%lx, 0x%lx)\n", name, start, end);

    if (start & (sizeof(L4_Word_t) - 1))
    {
	printf("install_module:\tModule '%s' is misaligned, ignoring\n", name);
	return -1;
    }

    if(check_elf64(addr, &entry, &func) && check_elf32(addr, &entry, &func)) {
	printf("install_module:\tModule '%s' is not an ELF file, ignoring\n", name);
	return -1;
    }

    if( server ) {
    	server->ip = entry;
	server->sp = 0;
	server->low = -1UL;
	server->high = 0;
    }

    elf_phdr phdr;
    L4_Word_t ehdr = addr;
    int next = -1;
    while(!func(ehdr, &next, &phdr)) {
	L4_Word_t copy_size = phdr.file_size;
	if(phdr.mem_size < copy_size)
	    copy_size = phdr.mem_size;

	printf("install_module:\t%s: (0x%lx, 0x%lx)\n", name, phdr.phys, copy_size); 

	/* Copy the data associated with the program header to its 
	 * target physical address.  I wonder what happens if this
	 * overlaps with the boot loader, or the exception vectors,
	 * or the platform's memory, or some other module?
	 */
	memcpy((void *)(phdr.phys + offset), 
	       (void *)(start + phdr.offset), copy_size);

        /* CEG FIX: zero out the rest */
        {
                char *z;
                unsigned long i;
                z = (char*) (phdr.phys + offset + copy_size);
                for( i = 0; i < phdr.mem_size - copy_size; i++ )
                        z[i] = 0;
        }

	if( server ) {
	    /* Calculate the start page of the region's physical range. */
	    addr = phdr.phys & PAGE_MASK;
    	    if( addr < server->low )
    		server->low = addr;

	    /* Calculate the end page of the region's physical range. */
	    addr = wrap_up(phdr.phys + phdr.mem_size, PAGE_SIZE);
    	    if( addr > server->high )
    		server->high = addr;
	}
    }

    return 1;
}

