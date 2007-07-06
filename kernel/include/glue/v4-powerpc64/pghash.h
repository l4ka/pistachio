/****************************************************************************
 *
 * Copyright (C) 2003,  National ICT Australia (NICTA)
 *
 * File path:	glue/v4-powerpc64/pghash.h
 * Description:	PowerPC64 page hash handler.
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
 * $Id: pghash.h,v 1.4 2004/06/04 02:52:57 cvansch Exp $
 *
 ***************************************************************************/

#ifndef __GLUE__V4_POWERPC64__PGHASH_H__
#define __GLUE__V4_POWERPC64__PGHASH_H__

#include INC_ARCH(pghash.h)
#include INC_GLUE(pgent.h)

class space_t;

class pghash_t
{
protected:
    ppc64_htab_t htab;

    bool try_location( word_t phys_start, word_t size );
    bool finish_init( word_t phys_start, word_t size );

public:
    ppc64_htab_t *get_htab() { return &this->htab; }

    bool init( word_t tot_phys_mem );

    void update_mapping( space_t *s, addr_t vaddr, pgent_t *pgent,
		    pgent_t::pgsize_e size );
    void insert_mapping( space_t *s, addr_t vaddr, pgent_t *pgent,
		    pgent_t::pgsize_e size, bool bolted = false );
    void flush_mapping( space_t *s, addr_t vaddr, pgent_t *pgent, pgent_t::pgsize_e size );
};

INLINE pghash_t *get_pghash()
{
    extern pghash_t pghash;
    return &pghash;
}

#endif	/* __GLUE__V4_POWERPC64__PGHASH_H__ */
