/****************************************************************************
 *
 * Copyright (C) 2002-2003, Karlsruhe University
 *
 * File path:	include/piggybacker/piggyback.h
 * Description:	
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
 * $Id: piggyback.h,v 1.2 2003/09/24 19:06:37 skoglund Exp $
 *
 ***************************************************************************/

#ifndef __PIGGYBACKER__INCLUDE__PIGGYBACK_H__
#define __PIGGYBACKER__INCLUDE__PIGGYBACK_H__

#include <l4/types.h>

#define SYMBOL(a)	extern L4_Word_t a []

SYMBOL(_binary_sigma0_mod_start);
SYMBOL(_binary_sigma0_mod_end);
SYMBOL(_binary_sigma0_mod_size);

SYMBOL(_binary_root_task_mod_start);
SYMBOL(_binary_root_task_mod_end);
SYMBOL(_binary_root_task_mod_size);

SYMBOL(_binary_kernel_mod_start);
SYMBOL(_binary_kernel_mod_end);
SYMBOL(_binary_kernel_mod_size);

L4_INLINE L4_Word_t get_sigma0_start( void )
    { return (L4_Word_t) _binary_sigma0_mod_start; }

L4_INLINE L4_Word_t get_sigma0_end( void )
    { return (L4_Word_t) _binary_sigma0_mod_end; }

L4_INLINE L4_Word_t get_sigma0_size( void )
    { return (L4_Word_t) _binary_sigma0_mod_size; }

L4_INLINE L4_Word_t get_root_task_start( void )
    { return (L4_Word_t) _binary_root_task_mod_start; }

L4_INLINE L4_Word_t get_root_task_end( void )
    { return (L4_Word_t) _binary_root_task_mod_end; }

L4_INLINE L4_Word_t get_root_task_size( void )
    { return (L4_Word_t) _binary_root_task_mod_size; }

L4_INLINE L4_Word_t get_kernel_start( void )
    { return (L4_Word_t) _binary_kernel_mod_start; }

L4_INLINE L4_Word_t get_kernel_end( void )
    { return (L4_Word_t) _binary_kernel_mod_end; }

L4_INLINE L4_Word_t get_kernel_size( void )
    { return (L4_Word_t) _binary_kernel_mod_size; }

#endif	/* __PIGGYBACKER__INCLUDE__PIGGYBACK_H__ */
