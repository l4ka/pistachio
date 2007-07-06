/****************************************************************************
 *
 * Copyright (C) 2002, Karlsruhe University
 *
 * File path:	glue/v4-powerpc/bat.h
 * Description:	Describes the kernel's bat register allocation.
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
 * $Id: bat.h,v 1.8 2003/09/24 19:04:51 skoglund Exp $
 *
 ***************************************************************************/

#ifndef __GLUE__V4_POWERPC__BAT_H__
#define __GLUE__V4_POWERPC__BAT_H__

/* TODO: use the unallocated BAT registers to map large, aligned regions of 
 * memory into the user address space.  This will be most useful for device 
 * drivers that occupy large regions of the address space.  It can help avoid 
 * numerous tlb misses when interacting with devices.  Since each device
 * driver will typically use a dedicated address space, each device driver
 * can make use of a BAT register (as opposed to a monolithic kernel such
 * as linux, which never changes its BAT mappings).
 *
 * If the user app attempts to execute code mapped by a dBAT register,
 * it will see a page fault.  We could then lazily install an iBAT.
 *
 * We can use the tcb resource-mask to track usage of the BAT registers.
 * The IPC fast path should deal with the BAT registers, since an app will
 * use the BAT registers as a performance enhancement.
 *
 * Problems: the BAT registers do not support referenced and changed bits.
 * They must be carefully used.  If someone inspects the referenced and/or
 * changed bits, assume the worst and return true.
 * Perhaps we can limit BAT mappings to requests beyond physical memory,
 * and thus only devices.
 *
 * We should probably only enable this option if a user app enables a bit (like
 * the small-address space enhancement).
 */

/* TODO: for SMP, use the unallocated dBAT to map per-cpu data regions.
 * The kernel needn't claim the full 128K per BAT.  The kernel just must
 * ensure that if it uses the page table to map the remaining pages from the
 * 128K (ex: TCB's), that it doesn't overlap with the virtual address region 
 * of the dBAT.
 */

#define CODE_BAT_PAGE_SIZE      BAT_256M_PAGE_SIZE
#define CODE_BAT_PAGE_MASK      BAT_256M_PAGE_MASK
#define CODE_BAT_BL             BAT_BL_256M

#define DATA_BAT_PAGE_SIZE      BAT_256M_PAGE_SIZE
#define DATA_BAT_PAGE_MASK      BAT_256M_PAGE_MASK
#define DATA_BAT_BL             BAT_BL_256M

#if defined(ASSEMBLY)

#define DBAT_UPPER_KERNEL	536
#define DBAT_LOWER_KERNEL	537
#define DBAT_UPPER_PGHASH	538
#define DBAT_LOWER_PGHASH	539
#define DBAT_UPPER_OPIC		540
#define DBAT_LOWER_OPIC		541
#define DBAT_UPPER_CPU		542
#define DBAT_LOWER_CPU		543

#define IBAT_UPPER_KERNEL	528
#define IBAT_LOWER_KERNEL	529
#define IBAT_UPPER_EXCEPT	530
#define IBAT_LOWER_EXCEPT	531
#define IBAT_UPPER_UNUSED0	532
#define IBAT_LOWER_UNUSED0	533
#define IBAT_UPPER_UNUSED1	534
#define IBAT_LOWER_UNUSED1	535

#else

#define ppc_set_kernel_dbat( which, bat )	ppc_set_dbat0##which( (bat) )
#define ppc_set_pghash_dbat( which, bat)	ppc_set_dbat1##which( (bat) )
#define ppc_set_opic_dbat( which, bat)		ppc_set_dbat2##which( (bat) )
#define ppc_set_cpu_dbat( which, bat)		ppc_set_dbat3##which( (bat) )

#define ppc_get_kernel_dbat( which )		ppc_get_dbat0##which()
#define ppc_get_pghash_dbat( which )		ppc_get_dbat1##which()
#define ppc_get_opic_dbat( which )		ppc_get_dbat2##which()
#define ppc_get_cpu_dbat( which )		ppc_get_dbat3##which()

#define ppc_set_kernel_ibat( which, bat )	ppc_set_ibat0##which( (bat) )
#define ppc_set_except_ibat( which, bat )	ppc_set_ibat1##which( (bat) )
#define ppc_set_unused0_ibat( which, bat )	ppc_set_ibat2##which( (bat) )
#define ppc_set_unused1_ibat( which, bat )	ppc_set_ibat3##which( (bat) )

#define ppc_get_kernel_ibat( which )		ppc_get_ibat0##which()
#define ppc_get_except_ibat( which )		ppc_get_ibat1##which()

#endif

#endif	/* __GLUE__V4_POWERPC__BAT_H__ */

