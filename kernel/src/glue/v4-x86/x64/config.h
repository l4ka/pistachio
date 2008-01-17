/*********************************************************************
 *                
 * Copyright (C) 2002-2008,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/x64/config.h
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
 * $Id: config.h,v 1.19 2006/10/20 18:29:12 reichelt Exp $
 *                
 ********************************************************************/
#ifndef __GLUE_V4_X86__X64__CONFIG_H__
#define __GLUE_V4_X86__X64__CONFIG_H__

#include INC_ARCH(x86.h)
#include INC_GLUE_SA(offsets.h)

/**
   attributes for system call functions
   @param x is the name of the system call lacking the leading sys_ .
   This makes it possible to place every system call in its own section
   if required. Default is empty.
 */
#define SYSCALL_ATTR(sec_name)


/*
 * endianness and word size
 */
#define KIP_API_FLAGS		{SHUFFLE2(endian:0, word_size:1)} // 64-bit, little endian 
#define KIP_API_FLAGS_32	{SHUFFLE2(endian:0, word_size:0)} // 32-bit, little endian 

/**
 * minimum size of UTCB area and number of UTCBs in this area
 * 1024 Bytes aligned, 1024 Bytes size, 4K area size 
 */ 
#define KIP_UTCB_INFO		{SHUFFLE3(multiplier:1, alignment:10, size:12)} 

/**
 * size of kernel interface page
 */
#define KIP_KIP_AREA		{size:12}   // 4KB

/** 
 * Minimum number of memory descriptors required in KIP 
 * */
#define KIP_MIN_MEMDESCS        (32)


/**
 * supported page sizes and access bits
 * rwx / 4KB, 2MB
 */
#define KIP_ARCH_PAGEINFO	{SHUFFLE2(rwx:7, size_mask:( ((1 << 12) | (1 << 21)) >> 10)) }


/**
 * address of syscall(x)
 * we use absolute addresses
 */
#define KIP_SYSCALL(x)		((word_t) (x))
#define KIP_SYSCALL_32(x)	((u8_t *) (x##_32) - (u8_t *) &kip_32)

/**
 * Size of a kernel TCB in bytes
 */
#define KTCB_BITS		(12)
#define KTCB_SIZE		(__UL(1) << KTCB_BITS)
#define KTCB_MASK		(~(KTCB_SIZE-1))

/**
 * Valid Part of the Global Thread ID 
 */
#define VALID_THREADNO_BITS	(18)
#define VALID_THREADNO_SHIFT	(L4_GLOBAL_VERSION_BITS - KTCB_BITS)
#define VALID_THREADNO_MASK	((__UL(1) << VALID_THREADNO_BITS) - 1)


/**
 * 
 * On change, adapt offsets.h!
 * pml4[511] pdp[511] 
 */

#define KERNEL_AREA_END		(~0UL)
#define APIC_MAPPINGS_START	(X86_X64_SIGN_EXTENSION | __UL(511) << X86_X64_PML4_BITS | __UL(511) << X86_X64_PDP_BITS | __UL(510) << X86_X64_PDIR_BITS)
#define UTCB_MAPPING		(X86_X64_SIGN_EXTENSION | __UL(511) << X86_X64_PML4_BITS | __UL(511) << X86_X64_PDP_BITS | __UL(509) << X86_X64_PDIR_BITS)
/*      KERNEL_OFFSET           (X86_X64_SIGN_EXTENSION | __UL(511) << X86_X64_PML4_BITS | __UL(511) << X86_X64_PDP_BITS) */

#define VIDEO_MAPPING		(0xB8000)
#define IOAPIC_MAPPING(x)	(APIC_MAPPINGS_START + ((x)+1)*X86_PAGE_SIZE)
#define APIC_MAPPINGS_END	(APIC_MAPPINGS_START + (CONFIG_MAX_IOAPICS + 1) * X86_PAGE_SIZE)

#define UTCB_MAPPING_32		((u32_t) UTCB_MAPPING)

#define CPULOCAL_PML4		511
#define CPULOCAL_PDP		511


/**
 * 
 * We remap the lower 4GB here
 *  
 * pml[511] pdp[510]  ... pml[511] pdp[507] 
 * 
 */
