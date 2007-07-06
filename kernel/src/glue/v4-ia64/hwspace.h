/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     glue/v4-ia64/hwspace.h
 * Description:   Conversion functions for hardware space addresses
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
 * $Id: hwspace.h,v 1.6 2003/09/24 19:04:37 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_IA64__HWSPACE_H__
#define __GLUE__V4_IA64__HWSPACE_H__

#include INC_ARCH(rr.h)


/**
 * Convert an address of any type from virtual to physical.
 * @param x	virtual address
 * @return phsyical address
 */
#define virt_to_phys(x) ((typeof (x)) (IA64_RR_MASK (x) - \
	                               CONFIG_IA64_PHYSMEM_OFFSET))


/**
 * Convert an address of any type from physical to virtual.
 * @param x	physical address
 * @return virtual address
 */
#define phys_to_virt(x) ((typeof (x)) (IA64_RR_BASE (7) + IA64_RR_MASK (x) \
				       + CONFIG_IA64_PHYSMEM_OFFSET))


/**
 * Convert an address of any type from physical to virtual (uncacheable).
 * @param x	physical address
 * @return virtual address in uncacheable address range
 */
#define phys_to_virt_uc(x) ((typeof (x)) (IA64_RR_BASE (6) + IA64_RR_MASK (x) \
				          + CONFIG_IA64_PHYSMEM_OFFSET))


/*
 * Functions for switching physical/vitual mode.
 */
extern "C" void ia64_switch_to_phys (void);
extern "C" void ia64_switch_to_virt (void);


#define RID_PHYS_MEM		(1)
#define RID_PHYS_MEM_UC		(2)
#define RID_TCB			(3)


#endif /* !__GLUE__V4_IA64__HWSPACE_H__ */
