/*********************************************************************
 *                
 * Copyright (C) 2002-2008,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/x64/space.h
 * Description:   AMD64 space_t implementation
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
 * $Id: space.h,v 1.20 2006/11/14 18:44:56 skoglund Exp $
 * 
 ********************************************************************/
#ifndef __GLUE_V4_X86__X64__SPACE_H__
#define __GLUE_V4_X86__X64__SPACE_H__

#include <debug.h>
#include INC_API(types.h)
#include INC_API(fpage.h)
#include INC_API(thread.h)
#include INC_ARCH(pgent.h)
#include INC_ARCH(atomic.h)
#include INC_GLUE(config.h)

#if defined(CONFIG_X86_IO_FLEXPAGES)
#include INC_GLUE(io_space.h)
#endif

// Even if new MDB is not used we need the mdb_t::ctrl_t
#include <mdb.h>

/* forward declarations - space_t depends on tcb_t and utcb_t */
class tcb_t;
class utcb_t;

#define PGSIZE_UTCB	(pgent_t::size_4k)
#define PGSIZE_KTCB	(pgent_t::size_4k)
#define PGSIZE_KERNEL	((KERNEL_PAGE_SIZE == X86_SUPERPAGE_SIZE) ? pgent_t::size_2m : pgent_t::size_4k)
#define PGSIZE_SIGMA    pgent_t::size_2m

   
/**
 * The address space representation
 */
class x86_space_t {
public:
    enum access_e {
	read		= 0,
	write		= 2,
	readwrite	= -1,
	execute		= 16
    };
    
protected:
    class kernel_pdp_t {
    public:
	union {
	    x86_pgent_t pdpe[512];
	    struct {	
		/* Copy area */
		x86_pgent_t copy_area[COPY_AREA_COUNT][COPY_AREA_SIZE >> X86_X64_PDP_BITS];
		/* Kernel area */
		x86_pgent_t reserved[512 - 6 - COPY_AREA_COUNT * ((COPY_AREA_SIZE >> X86_X64_PDP_BITS))];
		x86_pgent_t ktcb;
		x86_pgent_t remap32[4];
		x86_pgent_t kernel_area;
	    } __attribute__((aligned(X86_PTAB_BYTES)));
	};
    };

    class top_pdir_t {
    public:
	union {
	    pgent_t pgent[512];
	    struct {
		pgent_t user_area[X86_X64_PML4_IDX(USER_AREA_END)];
		space_t * space; /* space backlink */
		pgent_t kernel_pdp;
	    } __attribute__((aligned(X86_PTAB_BYTES)));
	};
	pgent_t *get_kernel_pdp_pgent()
	    {  return kernel_pdp.subtree((space_t *) this, pgent_t::size_512g); }
	kernel_pdp_t *get_kernel_pdp()
	    {  return (kernel_pdp_t *) get_kernel_pdp_pgent(); }

    };

    struct
    {	
	struct {
	    /* CPU-specific ptabs */
	    top_pdir_t* top_pdir;
	    atomic_t thread_count;
	} cpu_ptab [CONFIG_SMP_MAX_CPUS];
	word_t reference_ptab;
	fpage_t kip_area;
	fpage_t utcb_area;
	atomic_t thread_count;
#if defined(CONFIG_X86_IO_FLEXPAGES)
	io_space_t *io_space;
#endif
#if defined(CONFIG_X86_COMPATIBILITY_MODE)
	word_t compatibility_mode;
#endif		    
    } data;

public:

    static const addr_t sign_extend(addr_t addr) 
	{ return (addr_t) ((word_t) addr | X86_X64_SIGN_EXTENSION); }
    
#if defined(CONFIG_X86_COMPATIBILITY_MODE)
    /* Compatibility Mode specific functions */
    bool is_compatibility_mode() { return data.compatibility_mode == true; }
#endif

} __attribute__((aligned(X86_PTAB_BYTES)));







#endif /* !__GLUE_V4_X86__X64__SPACE_H__ */
