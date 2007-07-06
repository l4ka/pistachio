/*********************************************************************
 *
 * Copyright (C) 2003-2004,  National ICT Australia (NICTA)
 *
 * File path:     arch/arm/fass.h
 * Description:   FASS functions and defines
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
 * $Id: fass.h,v 1.10 2004/09/08 08:38:58 cvansch Exp $
 *
 ********************************************************************/

#ifndef __ARCH__ARM__FASS_H__
#define __ARCH__ARM__FASS_H__

#define ARM_DOMAINS    16

#define KERNEL_DOMAIN  0
#define INVALID_DOMAIN ARM_DOMAINS

#if !defined(ASSEMBLY)

typedef unsigned int arm_domain_t;
typedef unsigned char arm_pid_t;

class space_t;
extern space_t *cpd;

#define SET_BIT_WORD(w,n)  ((w) = (w) | (1 << (n)))
#define TEST_BIT_WORD(w,n) ((w) & (1 << (n)))

extern unsigned int utcb_dirty;
extern unsigned int domain_dirty;
extern unsigned int current_domain;
extern unsigned int current_pid;

typedef unsigned int cpd_bitfield_t;

#define CPD_BITFIELD_WORD_SIZE (8*sizeof(cpd_bitfield_t))
#define CPD_BITFIELD_ARRAY_SIZE ((1 << ARM_SECTION_BITS) /  \
        CPD_BITFIELD_WORD_SIZE)

#define CPD_BITFIELD_POS(section) (section / CPD_BITFIELD_WORD_SIZE)
#define CPD_BITFIELD_OFFSET(section) (section % CPD_BITFIELD_WORD_SIZE)

class arm_fass_t {
public:
    void init(void);
    void clean_all(void);
    void add_set(arm_domain_t domain, word_t section);
    void remove_set(arm_domain_t domain, word_t section);
    int set_member(arm_domain_t domain, word_t section);
    void activate_domain(space_t *space);
    space_t *get_space(arm_domain_t domain);
    void set_space(arm_domain_t domain, space_t *space);

private:
    int replacement_domain(void);
    arm_domain_t domain_recycle(space_t *space);

private:
    cpd_bitfield_t cpd_set[ARM_DOMAINS][CPD_BITFIELD_ARRAY_SIZE];
    space_t *domain_space[ARM_DOMAINS];
    unsigned int next_rr;
};

extern arm_fass_t arm_fass;
#endif

#endif /* __ARCH__ARM__FASS_H__ */
