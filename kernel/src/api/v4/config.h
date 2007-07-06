/*********************************************************************
 *                
 * Copyright (C) 2002-2006,  Karlsruhe University
 *                
 * File path:     api/v4/config.h
 * Description:   V4 specific configurations
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
 * $Id: config.h,v 1.15 2006/10/18 11:14:39 reichelt Exp $
 *                
 ********************************************************************/
#ifndef __API__V4__CONFIG_H__
#define __API__V4__CONFIG_H__

/*
 * selection of architecture-dependent constants and types
 */
#if defined(CONFIG_IS_32BIT)
# define L4_ARCH_CONST(const)		const##_32
#elif defined(CONFIG_IS_64BIT)
# define L4_ARCH_CONST(const)		const##_64
#else
# error undefined architecture width (32/64bit?)
#endif

#define L4_GLOBAL_THREADNO_BITS_32	18
#define L4_GLOBAL_INTRNO_BITS_32	18
#define L4_GLOBAL_VERSION_BITS_32	14
#define L4_LOCAL_ID_BITS_32		26
#define L4_LOCAL_ID_ZERO_BITS_32	6
#define L4_FPAGE_BASE_BITS_32		22

#define L4_GLOBAL_THREADNO_BITS_64	32
#define L4_GLOBAL_INTRNO_BITS_64	32
#define L4_GLOBAL_VERSION_BITS_64	32
#define L4_LOCAL_ID_BITS_64		58
#define L4_LOCAL_ID_ZERO_BITS_64	6
#define L4_FPAGE_BASE_BITS_64		54

#define L4_GLOBAL_THREADNO_BITS		L4_ARCH_CONST(L4_GLOBAL_THREADNO_BITS)
#define L4_GLOBAL_INTRNO_BITS		L4_ARCH_CONST(L4_GLOBAL_INTRNO_BITS)
#define L4_GLOBAL_VERSION_BITS		L4_ARCH_CONST(L4_GLOBAL_VERSION_BITS)
#define L4_LOCAL_ID_BITS		L4_ARCH_CONST(L4_LOCAL_ID_BITS)
#define L4_LOCAL_ID_ZERO_BITS		L4_ARCH_CONST(L4_LOCAL_ID_ZERO_BITS)
#define L4_FPAGE_BASE_BITS		L4_ARCH_CONST(L4_FPAGE_BASE_BITS)

#define DEFAULT_TIMESLICE_LENGTH	(time_t::period(625, 4))
#define DEFAULT_TOTAL_QUANTUM		(time_t::never())
#define MAX_PRIO			255
#define ROOT_PRIORITY			MAX_PRIO
#define DEFAULT_PRIORITY		100

/*
 * root server configuration
 */
#define ROOT_MAX_THREADS		256
#define ROOT_VERSION			1

/**
 * number of message registers
 */
#define IPC_NUM_MR			64
#define IPC_NUM_BR			33

#endif /* !__API__V4__CONFIG_H__ */
