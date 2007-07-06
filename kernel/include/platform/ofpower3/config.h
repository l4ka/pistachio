/*********************************************************************
 *                
 * Copyright (C) 2003,  National ICT Australia (NICTA)
 *                
 * File path:     platform/ofpower3/config.h
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
 * $Id: config.h,v 1.4 2004/06/04 02:11:05 cvansch Exp $
 *                
 ********************************************************************/

#ifndef __PLATFORM__OFPOWER3__CONFIG_H__
#define __PLATFORM__OFPOWER3__CONFIG_H__

#define CONFIG_POWERPC64_STAB		1
#define POWERPC64_STAB_SIZE		(POWERPC64_PAGE_SIZE)


#define CONFIG_POWERPC64_TLB_SIZE	256
#define CONFIG_POWERPC64_TLB_WAYS	2

/* We use a 64 bit virt address. (64-28 = 36) */
#define CONFIG_POWERPC64_ESID_BITS	36

/* Power3 has 80 bit virtual address. We use a 16-bit ASID tag which reduces
 * user space to 64bits :)
 * The kernel always uses the an ASID value of 0
 */
/*#define POWERPC64_VIRTUAL_BITS	80 */
#define POWERPC64_VIRTUAL_BITS	74	/* Only use 10 bits for ASID, avoid huge asid table */
#define POWERPC64_USER_BITS	64

#endif /* __PLATFORM__OFPOWER3__CONFIG_H__ */
