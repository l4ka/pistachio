/********************************************************************
 *
 * Copyright (C) 2003-2004,  National ICT Australia (NICTA)
 *
 * File path:     arch/arm/fass.c
 * Description:   FASS functionality
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
 * $Id: fass.cc,v 1.13 2004/09/08 08:35:25 cvansch Exp $
 *
 ********************************************************************/

#ifdef CONFIG_ENABLE_FASS

#include INC_API(space.h)
#include INC_ARCH(fass.h)
#include INC_API(tcb.h)
#include <kdb/tracepoints.h>

DECLARE_TRACEPOINT(ARM_FASS_RECYCLE);

arm_fass_t arm_fass;

void arm_fass_t::init(void)
{
    arm_fass.domain_space[0] = get_kernel_space();
    get_kernel_space()->set_domain(0);
    next_rr = 1;
}

void arm_fass_t::add_set(arm_domain_t domain, word_t section)
{
    ASSERT(domain < ARM_DOMAINS);

    cpd_set[domain][CPD_BITFIELD_POS(section)] |= 
            (1 << CPD_BITFIELD_OFFSET(section));
}

void arm_fass_t::remove_set(arm_domain_t domain, word_t section)
{
    ASSERT(domain < ARM_DOMAINS);

    cpd_set[domain][CPD_BITFIELD_POS(section)] &= 
            ~(1 << CPD_BITFIELD_OFFSET(section));
}

void arm_fass_t::clean_all(void)
{
    domain_dirty = utcb_dirty = 0;

    arm_cache::cache_flush();
    arm_cache::tlb_flush();
}

/* Chose a domain to replace - select a clean domain if available, otherwise
 * use round-robin. Should consider using a more sophisticated selection if
 * it will buy anything (for example, consider the number of CPD slots used by
 * the domains). Also, should consider moving this to user-level.
 */ 
int arm_fass_t::replacement_domain(void)
{
    unsigned int i;

    for (i = 1; i < ARM_DOMAINS; ++i)
        if (!TEST_BIT_WORD(domain_dirty, i))
            return i;

    do {
	i = next_rr;

	next_rr = (next_rr + 1) % ARM_DOMAINS;
	next_rr = next_rr ? next_rr : 1;	/* Can't use domain 0 */
    } while (i == current_domain);

    return i;
}

arm_domain_t arm_fass_t::domain_recycle(space_t *space)
{
    arm_domain_t target;

    TRACEPOINT(ARM_FASS_RECYCLE,
        printf("Recycling domain for %x\n", space));

    target = replacement_domain();

    if (domain_space[target])
        domain_space[target]->set_domain(INVALID_DOMAIN);

    space->set_domain(target);

    domain_space[target] = space;

    /* Remove the elements in the CPD belonging to the domain to be
     * recycled.
     */
    for (unsigned int i = 0, j; i < CPD_BITFIELD_ARRAY_SIZE; ++i) {
        if ((j = cpd_set[target][i]))
            for (unsigned int k = 0; k < CPD_BITFIELD_WORD_SIZE; ++k)
                if (j & (1 << k)) {
                    word_t section = i * CPD_BITFIELD_WORD_SIZE + k;

                    cpd->pt.pdir[section].clear(cpd, pgent_t::size_1m, true);
                }

        cpd_set[target][i] = 0;
    }

    if (TEST_BIT_WORD(domain_dirty, target))
        clean_all();

    return target;
}

space_t *arm_fass_t::get_space(arm_domain_t domain)
{
    if (domain == INVALID_DOMAIN)
        return NULL;

    return domain_space[domain];
}

void arm_fass_t::set_space(arm_domain_t domain, space_t *space)
{
    if (domain != INVALID_DOMAIN)
        domain_space[domain] = space;
}



#endif /* CONFIG_ENABLE_FASS */
