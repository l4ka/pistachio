/*********************************************************************
 *                
 * Copyright (C) 2002-2003,  University of New South Wales
 *                
 * File path:     elf.cc
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
 * $Id: elf.cc,v 1.1 2004/03/01 19:04:32 stoess Exp $
 *                
 ********************************************************************/

#include "globals.h"

Elf64_Shdr *elf64_next_shdr(Elf64_Ehdr *ehdr, int *index)						
{														
    Elf64_Shdr *result = NULL;

    if (*index < 0 )
	return NULL;
    
    while(*index < ehdr->e_shnum){
	result = (Elf64_Shdr *)(((L4_Word_t) ehdr) + (L4_Word_t)ehdr->e_shoff + *(index) * ehdr->e_shentsize);	
	(*index)++;

 	if (result->sh_type != SHT_NULL){
	    return result;
	}
	
    }
    return NULL;													
}													

Elf64_Phdr *elf64_next_phdr(Elf64_Ehdr *ehdr, int *index)						
{														
    Elf64_Phdr *result = NULL;

    if (*index < 0)
	return NULL;
    
    while(*index < ehdr->e_phnum){
	result = (Elf64_Phdr *)(((L4_Word_t) ehdr) + (L4_Word_t) ehdr->e_phoff + *index * ehdr->e_phentsize);	
	(*index)++;
	
 	if (result->p_type == PT_LOAD){
	    return result;
	}
	
    }
    return NULL;
}													
														
Elf64_Ehdr *valid_elf64(L4_Word_t addr,  L4_Word64_t *entry)					
{														
    char *e_ident = (char *) addr;										
														
    if(e_ident[EI_MAG0] != ELFMAG0 ||										
       e_ident[EI_MAG1] != ELFMAG1 ||										
       e_ident[EI_MAG2] != ELFMAG2 ||										
       e_ident[EI_MAG3] != ELFMAG3)										
        return NULL;												
														
    if(e_ident[EI_CLASS] != ELFCLASS64)									
        return NULL;												
														
    /* Possibly allow for old elf formats */									
    if(e_ident[EI_VERSION] != EV_CURRENT)									
        return NULL;												
														
    Elf64_Ehdr *ehdr = (Elf64_Ehdr *) addr;								
    
    *entry = ehdr->e_entry;											
														
    return ehdr;													
}

void elf64_install_image(Elf64_Ehdr *ehdr,  L4_Word64_t *image_start,  L4_Word64_t *image_end, bool copy)
{														
    Elf64_Phdr *phdr;
    int s_idx = 0;

    L4_Word_t start = ~0;
    L4_Word_t end = 0;
    
    while ((phdr = elf64_next_phdr(ehdr, &s_idx))){ 

	//printf("\t PHDR #%d (%p/%x) to %p\n",
	// s_idx, 
	//(void *) ( (L4_Word_t) ehdr + (L4_Word_t) phdr->p_offset),
	//(L4_Word_t) phdr->p_filesz,
	//(void *) ((L4_Word_t) phdr->p_paddr));

	if (copy)
	    memcpy((void *) ( (L4_Word_t) phdr->p_paddr),\
		   (void *) ( (L4_Word_t) ehdr + (L4_Word_t) phdr->p_offset),\
		   (L4_Word_t) phdr->p_filesz);
	
       
	start <?= ((L4_Word_t) phdr->p_paddr);
	end >?= (((L4_Word_t) phdr->p_paddr) + (L4_Word_t) phdr->p_filesz);
	
	*image_start = (L4_Word64_t) start;
	*image_end = (L4_Word64_t) end;
    }
    
    
}													
														
