/*********************************************************************
 *                
 * Copyright (C) 2003-2004, 2006,  Karlsruhe University
 *                
 * File path:     elf.h
 * Description:   Struct and defs for simple ELF loader
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
 * $Id: elf.h,v 1.10 2006/10/22 19:41:23 reichelt Exp $
 *                
 ********************************************************************/
#ifndef __KICKSTART__ELF_H__
#define __KICKSTART__ELF_H__

/**
   \file        elf.h
   \brief       Rudimentary ELF file format structures

   Assumptions:

   Unless ELF_32on64 or ELF_64on32 is defined we build for the ELF
   support of the current platform.  If either of the two defines are
   set we build ELF support for 32 or 64 bit platforms respectively.
   
*/

#include <l4/types.h>

#include "bootinfo.h"

#if defined(ELF_32on64)
# define L4_32BIT
# undef  L4_64BIT
# define L4_Word_t L4_Word32_t

#elif defined(ELF_64on32)
# undef  L4_32BIT
# define L4_64BIT
# define L4_Word_t L4_Word64_t

#endif


#if defined(L4_32BIT)
# define __elf_func(x)	x##32
#else
# define __elf_func(x)	x##64
#endif


/*
 * ELF header
 */

class ehdr_t 
{
public:
    unsigned char ident[16];
    L4_Word16_t type;
    L4_Word16_t machine;
    L4_Word32_t version;
    L4_Word_t   entry;          // Program start address
    L4_Word_t   phoff;          // File offset of program header table
    L4_Word_t   shoff;
    L4_Word32_t flags;
    L4_Word16_t ehsize;         // Size of this ELF header
    L4_Word16_t phentsize;      // Size of a program header
    L4_Word16_t phnum;          // Number of program headers
    L4_Word16_t shentsize;
    L4_Word16_t shnum;
    L4_Word16_t shstrndx;

    bool is_32bit (void) { return ident[4] == 1; }
    bool is_64bit (void) { return ident[4] == 2; }
};


/*
 * Program header
 */

class phdr_t
{
public:
    L4_Word32_t type;
#if defined(L4_64BIT)
    L4_Word32_t flags;
#endif
    L4_Word_t   offset;
    L4_Word_t   vaddr;
    L4_Word_t   paddr;
    L4_Word_t   fsize;
    L4_Word_t   msize;
#if defined(L4_32BIT)
    L4_Word32_t	flags;
#endif
    L4_Word_t   align;
};

enum phdr_type_e 
{
    PT_LOAD =   1       /* Loadable program segment */
};

enum phdr_flags_e
{
    PF_X = 	1,
    PF_W = 	2,
    PF_R = 	4
};


/*
 * Section header
 */

class shdr_t
{
public:
    L4_Word32_t	name;
    L4_Word32_t	type;
    L4_Word_t	flags;
    L4_Word_t	addr;
    L4_Word_t	offset;
    L4_Word_t	size;
    L4_Word32_t	link;
    L4_Word32_t	info;
    L4_Word_t	addralign;
    L4_Word_t	entsize;
};

enum shdr_type_e
{
    SHT_PROGBITS =	1,
    SHT_NOBITS =	8
};

enum shdr_flags_e
{
    SHF_WRITE =		1,
    SHF_ALLOC =		2,
    SHF_EXECINSTR =	4
};


#if defined(ELF_32on64)
# undef  L4_32BIT
# define L4_64BIT
# undef  L4_Word_t
#elif defined(ELF_64on32)
# define L4_32BIT
# undef  L4_64BIT
# undef  L4_Word_t
#endif


/*
 * Prototypes.
 */

typedef bool (* L4_MemCheck_Func_t) (L4_Word_t start, L4_Word_t end);

bool elf_load (L4_Word_t file_start,
	       L4_Word_t file_end,
	       L4_Word_t *memory_start,
	       L4_Word_t *memory_end,
	       L4_Word_t *entry,
	       L4_Word_t *type,
	       L4_MemCheck_Func_t check);

bool elf_find_sections (L4_Word_t addr,
			BI32::L4_Boot_SimpleExec_t * exec);

bool elf_find_sections (L4_Word_t addr,
			BI64::L4_Boot_SimpleExec_t * exec);


#endif /* !__KICKSTART__ELF_H__ */
