/*********************************************************************
 *
 * Copyright (C) 2003-2004,  National ICT Australia (NICTA)
 *
 * File path:     arch/arm/fass_inline.h
 * Description:   FASS inline functions (needed to break circular dependency)
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
 * $Id: fass_inline.h,v 1.4 2004/06/04 02:14:22 cvansch Exp $
 *
 ********************************************************************/

#ifndef __ARCH__ARM__FASS_INLINE_H__
#define __ARCH__ARM__FASS_INLINE_H__

#ifdef CONFIG_ENABLE_FASS

#include INC_ARCH(fass.h)
#include INC_API(space.h)

INLINE int arm_fass_t::set_member(arm_domain_t domain, word_t section)
{
    return cpd_set[domain][CPD_BITFIELD_POS(section)] &
            (1 << CPD_BITFIELD_OFFSET(section));
}

INLINE void arm_fass_t::activate_domain(space_t *space)
{
    /* Don't need to switch domains as we're still inside the kernel if
     * !space.
     */
    if (EXPECT_FALSE(space == NULL)) {
        current_domain = KERNEL_DOMAIN;
        return;
    }

    arm_domain_t target = space->get_domain();

    if (EXPECT_FALSE(target == INVALID_DOMAIN)) 
        target = domain_recycle(space);

    SET_BIT_WORD(domain_dirty, target);
    current_domain = target;
    current_pid = space->get_pid();
}

#endif

#endif /* __ARCH__ARM__FASS_INLINE_H__ */
