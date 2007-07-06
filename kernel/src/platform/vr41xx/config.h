/*********************************************************************
 *                
 * Copyright (C) 2003,  National ICT Australia (NICTA)
 *                
 * File path:     platform/vr41xx/config.h
 * Description:   Platform specific configuration
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

#ifndef __PLATFORM__VR41XX__CONFIG_H__
#define __PLATFORM__VR41XX__CONFIG_H__


#define CONFIG_MIPS64_ADDRESS_BITS	    40
#define CONFIG_MIPS64_PHYS_ADDRESS_BITS	    32
#define CONFIG_MIPS64_VPN_SHIFT		    10
#define CONFIG_MIPS64_PAGEMASK_4K	    0x1800

#define CONFIG_MIPS64_CONSOLE_RESERVE	    (0)

#define CONFIG_MIPS64_TLB_SIZE 32

#define CONFIG_MIPS64_STATUS_MASK 0x0fffff00

#if !defined(__ASSEMBLER__)

#else /* ASSEMBLER */

/* 64-bit virtual memory kernel mode */
/* 64-bit virtual memory supervisor mode */
/* 64-bit virtual memory user mode */
#define	INIT_CP0_STATUS_SET			\
    (ST_KX|ST_SX|ST_UX)

/* not used here: disable reverse endian */
/* go into kernel mode */
/* remove error condition */
/* remove exception level */
/* clear NMI/soft reset */
#define	INIT_CP0_STATUS_CLEAR			\
    (ST_KSU|ST_ERL|ST_EXL|ST_SR)

#endif

#endif /* __PLATFORM__VR41XX__CONFIG_H__ */