#define REMAP_32BIT_END		(X86_X64_SIGN_EXTENSION | __UL(511) << X86_X64_PML4_BITS | __UL(511) << X86_X64_PDP_BITS)
#define REMAP_32BIT_START	(X86_X64_SIGN_EXTENSION | __UL(511) << X86_X64_PML4_BITS | __UL(507) << X86_X64_PDP_BITS)		 
#define REMAP_32BIT_SIZE	(REMAP_32BIT_END - REMAP_32BIT_START)		 

/**
 * KTCB Area 
 * 
 * pml[511] pdp[506]
 * 
 * 
 */
#define KTCB_AREA_SIZE		(KTCB_SIZE << VALID_THREADNO_BITS)
#define KTCB_AREA_START		((REMAP_32BIT_START - KTCB_AREA_SIZE) & (X86_X64_SIGN_EXTEND_MASK | X86_X64_PML4_MASK | X86_X64_PDP_MASK) )
#define KTCB_AREA_END		(KTCB_AREA_START + KTCB_AREA_SIZE)



/**
 * Copy Area
 * 
 *   
 * pml[511] pdp[2 * count !<= 505]  ... pml[511] pdp[0] 
 * thus count must be less than 506/2 = 253
 */

#define COPY_AREA_COUNT		1
#define COPY_AREA_PDIRS		2
#define COPY_AREA_SIZE		(__UL(2) * X86_X64_PDP_SIZE)
#define COPY_AREA_START		(X86_X64_SIGN_EXTENSION | __UL(511) << X86_X64_PML4_BITS)
#define COPY_AREA_END		(COPY_AREA_START + (COPY_AREA_COUNT * COPY_AREA_SIZE))

#define SPACE_BACKLINK		(X86_X64_SIGN_EXTENSION | __UL(510) << X86_X64_PML4_BITS)
#define KERNEL_AREA_START	(SPACE_BACKLINK)
#define KERNEL_AREA_SIZE	(KERNEL_AREA_END - KERNEL_AREA_START + 1)

/**
 * User Area
 *  
 * pml[510/511] .. pml[0] (without sign extension)
 * 
 * 
 */

#define USER_AREA_END		(KERNEL_AREA_START)
#define USER_AREA_START		(__UL(0))				
#define USER_AREA_SIZE		(USER_AREA_END - USER_AREA_START)




/**
 * Base address of the root task's UTCB area
 */
#define ROOT_UTCB_START		(0xBF000000)

/**
 * Address of the KIP in the root task
 */
#define ROOT_KIP_START		(0xBFF00000)

/* defines the granularity kernel code and data is mapped */
/* defines the granularity kernel code and data is mapped */
#if !defined(CONFIG_X86_IO_FLEXPAGES) && defined (CONFIG_X86_PSE) 
#define KERNEL_PAGE_SIZE	(X86_SUPERPAGE_SIZE)
#else
#define KERNEL_PAGE_SIZE	(X86_PAGE_SIZE)
#endif

#define VIDEO_AREA


/* Segment register values */

/* DS, ES, SS is ignored in 64bit mode, so we don't need to have 
 * segment selectors/descriptors for them
 * 
 * KCS and KCS+8 = KDS will be loaded on syscall
 * UCS and UCS+8 = UDS will be loaded on sysret
 * 
 * TSS descriptor is 16 bytes long, therefore we place it at the end
 */
#define X86_X64_INVS	         0x0		/* 0		*/
#define X86_KCS                  0x8		/* 1, RPL = 0	*/
#define X86_KDS                  0x10		/* 2, RPL = 0	*/
#define X86_UCS32                0x1b		/* 3, RPL = 3	*/
#define X86_UDS                  0x23		/* 4, RPL = 3	*/
#define X86_UCS                  0x2b		/* 5, RPL = 3	*/
#define X86_UTCBS                0x33		/* 6, RPL = 3	*/
#define X86_X64_KDBS               0x38		/* 7, RPL = 0	*/
#define X86_TBS                  0x43		/* 8, RPL = 0	*/
#define X86_TSS			 0x48		/* 9, RPL = 0	*/

