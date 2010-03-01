/*********************************************************************
 *
 * Copyright (C) 2006-2007,  Karlsruhe University
 *
 * File path:     glue/v4-ia32/hvm/vtlb.h
 * Description:   Full Virtualization Extensions - Generic VTLB
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
 * $Id$
 *
 ********************************************************************/
#ifndef __GLUE__V4_X86__HVM_VTLB_H__
#define __GLUE__V4_X86__HVM_VTLB_H__

#include INC_ARCH_SA(ptab.h)

class arch_hvm_ktcb_t;


class x86_hvm_vtlb_t
{
public:
    bool alloc (space_t *space);
    void free ();

    /* Flush a gphys mapping from all VTLBs. */
    void flush_gphys () 
	{ flush_hpdir(hpdir_paged); flush_hpdir(hpdir_nonpaged); }
    void flush_gphys (addr_t gvaddr)
	{ flush_hpdir(hpdir_paged, gvaddr); flush_hpdir(hpdir_nonpaged, gvaddr); }

    /* Flush a gvirt mapping from the current VTLB. */
    void flush_gvirt () 
	{ flush_hpdir(hpdir);  }
    void flush_gvirt (addr_t gvaddr)
	{ flush_hpdir(hpdir, gvaddr); }

    
    word_t get_active_top_pdir ()
	{ return virt_to_phys ((word_t) hpdir);	}

    void set_guest_top_pdir (pgent_t *pdir)
	{ gpdir = pdir; }

    pgent_t *get_guest_top_pdir ()
	{ return gpdir; }

    void set_pe (bool pe) { flags.pe = pe; hpdir = pe ? hpdir_paged : hpdir_nonpaged; }
    void set_wp (bool wp) { flags.wp = wp; };
    void set_pg (bool pg) { flags.pg = pg; }

    /* Called on a VTLB miss. */
    bool handle_vtlb_miss (addr_t gvaddr, word_t access);

    /* Lookup guest-virtual memory. */
    bool lookup_gphys_addr (addr_t gvaddr, addr_t *gpaddr);

    /* Lookup guest-virtual memory and dump corresponding ptab entry */
    bool dump_ptab_entry (addr_t gvaddr);
	
private:
    
    /* Insert mapping into VTLB. */
    void set_gphys_entry (addr_t gvaddr, addr_t gpaddr, pgent_t::pgsize_e gvpgsz, 
			  word_t rwx, word_t attrib, bool kernel, bool global,
			  word_t access);

    /* Insert actual VTLB entry. */
    void set_hphys_entry (addr_t gvaddr, addr_t hpaddr, pgent_t::pgsize_e hpgsz, 
		    word_t rwx, word_t attrib, bool kernel, bool global);


private:
    
    /* Flush the VTLB or the entry related to an address. */
    void flush_hpdir (pgent_t *pdir);
    void flush_hpdir (pgent_t *pdir, addr_t gvaddr);

    /* Unpaged host pdir */
    pgent_t *hpdir_nonpaged;
    pgent_t *hpdir_paged;

    /* Current host pdir (shadow page table) */
    pgent_t *hpdir;
    
    /* Guest pdir ptr */
    pgent_t *gpdir;
    /* Guest physical space */
    space_t	*space;
    
    /* Virtual register change flags */
    struct {
	bool wp;
	bool pg;
	bool pe;
    } flags;

};



#endif /* !__GLUE__V4_X86__HVM_VTLB_H__ */
