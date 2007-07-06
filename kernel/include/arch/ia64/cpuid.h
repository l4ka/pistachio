/*********************************************************************
 *                
 * Copyright (C) 2002,  Karlsruhe University
 *                
 * File path:     arch/ia64/cpuid.h
 * Description:   IA-64 CPUID access
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
 * $Id: cpuid.h,v 1.2 2003/09/24 19:04:28 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__IA64__CPUID_H__
#define __ARCH__IA64__CPUID_H__

// Can not be declared static inside the inline function
static union {
    char	string[12];
    u64_t	raw[3];
} __cpuid_vendor_info;


INLINE u64_t cpuid_get (word_t num)
{
    u64_t reg;
    __asm__ ("mov %0 = cpuid[%1]" :"=r" (reg) :"r" (num));
    return reg;
}

INLINE char * cpuid_vendor_info (void)
{
    __cpuid_vendor_info.raw[0] = cpuid_get (0);
    __cpuid_vendor_info.raw[1] = cpuid_get (1);
    __cpuid_vendor_info.string[16] = '\0';

    return __cpuid_vendor_info.string;
}

INLINE u64_t cpuid_serial_number (void)
{
    return cpuid_get (2);
}

class cpuid_version_info_t
{
public:
    union {
	struct {
	    word_t number	: 8;
	    word_t revision	: 8;
	    word_t model	: 8;
	    word_t family	: 8;
	    word_t archrev	: 8;
	};
	u64_t raw;
    };
};

INLINE cpuid_version_info_t cpuid_version_info (void)
{
    cpuid_version_info_t version;
    version.raw = cpuid_get (3);
    return version;
}



#endif /* !__ARCH__IA64__CPUID_H__ */
