/*********************************************************************
 *                
 * Copyright (C) 2005,  Karlsruhe University
 *                
 * File path:     pistachio/user/serv/sigma0/sigma0.h
 * Description:   Global sigma0 stuff
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
 * $Id: sigma0.h,v 1.4 2005/06/22 15:51:57 stoess Exp $
 *                
 ********************************************************************/
#ifndef __SIGMA0_H__
#define __SIGMA0_H__

#include <l4/kip.h>


/*
 * Some API defined thread IDs.
 */
extern L4_ThreadId_t kernel_id;	// Lowest possible system thread
extern L4_ThreadId_t sigma0_id;
extern L4_ThreadId_t sigma1_id;
extern L4_ThreadId_t rootserver_id;


/**
 * Minimum page size (log2) supported by architecture/kernel
 * (initialized from kernel interface page).
 */
extern L4_Word_t min_pgsize;

/**
 * Verbose level for sigma0 output.
 */
extern int verbose;


/**
 * Pointer to sigma0's kernel interface page.
 */
extern L4_KernelInterfacePage_t * kip;


#ifndef NULL
#define NULL ((void *) 0)
#endif


/**
 * Do printout if indicated verboseness exceeds or equals current
 * verbose level.
 *
 * @param v		verbose level to match
 * @param args...	arguments passed on to printf()
 */
#define dprintf(v, args...) \
    do { if (verbose > (v)) printf (args); } while(0)


/**
 * Check if thread is a kernel thread.
 * @param t	thread id to check
 * @return true if thread is a kernel thread, false otherwise
 */
L4_INLINE bool is_kernel_thread (L4_ThreadId_t t)
{
    return (t.raw < sigma0_id.raw) && (t.raw >= kernel_id.raw);
}



/* From sigma0_mem.cc */
void init_mempool (void);
void dump_mempools (void);
bool allocate_page (L4_ThreadId_t tid, L4_Word_t addr, L4_Word_t log2size,
		    L4_MapItem_t & map, bool only_conventional = false);
bool allocate_page (L4_ThreadId_t tid, L4_Word_t log2size, L4_MapItem_t & map);

#if defined(L4_ARCH_IA32) || defined(L4_ARCH_AMD64)
#include <l4/arch.h>

#define L4_IO_PAGEFAULT		(-8UL << 20)
#define L4_IO_PORT_START	(0)
#define L4_IO_PORT_END		(1<<16)

/* From sigma0_io.cc */
void init_iopool (void);
bool allocate_iopage (L4_ThreadId_t tid, L4_Fpage_t iofp, L4_MapItem_t & map);
#endif

#endif /* !__SIGMA0_H__ */
