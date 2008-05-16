/*********************************************************************
 *                
 * Copyright (C) 2002, 2004-2008,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/x32/space.h
 * Description:   ia32-specific space implementation
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
 * $Id: space.h,v 1.55 2007/01/08 14:15:59 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __GLUE_V4_X86__X32__SPACE_H__
#define __GLUE_V4_X86__X32__SPACE_H__


#include <debug.h>
#include INC_API(types.h)
#include INC_API(fpage.h)
#include INC_API(thread.h)
#include INC_ARCH(atomic.h)
#include INC_ARCH(pgent.h)
#include INC_GLUE(config.h)


#if defined(CONFIG_X86_SMALL_SPACES)
#include INC_ARCH(segdesc.h)
#include INC_GLUE_SA(smallspaces.h)
#endif

#if defined(CONFIG_X86_IO_FLEXPAGES)
#include INC_GLUE(io_space.h)
#endif

// Even if new MDB is not used we need the mdb_t::ctrl_t
#include <mdb.h>

#define PGSIZE_KTCB	(pgent_t::size_4k)
#define PGSIZE_UTCB	(pgent_t::size_4k)
#define PGSIZE_KERNEL	((KERNEL_PAGE_SIZE == X86_SUPERPAGE_SIZE) ? pgent_t::size_4m : pgent_t::size_4k)
#define PGSIZE_KIP	(pgent_t::size_4k)
#define PGSIZE_SIGMA    PGSIZE_KERNEL


class utcb_t;
class tcb_t;
class space_t;

class x86_space_t 
{
public:
    enum access_e {
	read		= 0,
	write		= 2,
	readwrite	= -1,
	execute		= 0
    };

    class top_pdir_t {
    public:
	union {
	    pgent_t pgent[1024];
	    struct {
		x86_pgent_t user[USER_AREA_END >> X86_X32_PDIR_BITS];
		x86_pgent_t small[SMALLSPACE_AREA_SIZE >> X86_X32_PDIR_BITS];
		x86_pgent_t copy_area[COPY_AREA_COUNT][COPY_AREA_SIZE >> X86_X32_PDIR_BITS];
		x86_pgent_t readmem_area[MEMREAD_AREA_SIZE >> X86_X32_PDIR_BITS];
		space_t * space; /* back link ptr, "automagically" invalid */
		/* the rest, e.g., TSS, APIC_MAPPINGS, ... */
	    };
	} ;
    };

public:
    /* Shadow pagetable */
    pgent_t user_pgent[USER_AREA_END >> X86_X32_PDIR_BITS];

    struct {
	/* CPU-specific ptabs */
	struct {
	    top_pdir_t* top_pdir;
	    atomic_t thread_count;
	} cpu_ptab [CONFIG_SMP_MAX_CPUS];
	cpuid_t reference_ptab;

	/* Administrative data */
	fpage_t kip_area;
	fpage_t utcb_area;
	atomic_t thread_count;
	
#if defined(CONFIG_X86_IO_FLEXPAGES)
	io_space_t *	io_space;
#endif
	
#if defined(CONFIG_X86_SMALL_SPACES)
	smallspace_id_t smallid;
	x86_segdesc_t segdesc;
	x86_space_t *prev;
	x86_space_t *next;
#endif
    } data;
    
public:

#if defined(CONFIG_X86_SMALL_SPACES)
    bool make_small (smallspace_id_t id);
    void make_large (void);
    bool is_small (void);
    bool is_smallspace_area (addr_t addr);
    bool sync_smallspace (addr_t addr);
    word_t smallspace_offset (void);
    word_t smallspace_size (void);
    
#   define HAVE_ARCH_FREE_SPACE
    void arch_free (void);
    
    smallspace_id_t *smallid (void)
	{ return &data.smallid; }
    
    x86_segdesc_t * segdesc (void)
	{ return &data.segdesc; }

    x86_space_t * get_prev (void) { return data.prev; }
    x86_space_t * get_next (void) { return data.next; }
    void set_prev (x86_space_t * p) { data.prev = p; }
    void set_next (x86_space_t * n) { data.next = n; }

    void dequeue_polluted (void);
    void enqueue_polluted (void);

#endif

    static const addr_t sign_extend(addr_t addr) { return addr; }
    
} __attribute__((aligned(X86_PAGE_SIZE)));

#if defined(CONFIG_X86_SMALL_SPACES)
INLINE bool x86_space_t::is_small (void)
{
    return smallid ()->is_small ();
}

INLINE bool x86_space_t::is_smallspace_area (addr_t addr)
{
    return (addr >= (addr_t) SMALLSPACE_AREA_START && 
	    addr < (addr_t) SMALLSPACE_AREA_END);
}

INLINE word_t x86_space_t::smallspace_offset (void)
{
    return smallid ()->offset () + SMALLSPACE_AREA_START;
}

INLINE word_t x86_space_t::smallspace_size (void)
{
    return smallid ()->size ();
}

INLINE void x86_space_t::arch_free (void)
{
    make_large ();
    dequeue_polluted ();
}



#endif /* CONFIG_X86_SMALL_SPACES */

#endif /* !__GLUE_V4_X86__X32__SPACE_H__ */
