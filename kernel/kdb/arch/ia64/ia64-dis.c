/*********************************************************************
 *                
 * Copyright (C) 2002,  Karlsruhe University
 *                
 * File path:     kdb/arch/ia64/ia64-dis.c
 * Description:   Wrapper for IA-64 disassembler
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
 * $Id: ia64-dis.c,v 1.8 2003/09/24 19:05:06 skoglund Exp $
 *                
 ********************************************************************/

#define SEC_KDEBUG ".kdebug"

/* define stuff needed by the disassembler */

#define PARAMS(x) x SECTION(SEC_KDEBUG)
#define ATTRIBUTE_UNUSED 
#define abort() do { } while (0)

#define BFD_HOST_64_BIT u64_t
#define BFD_HOST_U_64_BIT u64_t

typedef u64_t	bfd_vma;
typedef u8_t	bfd_byte;

int printf(const char * format, ...);
int sprintf(char* obuf, const char* format, ...);
int fprintf(char* f, const char* format, ...);
typedef int (*fprintf_ftype) (char*, const char*, ...);


typedef struct disassemble_info {
    void* stream;
    int (*read_memory_func)(bfd_vma memaddr,
			    bfd_byte *myaddr, word_t length,
			    struct disassemble_info *info);
    void (*memory_error_func)(int status,
			      bfd_vma memaddr,
			      struct disassemble_info *info);
    int bytes_per_line;
    int display_endian;
    int endian;

    fprintf_ftype fprintf_func;
    void (*print_address_func)(bfd_vma addr, struct disassemble_info *info);
} disassemble_info;







/*
 * functions that the disassembler calls
 */

/* load with enforced little endian */
bfd_vma SECTION(SEC_KDEBUG) bfd_getl64 (bfd_byte* addr)
{
    return (((bfd_vma) addr[7] << 56) | ((bfd_vma) addr[6] << 48) |
	    ((bfd_vma) addr[5] << 40) | ((bfd_vma) addr[4] << 32) |
	    ((bfd_vma) addr[3] << 24) | ((bfd_vma) addr[2] << 16) |
	    ((bfd_vma) addr[1] <<  8) | ((bfd_vma) addr[0]      ));
}

/* read memory */
int SECTION(SEC_KDEBUG) rmf(bfd_vma memaddr,
	bfd_byte *myaddr, word_t length,
	struct disassemble_info *info)
{
    int readmem (addr_t vaddr, addr_t contents, word_t size);

    char* d = (char*) myaddr;
    char* s = (char*) memaddr;
    int len = length;
    while (len--)
    {
#if 1
	if (! readmem ((addr_t) s, (addr_t) d, sizeof (char)))
	    return 1;
#else
	*d = *s;
#endif
	d++; s++;
    };
    return 0;
};

/* respond to memory read error */
void SECTION(SEC_KDEBUG) mef(int status,
	 bfd_vma memaddr,
	 struct disassemble_info *info)
{
    //printf("%s(%x,%x,%x)\n", __FUNCTION__, status, memaddr, info);
    printf("##");
};

/* print address */
void SECTION(SEC_KDEBUG) paf(bfd_vma addr, struct disassemble_info *info)
{
    printf("%p", addr);
};



char SECTION(SEC_KDEBUG) *strcpy(char *dest, const char *src)
{
    char* d = dest;
    const char* s = src;
    do { *d++ = *s; } while (*s++);
    return dest;
};

char SECTION(SEC_KDEBUG) *strcat(char *dest, const char *src)
{
    char* d = dest;
    char* s = (char*) src;
    /* scan for trailing \0 */
    while (*d)
	d++;
    /* copy until end of s, including the \0 */
    while ( (*d++ = *s++) );
    return dest;
};





int print_insn_ia64 (bfd_vma pc, disassemble_info *info) SECTION(SEC_KDEBUG);

int SECTION(SEC_KDEBUG) disas(addr_t pc)
{
    disassemble_info info =
    {
	NULL,
	rmf,
	mef,
	6,
	0,
	0,
	fprintf,
	paf
    };
    return print_insn_ia64((u64_t)pc, &info);
}




#include <../../contrib/disas/cpu-ia64-opc.c>
#include <../../contrib/disas/ia64-dis.c>
#undef PARAMS
#define PARAMS(x) x SECTION(SEC_KDEBUG)
#include <../../contrib/disas/ia64-opc.c>
