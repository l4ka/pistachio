/*********************************************************************
 *                
 * Copyright (C) 2003,  Karlsruhe University
 *                
 * File path:     l4/ia64/arch.h
 * Description:   Architecture specific functionality
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
 * $Id: arch.h,v 1.2 2003/09/24 19:06:24 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __L4__IA64__ARCH_H__
#define __L4__IA64__ARCH_H__


/*
 * SAL Access
 */

#define __L4_SAL_PCI_CONFIG_READ	(0x01000010)
#define __L4_SAL_PCI_CONFIG_WRITE	(0x01000011)

L4_INLINE L4_Word_t L4_SAL_PCI_ConfigRead (L4_Word_t address, L4_Word_t size,
					   L4_Word_t *value)
{
    L4_Word_t dummy;
    return L4_SAL_Call (__L4_SAL_PCI_CONFIG_READ, 
			address, size, 0, 0, 0, 0,
			value, &dummy, &dummy);
}

L4_INLINE L4_Word_t L4_SAL_PCI_ConfigWrite (L4_Word_t address, L4_Word_t size,
					    L4_Word_t value)
{
    L4_Word_t dummy;
    return L4_SAL_Call (__L4_SAL_PCI_CONFIG_WRITE,
			address, size, value, 0, 0, 0,
			&dummy, &dummy, &dummy);
}


#endif /* !__L4__IA64__ARCH_H__ */
