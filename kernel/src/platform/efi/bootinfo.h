/*********************************************************************
 *                
 * Copyright (C) 2002,  Karlsruhe University
 *                
 * File path:     platform/efi/bootinfo.h
 * Description:   Bootinfo structure passed on from bootloader.
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
 * $Id: bootinfo.h,v 1.3 2003/09/24 19:04:55 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __PLATFORM__EFI__BOOTINFO_H__
#define __PLATFORM__EFI__BOOTINFO_H__

#define EFI_BOOTINFO_VERSION	(1)

class efi_bootinfo_t
{
public:
    union {
	char	string[8];		// "L4Ka ldr"
	u64_t	number;
    } magic;
    u64_t	version;		// Version 1
	
    u64_t	systab;			// EFI system table
    u64_t	memmap;			// EFI memory map (phys addr)
    u64_t	memmap_size;		// Size of EFI memory map
    u64_t	memdesc_size;		// Size of EFI memory desc
    u32_t	memdesc_version;	// Version of EFI memory desc

    bool is_valid (void)
	{
	    if (magic.string[0] != 'L' || magic.string[1] != '4' ||
		magic.string[2] != 'K' || magic.string[3] != 'a' ||
		magic.string[4] != ' ' || magic.string[5] != 'l' ||
		magic.string[6] != 'd' || magic.string[7] != 'r')
		return false;
	    if (version != 1)
		return false;
	    return true;
	}
};


#endif /* !__PLATFORM__EFI__BOOTINFO_H__ */
