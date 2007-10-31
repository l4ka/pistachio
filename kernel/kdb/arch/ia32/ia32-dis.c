/*********************************************************************
 *                
 * Copyright (C) 2002-2004, 2007,  Karlsruhe University
 *                
 * File path:     kdb/arch/ia32/ia32-dis.c
 * Description:   Wrapper for IA-32 disassembler
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
 * $Id: ia32-dis.c,v 1.9 2004/08/24 19:40:14 ud3 Exp $
 *                
 ********************************************************************/


#define _(x) x
#define VOLATILE __volatile__
#define PTR char *
#define PARAMS(x) x
#define ATTRIBUTE_UNUSED
#define bfd_mach_i386_i386 0
#define bfd_mach_i386_i8086 1
#define bfd_mach_i386_i386_intel_syntax 2
#define bfd_mach_x86_64 64
#define bfd_mach_x86_64_intel_syntax 65
#define abort() do { } while (0)
#define sprintf_vma(s,x) sprintf (s, "%08lx", x)


#define SEC_KDEBUG ".kdebug"

int printf(const char* format, ...);
int sprintf(char* s, const char* format, ...);
int fprintf(char* f, const char* format, ...);
static inline int SECTION(SEC_KDEBUG) strlen(const char* p) { int i=0; while (*(p++)) i++; return i; };


typedef struct jmp_buf { u32_t eip; u32_t esp; u32_t ebp; int val; } jmp_buf[1];

static inline void SECTION(SEC_KDEBUG) longjmp(jmp_buf env, int val)
{
#if 0
    printf("%s(%x,%x) called from %x\n", __FUNCTION__, &env, val,
	   ({u32_t x;__asm__ __volatile__("call 0f;0:popl %0":"=r"(x));x;}));
    printf("jumping to eip=%x esp=%x, ebp=%x\n",
	   env->eip, env->esp, env->ebp);
#endif

    env->val = val;
    __asm__ __volatile__ (
	"jmp	*(%%ebx)	\n\t"
	"orl	%%esp,%%esp	\n\t"
	:
	: "a" (&env->ebp), "c"(&env->esp), "b" (&env->eip)
	: "edx", "esi", "edi", "memory");
    while(1);
};
static inline int SECTION(SEC_KDEBUG) setjmp(jmp_buf env)
{
#if 0
    printf("%s(%x) called from %x\n", __FUNCTION__, &env,
	   ({u32_t x;__asm__ __volatile__("call 0f;0:popl %0":"=r"(x));x;}));
#endif
    env->val = 0;
    __asm__ __volatile__ (
	"movl	%%ebp, (%0)	\n\t"
	"movl	%%esp, (%1)	\n\t"
	"movl	$0f, (%2)	\n\t"
	"0:			\n\t"
	"movl	%%esp,%%esp	\n\t"
	"movl	(%1),%%esp	\n\t"
	"movl	(%0),%%ebp	\n\t"
	:
	: "a" (&env->ebp), "c"(&env->esp), "b" (&env->eip)
	: "edx", "esi", "edi", "memory");
#if 0
    printf("prepared to return to eip=%x esp=%x, ebp=%x\n",
	   env->eip, env->esp, env->ebp);
#endif	
    return env->val;
};




typedef u32_t bfd_vma;
typedef s32_t bfd_signed_vma;
typedef u8_t bfd_byte;



#define MAXLEN 20
typedef struct dis_private
{
  /* Points to first byte not fetched.  */
  bfd_byte *max_fetched;
  bfd_byte the_buffer[MAXLEN];
  bfd_vma insn_start;
  jmp_buf bailout;
  int orig_sizeflag;
} dis_private;



typedef int (*fprintf_ftype) PARAMS((PTR, const char*, ...));

typedef struct disassemble_info {
    struct dis_private *private_data;
    int (*read_memory_func)(bfd_vma memaddr,
			    bfd_byte *myaddr, int length,
			    struct disassemble_info *info);
    void (*memory_error_func)(int status,
			      bfd_vma memaddr,
			      struct disassemble_info *info);
    int mach;
    int bytes_per_line;
    char* stream;
    fprintf_ftype fprintf_func;
    void (*print_address_func)(bfd_vma addr, struct disassemble_info *info);
} disassemble_info;


char SECTION(SEC_KDEBUG) *strncpy(char *dest, const char *src, unsigned int n)
{
    char* d = dest;
    const char* s = src;
    do { *d++ = *s; } while (*s++ && (--n));
    return dest;
};
char SECTION(SEC_KDEBUG) *strcpy(char *dest, const char *src)
{
    char* d = dest;
    const char* s = src;
    do { *d++ = *s; } while (*s++);
    return dest;
};

extern int kdb_disas_readmem(char * s, char * d);

int SECTION(SEC_KDEBUG) rmf(bfd_vma memaddr,
	bfd_byte *myaddr, int length,
	struct disassemble_info *info)
{
    char* d = (char*) myaddr;
    char* s = (char*) memaddr;
    int len = length;
    while (len--)
    {
#if 1
	if (!kdb_disas_readmem (s, d))
	    return 1;
#else
	*d = *s;
#endif
	d++; s++;
    };
    return 0;
};

void SECTION(SEC_KDEBUG) mef(int status,
	 bfd_vma memaddr,
	 struct disassemble_info *info)
{
    //printf("%s(%x,%x,%x)\n", __FUNCTION__, status, memaddr, info);
    printf("##");
};

void SECTION(SEC_KDEBUG) paf(bfd_vma addr, struct disassemble_info *info)
{
    printf("%x", addr);
};



int SECTION(SEC_KDEBUG) disas(addr_t pc)
{
    disassemble_info info =
    {
	NULL,
	rmf,
	mef,
	bfd_mach_i386_i386,
	0,
	NULL,
	fprintf,
	paf
    };
    extern int print_insn_i386_att (bfd_vma pc, disassemble_info *info);
    return print_insn_i386_att((bfd_vma) pc, &info);
}






/* and here comes the ugly part */

/* enable ol'style int3 decoder */
//#define __X0_INT3_MAGIC__

#include <../../contrib/disas/ia32-disas.c>