/* user mode e-flags   */
#if defined(CONFIG_X86_PVI)
#define X86_USER_FLAGS      (X86_FLAGS_IOPL(0) | X86_FLAGS_IF | X86_FLAGS_VIF | 2)
#define X86_USER_FLAGMASK   (X86_FLAGS_CF | X86_FLAGS_PF | X86_FLAGS_AF | X86_FLAGS_ZF | X86_FLAGS_SF | X86_FLAGS_OF| X86_FLAGS_VIF | X86_FLAGS_VIP)
#elif defined(CONFIG_X86_IO_FLEXPAGES)
#define X86_USER_FLAGS      (X86_FLAGS_IOPL(0) | X86_FLAGS_IF | 2)
#define X86_USER_FLAGMASK   (X86_FLAGS_CF | X86_FLAGS_PF | X86_FLAGS_AF | X86_FLAGS_ZF | X86_FLAGS_SF | X86_FLAGS_OF)
#else
#define X86_USER_FLAGS      (X86_FLAGS_IOPL(3) | X86_FLAGS_IF | 2)
#define X86_USER_FLAGMASK   (X86_FLAGS_CF | X86_FLAGS_PF | X86_FLAGS_AF | X86_FLAGS_ZF | X86_FLAGS_SF | X86_FLAGS_OF)
#endif

/* IDT, GDT, etc. */
#define IDT_SIZE		256		/* 256 Ent.		*/
#define GDT_SIZE		11		/* 10  Ent. (2 for TSS)	*/
#define GDT_IDX(x)		((x) >> 3)

/* Syscall/Sysret code and stack segment
 * 
 * On syscall : 
 *		CS <- X86_X64_SYSCALLCS
 *		SS <- X86_X64_SYSCALLCS + 8
 * 
 * On sysret into compatibility mode:  
 *		CS <- X86_X64_SYSRETCS
 *		SS <- X86_X64_SYSRETCS + 8
 * On sysret into long mode:  
 *		CS <- X86_X64_SYSRETCS + 16
 *		SS <- X86_X64_SYSRETCS + 8
 */

#define X86_SYSCALLCS		((u64_t) X86_KCS)
#define X86_SYSRETCS		((u64_t) X86_UCS32)

/* Syscall/Sysret RFLAGS MASK register value */
#define X86_X64_SYSCALL_FLAGMASK	(X86_FLAGS_IF | X86_FLAGS_RF | X86_FLAGS_VM)

/* global IDT entries */
#define IDT_LAPIC_TIMER         0x40
#define IDT_LAPIC_THERMAL       0x41
#define IDT_LAPIC_XCPU_IPI	0x42
#define IDT_LAPIC_ERROR		0x43
#define IDT_IOAPIC_BASE         0x44
#define IDT_IOAPIC_MAX		0xf0
#define IDT_IOAPIC_SPURIOUS	0xfb

/* Page size for APIC and ACPI mappings */
#define APIC_PGENTSZ	        pgent_t::size_4k
#define ACPI_PGENTSZ	        pgent_t::size_2m

/* spurious int must have lowermost 4 bits set */
#define IDT_LAPIC_SPURIOUS_INT  0x3f


/* Exception Settings */
#define EXC_INTERRUPT(name)	X86_EXCNO_ERRORCODE(name, 0)
#define NUM_EXC_REGS		20	 
    

/* 1.953ms per timer tick
 * VU: the hardware clock can only be configured to tick in 2^n Hz
 * 1000 / 512 Hz = 1.953125 ms/tick */
#define TIMER_TICK_LENGTH       (1953)

/*
 * stack layout (counted in words)
 * 
 * SS
 * RSP
 * RFLAGS
 * CS
 * RIP
 * ErrorCode
 * ReturnFromIPC
 */
#define KSTACK_USP              (-2)
#define KSTACK_UFLAGS           (-3)
#define KSTACK_CS               (-4)
#define KSTACK_UIP              (-5)
#define KSTACK_RET_IPC          (-8)

#define CACHE_LINE_SIZE		(X86_X64_CACHE_LINE_SIZE)
#define SMP_STARTUP_ADDRESS	(0x4000) 

#define CONFIG_SMP_SYNC_REQUEST

#if defined(BUILD_TCB_LAYOUT)
#undef CONFIG_X86_COMPATIBILITY_MODE
#endif

#endif /* !__GLUE_V4_X86__X64__CONFIG_H__ */
