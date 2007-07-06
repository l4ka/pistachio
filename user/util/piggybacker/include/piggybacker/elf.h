/****************************************************************************
 *
 * Copyright (C) 2002-2003, Karlsruhe University
 *
 * File path:	include/piggybacker/elf.h
 * Description:	Basic elf file declarations
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
 * $Id: elf.h,v 1.4 2003/09/24 19:06:37 skoglund Exp $
 *
 ***************************************************************************/

#ifndef __PIGGYBACKER__INCLUDE__ELF_H__
#define __PIGGYBACKER__INCLUDE__ELF_H__

#include <l4/types.h>

class elf_ehdr_t
{
public:
    unsigned char e_ident[16];
    L4_Word16_t	  e_type;
    L4_Word16_t	  e_machine;
    L4_Word32_t	  e_version;
    L4_Word_t	  e_entry;
    L4_Word_t	  e_phoff;
    L4_Word_t	  e_shoff;
    L4_Word32_t	  e_flags;
    L4_Word16_t	  e_ehsize;
    L4_Word16_t	  e_phentsize;
    L4_Word16_t	  e_phnum;
    L4_Word16_t	  e_shentsize;
    L4_Word16_t	  e_shnum;
    L4_Word16_t	  e_shstrndx;
};

class elf_phdr_t
{
public:
    L4_Word32_t	p_type;
#if defined(L4_64BIT)
    L4_Word32_t	p_flags;
#endif
    L4_Word_t	p_offset;
    L4_Word_t	p_vaddr;
    L4_Word_t	p_paddr;
    L4_Word_t	p_filesz;
    L4_Word_t	p_memsz;
#if defined(L4_32BIT)
    L4_Word_t	p_flags;
#endif
    L4_Word_t	p_align;
};

class elf_shdr_t
{
public:
    L4_Word32_t	sh_name;
    L4_Word32_t	sh_type;
    L4_Word_t	sh_flags;
    L4_Word_t	sh_addr;
    L4_Word_t	sh_offset;
    L4_Word_t	sh_size;
    L4_Word32_t	sh_link;
    L4_Word32_t	sh_info;
    L4_Word_t	sh_addralign;
    L4_Word_t	sh_entsize;
};

enum phdr_type_e
{
    PT_LOAD = 1,
};

#endif	/* __PIGGYBACKER__INCLUDE__ELF_H__ */
