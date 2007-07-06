/*********************************************************************
 *                
 * Copyright (C) 2002-2003,  Karlsruhe University
 *                
 * File path:     globals.h
 * Description:   Version 4 kernel-interface page
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
 * $Id: globals.h,v 1.1 2004/03/01 19:04:32 stoess Exp $
 *                
 ********************************************************************/
#include <l4/types.h>
#include <elf.h>
#include <multiboot.h>
#include <stdarg.h>

#define NULL    (0)


/* From elf.cc */
Elf64_Shdr *elf64_next_shdr(Elf64_Ehdr *ehdr, int *index);						
Elf64_Phdr *elf64_next_phdr(Elf64_Ehdr *ehdr, int *index);						
Elf64_Ehdr *valid_elf64(L4_Word_t addr,  L4_Word64_t *entry);					
void elf64_install_image(Elf64_Ehdr *ehdr,  L4_Word64_t *image_start,  L4_Word64_t *image_end, bool copy=1);

/* From string.cc */
int strcmp(const char * cs,const char * ct);
int strncmp(const char * cs,const char * ct, unsigned int count);
void memcpy(void * dst, void * src, unsigned long size);
    
/* From screen.c */
void putc(int c);
char getc(void);

/* From mini-print.c */
int vsnprintf(char *str, int size, const char *fmt, va_list ap);
int snprintf(char *str, int size, const char *fmt, ...);
int printf(const char *fmt, ...);


/* This should be definde by ld */
extern L4_Word_t _binary_kernel_mod_start[];
extern L4_Word_t _binary_kernel_mod_end[];


/**
  *  mem_region_t:
  */

#define MEM_REGION_UND		0x0
#define MEM_REGION_CONVENTIONAL		0x1
#define MEM_REGION_RESERVED		0x2
#define MEM_REGION_DEDICATED		0x3
#define MEM_REGION_SHARED		0x4
#define MEM_REGION_BOOT_SPECIFIC	0xe
#define MEM_REGION_ARCH_SPECIFIC	0xf

typedef struct
{
    L4_Word64_t  type	:4;
    L4_Word64_t  t	:4;
    L4_Word64_t  res0	:1;
    L4_Word64_t  v	:1;
    L4_Word64_t  low	:54;
    L4_Word64_t  res1	:10;
    L4_Word64_t  high	:54;
}  kip_memreg_t;


/**
 * descriptor for one of the initial servers (sigma0, root server, sigma1)
 */

typedef struct 
{
    /** initial stack pointer, physical address */
     L4_Word64_t		sp;
    /** initial instruction pointer, physical address */
     L4_Word64_t		ip;
    /** memory region occupied by this server, physical addresses */
     L4_Word64_t		low;
     L4_Word64_t		high;
} kip_root_server_t;

/**
 * info on location and number of memory descriptors
 */
typedef struct 
{
    L4_Word32_t n;
    kip_memreg_t *memdesc_ptr;

} kip_memory_info_t;


typedef struct{
    union {
	char string[4];
	 L4_Word64_t raw;
    };
} kip_magic_word_t;

typedef struct 
{
     L4_Word64_t                 : 16;
     L4_Word64_t subversion      : 8;
     L4_Word64_t version         : 8;
     L4_Word64_t                 : 32;

} kip_api_version_t;
/**
 * The kernel interface page (KIP)
 */
typedef struct 
{
    kip_magic_word_t	magic;
    kip_api_version_t	api_version;
    L4_Word64_t		api_flags;
    L4_Word64_t		kernel_desc_ptr;

    /* kdebug */
    L4_Word64_t		kdebug_init;
    L4_Word64_t		kdebug_entry;
    kip_memreg_t	kdebug_mem;

    /* root server */
    kip_root_server_t	sigma0;
    kip_root_server_t	sigma1;
    kip_root_server_t	root_server;

    L4_Word64_t		reserved0;
    kip_memory_info_t	memory_info;
    L4_Word64_t		kdebug_config[2];

    /* former memory regions */
    L4_Word64_t		reserved1[16];

    /* info fields */
    L4_Word64_t		reserved2[2];
    L4_Word64_t    	utcb_area_info;
    L4_Word64_t   	kip_area_info;
    
    L4_Word64_t		reserved3[2];
    L4_Word64_t		boot_info;
    L4_Word64_t		proc_desc_ptr;

} kip_t;

