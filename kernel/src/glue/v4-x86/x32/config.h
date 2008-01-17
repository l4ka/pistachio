/*********************************************************************
 *                
 * Copyright (C) 2002-2005, 2007-2008,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/x32/config.h
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


#ifndef __GLUE__V4_X86__X32__CONFIG_H__
#define __GLUE__V4_X86__X32__CONFIG_H__

#include INC_ARCH(x86.h)
#include INC_GLUE_SA(offsets.h)


/**********************************************************************
 *                  Kernel interface page values
 **********************************************************************/

/* 32bit, little endian	*/
#define KIP_API_FLAGS		{SHUFFLE2(endian:0, word_size:0)}
/* 512 Bytes aligned, 512 Bytes size, 4K area size */
#define KIP_UTCB_INFO		{SHUFFLE3(multiplier:1, alignment:9, size:12)}
/* 4KB */
#define KIP_KIP_AREA		{ size:12 }
#if defined(CONFIG_X86_PSE)
/* write+read, 4M+4K */
#define KIP_ARCH_PAGEINFO	{SHUFFLE2(rwx:6, size_mask:((1 << X86_SUPERPAGE_BITS) | (1 << X86_PAGE_BITS)) >> 10)}
#else /* !CONFIG_X86_PSE */
/* write+read, 4K */
#define KIP_ARCH_PAGEINFO	{SHUFFLE2(rwx:6, size_mask:((1 << X86_PAGE_BITS)) >> 10)}
#endif
#define KIP_SYSCALL(x)		((u8_t*)(x) - (u8_t*)&kip)

/* configuration for KTCBs
 * make sure the KTCBs fit entirely into the TCB area
 * TCB area size == 2^(KTCB_BITS + VALID_THREADNO_BITS) */
#define KTCB_BITS		(11)
#define KTCB_SIZE		(1 << KTCB_BITS)
#define KTCB_MASK		(~(KTCB_SIZE-1))

#define VALID_THREADNO_BITS	(17)
#define VALID_THREADNO_SHIFT	(L4_GLOBAL_VERSION_BITS - KTCB_BITS)
#define VALID_THREADNO_MASK	((__UL(1) << VALID_THREADNO_BITS) - 1)


/**********************************************************************
 *                  Virtual Address Space Layout
 **********************************************************************/

/* user area */
#define USER_AREA_START		__UL(0x00000000)
#define USER_AREA_END		__UL(0xC0000000)
#define USER_AREA_SIZE		(USER_AREA_END - USER_AREA_START)

/* small space area */
#define SMALLSPACE_AREA_START	(USER_AREA_END)
#define SMALLSPACE_AREA_SIZE	(16 * X86_X32_PDIR_SIZE)
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
#define UTCB_MAPPING		__UL(0xDF000000)

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
#define KTCB_AREA_START		(0xE0000000)
#define KTCB_AREA_SIZE		(1 << (KTCB_BITS + VALID_THREADNO_BITS))
#define KTCB_AREA_END		(KTCB_AREA_START + KTCB_AREA_SIZE)

/* KERNEL_AREA
 * synched on AS creation, so include all necessary regions!!! */
#define KERNEL_AREA_START	(UTCB_MAPPING)
#define KERNEL_AREA_END		(0xFF000000UL)
#define KERNEL_AREA_SIZE	(KERNEL_AREA_END - KERNEL_AREA_START)

/* address of UTCB and KIP for root servers, at end of user-AS */
#define ROOT_UTCB_START		(0xBF000000)
#define ROOT_KIP_START		(0xBFF00000)

/* startup address for application processors */
#define SMP_STARTUP_ADDRESS	(0x4000)

/* some additional memory for the kernel*/
#define ADDITIONAL_KMEM_SIZE	(0x01000000)

