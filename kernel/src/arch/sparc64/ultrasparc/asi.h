/*********************************************************************
 *                
 * Copyright (C) 2003-2004, University of New South Wales
 *                
 * File path:   arch/sparc64/ultrasparc/asi.h
 * Description: UltraSPARC specific alternative space identifiers
 *              (ASIs).
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
 * $Id: asi.h,v 1.4 2004/01/21 23:55:51 philipd Exp $
 *                
 ********************************************************************/

#ifndef __ARCH__SPARC64__ULTRASPARC__ASI_H__
#define __ARCH__SPARC64__ULTRASPARC__ASI_H__

/* E-bit means accesses have side effects */

#define ASI_PHYS_USE_EC        0x14 /* Physical, ext cachable                */
#define ASI_PHYS_BYPASS_EC_E   0x15 /* Physical, E-bit                       */
#define ASI_PHYS_USE_UE_L      0x1C /* Physical, ext cacheble, little endian */
#define ASI_PHYS_BYPASS_EC_E_L 0x1D /* Physical, E-bit, little endian        */

#define ASI_LSU_CONTROL_REG    0x45 /* Load/Store unit control register.     */

#define ASI_NUCLEUS_QUAD_LDD   0x24 /* Atomic 128-bit load from nucleus      */

/***********
* MMU ASIs *
***********/

#define ASI_IMMU                0x50 /* Instruction-MMU main register space. */
#define ASI_IMMU_TSB_8KB_PTR    0x51 /* I-MMU TSB 8KB pointer register.      */
#define ASI_IMMU_TSB_64KB_PTR   0x52 /* I-MMU TSB 64KB pointer register.     */
#define ASI_ITLB_DATA_IN        0x54 /* I-MMU TLB data in register.          */
#define ASI_ITLB_DATA_ACCESS    0x55 /* I-MMU TLB data access register.      */
#define ASI_ITLB_TAG_READ       0x56 /* I-MMU TLB tag read register.         */
#define ASI_IMMU_DEMAP          0x57 /* I-MMU TLB demap.                     */

#define ASI_DMMU                0x58 /* Data-MMU main register space.        */
#define ASI_DMMU_TSB_8KB_PTR    0x59 /* D-MMU TSB 8KB pointer register.      */
#define ASI_DMMU_TSB_64KB_PTR   0x5A /* D-MMU TSB 64KB pointer register.     */
#define ASI_DMMU_TSB_DIRECT_PTR 0x5B /* D-MMU TSB direct pointer register.   */
#define ASI_DTLB_DATA_IN        0x5C /* D-MMU TLB data in register.          */
#define ASI_DTLB_DATA_ACCESS    0x5D /* D-MMU TLB data access register.      */
#define ASI_DTLB_TAG_READ       0x5E /* D-MMU TLB tag read register.         */
#define ASI_DMMU_DEMAP          0x5F /* D-MMU TLB demap.                     */

/**
 *  I/D-MMU main register space.
 */

// Accessable via both ASI_IMMU and ASI_DMMU.

#define TSB_TAG_TARGET    0x00 /* I/D-MMU Tag Target register.               */
#define TLB_SFSR          0x18 /* I/D-MMU Synchronous fault status regiser.  */
#define TSB_REGISTER      0x28 /* I/D-MMU TSB register.                      */
#define TLB_TAG_ACCESS    0x30 /* I/D-MMU TLB tag access register.           */

// Accessable via ASI_DMMU only.

#define TLB_SFAR          0x20 /* D-MMU Synchronous fault address register.  */
#define VADDR_WATCHPOINT  0x38 /* D-MMU VA data watchpoint register.         */
#define PADDR_WATCHPOINT  0x40 /* D-MMU PA data watchpoint register.         */

// Unified, only accessable via ASI_DMMU only.

#define PRIMARY_CONTEXT   0x08 /* I/D-MMU Primary context (ASID) register.   */
#define SECONDARY_CONTEXT 0x10 /* I/D-MMU Secondary context (ASID) register. */


#endif /* !__ARCH__SPARC64__ULTRASPARC__ASI_H__ */
