/*********************************************************************
 *                
 * Copyright (C) 2002-2005, 2007-2008, 2010,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/x32/hvm-config.h
 * Description:   configuration of IA32 architecture
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
 * $Id: config.h,v 1.47 2006/06/07 15:34:09 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_X86__X32__HVM_CONFIG_H__
#define __GLUE__V4_X86__X32__HVM_CONFIG_H__
#include INC_ARCH(x86.h)
#include INC_GLUE_SA(offsets.h)


#define VALID_THREADNO_BITS	(16)
#define VALID_THREADNO_SHIFT	(L4_GLOBAL_VERSION_BITS - KTCB_BITS)
#define TOTAL_KTCBS		(__UL(1) << VALID_THREADNO_BITS)
#define VALID_THREADNO_MASK	(TOTAL_KTCBS - 1)


/**********************************************************************
 *                  Virtual Address Space Layout
 **********************************************************************/

/* user area */
#define USER_AREA_START		__UL(0x00000000)
#define USER_AREA_END		__UL(0xE0000000)
#define USER_AREA_SIZE		(USER_AREA_END - USER_AREA_START)

/* small space area */
#define SMALLSPACE_AREA_START	(USER_AREA_END)
#define SMALLSPACE_AREA_SIZE	(0 * X86_X32_PDIR_SIZE)
#define SMALLSPACE_AREA_END	(SMALLSPACE_AREA_START + SMALLSPACE_AREA_SIZE)

/* copy areas, 8MB each */
#define COPY_AREA_COUNT		1
#define COPY_AREA_PDIRS		1
#define COPY_AREA_SIZE		(2 * X86_X32_PDIR_SIZE)
#define COPY_AREA_START		(SMALLSPACE_AREA_END)
#define COPY_AREA_END		(COPY_AREA_START + (COPY_AREA_COUNT * COPY_AREA_SIZE))

/* readmem_phys remap area */
#define MEMREAD_AREA_START	(COPY_AREA_END)
#define MEMREAD_AREA_SIZE	(2 * X86_X32_PDIR_SIZE)
#define MEMREAD_AREA_END	MEMREAD_AREA_START + MEMREAD_AREA_SIZE
#define SPACE_BACKLINK		(MEMREAD_AREA_END)

/* V4 UTCB addressed via %gs:0 */
#define UTCB_MAPPING		__UL(0xE6000000)

/* trampoline to set up segment registers after sysexit */
#define UTRAMP_MAPPING		(UTCB_MAPPING + X86_PAGE_SIZE)

#if !defined(CONFIG_MAX_IOAPICS)
#define CONFIG_MAX_IOAPICS	0
#endif

/* device memory */
#define APIC_MAPPINGS_START	(UTRAMP_MAPPING + X86_PAGE_SIZE)
#define IOAPIC_MAPPING(x)	(APIC_MAPPINGS_START + ((x)+1)*X86_PAGE_SIZE)
#define APIC_MAPPINGS_END	(APIC_MAPPINGS_START + (CONFIG_MAX_IOAPICS + 1) * X86_PAGE_SIZE)
#define VIDEO_MAPPING		(0xb8000)


/* kernel level thread control blocks */
#define KTCB_AREA_START		(0xE8000000)
#define KTCB_AREA_SIZE		(1 << (KTCB_BITS + VALID_THREADNO_BITS))
#define KTCB_AREA_END		(KTCB_AREA_START + KTCB_AREA_SIZE)

/* KERNEL_AREA
 * synched on AS creation, so include all necessary regions!!! */
#define KERNEL_AREA_START	(UTCB_MAPPING)
#define KERNEL_AREA_END		(0xFF000000UL)
#define KERNEL_AREA_SIZE	(KERNEL_AREA_END - KERNEL_AREA_START)



#endif /* !__GLUE__V4_X86__X32__HVM_CONFIG_H__ */