/* defines the granularity kernel code and data is mapped */
#if !defined(CONFIG_SMP) && !defined(CONFIG_X86_IO_FLEXPAGES) && defined (CONFIG_X86_PSE) 
#define KERNEL_PAGE_SIZE	(X86_SUPERPAGE_SIZE)
#else
#define KERNEL_PAGE_SIZE	(X86_PAGE_SIZE)
#endif

/* Cache configuration */
#define CACHE_LINE_SIZE		(X86_X32_CACHE_LINE_SIZE_L2)



/**********************************************************************
 *          Architectural defines (segments, interrupts, etc.)
 **********************************************************************/

/* Segment register values */
#define X86_KCS		0x08
#define X86_KDS		0x10
#define X86_UCS		0x1b
#define X86_UDS		0x23
#define X86_UTCBS      	0x2b
#define X86_TSS		0x30
#define X86_KDB		0x38
#define X86_TBS		0x43

/* user mode e-flags */
#if defined(CONFIG_X86_PVI)
#define X86_USER_FLAGS      (X86_FLAGS_IOPL(0) | X86_FLAGS_IF | X86_FLAGS_VIF | 2)
#define X86_USER_FLAGMASK   (X86_FLAGS_CF | X86_FLAGS_PF | X86_FLAGS_AF | X86_FLAGS_ZF | X86_FLAGS_SF | X86_FLAGS_OF| X86_FLAGS_VIF | X86_FLAGS_VIP)
#elif defined(CONFIG_X86_IO_FLEXPAGES)
#define X86_USER_FLAGS      (X86_FLAGS_IOPL(0) | X86_FLAGS_IF | 2)
#define X86_USER_FLAGMASK  (X86_FLAGS_CF | X86_FLAGS_PF | X86_FLAGS_AF | X86_FLAGS_ZF | X86_FLAGS_SF | X86_FLAGS_OF)
#else
#define X86_USER_FLAGS      (X86_FLAGS_IOPL(3) | X86_FLAGS_IF | 2)
#define X86_USER_FLAGMASK   (X86_FLAGS_CF | X86_FLAGS_PF | X86_FLAGS_AF | X86_FLAGS_ZF | X86_FLAGS_SF | X86_FLAGS_OF)
#endif


/* IDT, GDT, etc. */
#define IDT_SIZE		256
#define GDT_SIZE		16

/* global IDT entries */
#define IDT_LAPIC_SPURIOUS_INT	0x3f /* spurious int vector must 
				      * have lowermost 4 bits set */
#define IDT_LAPIC_TIMER		0x40
#define IDT_LAPIC_THERMAL	0x41
#define IDT_LAPIC_XCPU_IPI	0x42
#define IDT_LAPIC_ERROR		0x43
#define IDT_IOAPIC_SPURIOUS	0xfb
#define IDT_IOAPIC_BASE		0x44
#define IDT_IOAPIC_MAX		0xf0

/* Page size for APIC and ACPI mappings */
#define APIC_PGENTSZ	        pgent_t::size_4k
#define ACPI_PGENTSZ	        pgent_t::size_4m

#define EXC_INTERRUPT(name)	X86_EXCNO_ERRORCODE(name, 0)				
#define NUM_EXC_REGS		13	      

/* timer frequency */
#ifdef CONFIG_IOAPIC
# define TIMER_TICK_LENGTH	(CONFIG_APIC_TIMER_TICK)
#else
/* 1.953ms per timer tick
 * VU: the hardware clock can only be configured to tick in 2^n Hz
 * 1000 / 512 Hz = 1.953125 ms/tick */
# define TIMER_TICK_LENGTH	(1953)
#endif

/* enable synchronous XCPU-requests, for TLB shoot-downs */
#define CONFIG_SMP_SYNC_REQUEST



/**********************************************************************
 *                       kernel stack layout 
 **********************************************************************/

#define KSTACK_USP		(-2)
#define KSTACK_UFLAGS		(-3)
#define KSTACK_UIP		(-5)
#define KSTACK_RET_IPC		(-7)


#endif /* !__GLUE__V4_X86__X32__CONFIG_H__ */
