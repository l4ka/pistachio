/*********************************************************************
 *                
 * Copyright (C) 2002,  Karlsruhe University
 *                
 * File path:     glue/v4-ia64/memory.h
 * Description:   V4 IA-64 memory defines
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
 * $Id: memory.h,v 1.3 2003/09/24 19:04:37 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_IA64__MEMORY_H__
#define __GLUE__V4_IA64__MEMORY_H__

/*
 * Symbols defined by linker script.
 */

/* Boot memory */
extern char _start_bootmem[];
extern char _end_bootmem[];
extern char _start_bootmem_phys[];
extern char _end_bootmem_phys[];

/* Kernel code and data */
extern char _start_text_phys[];
extern char _end_text_phys[];
extern char _start_text[];
extern char _end_text[];

/* CPU local memory */
extern char _start_cpu_local[];
extern char _end_cpu_local[];
extern char _start_cpu_local_mem[];
extern char _end_cpu_local_mem[];

/*
 * Wrapper macros to access linker symbols.
 */

#define start_text_phys		((addr_t) _start_text_phys)
#define end_text_phys		((addr_t) _end_text_phys)
#define start_bootmem_phys	((addr_t) _start_bootmem_phys)
#define end_bootmem_phys	((addr_t) _end_bootmem_phys)
#define start_bootmem		((addr_t) _start_bootmem)
#define end_bootmem		((addr_t) _end_bootmem)
#define start_cpu_local		((addr_t) _start_cpu_local)
#define end_cpu_local		((addr_t) _end_cpu_local)
#define start_cpu_local_mem	((addr_t) _start_cpu_local_mem)
#define end_cpu_local_mem	((addr_t) _end_cpu_local_mem)


#endif /* !__GLUE__V4_IA64__MEMORY_H__ */
