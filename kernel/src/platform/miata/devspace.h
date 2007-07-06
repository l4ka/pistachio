/*********************************************************************
 *                
 * Copyright (C) 2002-2003,   University of New South Wales
 *                
 * File path:     platform/miata/devspace.h
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
 * $Id: devspace.h,v 1.3 2003/09/24 19:04:56 skoglund Exp $
 *                
 ********************************************************************/

#ifndef __PLATFORM_DEVSPACE_H__
#define __PLATFORM_DEVSPACE_H__

#include INC_ARCH(page.h)

class devspace_t {
 private:
    /* Various PCI spaces */
    enum pci_space {
	PCI_SPARSE_OFFSET = (AS_KSEG_START + 0x8000000000ull),
	PCI_IO_OFFSET = (AS_KSEG_START + 0x8580000000ull),
	PCI_DENSE_OFFSET = (AS_KSEG_START + 0x8600000000ull)
    };

    /* sjw (28/01/2003): Hopefully these will be optimised away */
    
    /* we don't support tri-bytes, so don't ask.  Will break for 64bit */
     __inline__ static word_t mk_cpu_addr(word_t space, word_t pci_addr, int nbytes) {
	word_t size = ((nbytes - 1) & 0x3) << 3;
	return space | (pci_addr << 5) | size;
    }

     __inline__ static u8_t extbl(u32_t data, int offset) {
	u8_t ret;
	__asm__ ("extbl %2,%1,%0" : "=r"(ret) : "rI"(offset), "r"(data));
	return ret;
    }

     __inline__ static u32_t insbl(u8_t data, int offset) {
	u32_t ret;
	__asm__ ("insbl %2,%1,%0" : "=r"(ret) : "rI"(offset), "r"(data));
	return ret;
    }

 public:
    __inline__ static void mb() {
	__asm__ __volatile__("mb": : :"memory");
    }

    /* Reads a byte from PCI space */
    __inline__ static u8_t read8(word_t pci_addr) {
	volatile u32_t *addr = 
	    (volatile u32_t *) mk_cpu_addr(PCI_SPARSE_OFFSET, pci_addr, 1);

	mb();
	return extbl(*addr, pci_addr & 0x3);
    }

    __inline__ static void write8(word_t pci_addr, u8_t data) {
	volatile u32_t *addr = 
	    (volatile u32_t *) mk_cpu_addr(PCI_SPARSE_OFFSET, pci_addr, 1);

	*addr = insbl(data, pci_addr & 0x3);
	mb();
    }


    __inline__ static u8_t dense_read8(word_t pci_addr) {
	volatile u8_t *val = (volatile u8_t *) (PCI_DENSE_OFFSET + pci_addr);

	return *val;
    }

    __inline__ static void dense_write8(word_t pci_addr, u8_t data) {
	volatile u8_t *val = (volatile u8_t *) (PCI_DENSE_OFFSET + pci_addr);

	*val = data;
    }

    /* Reads a quadword from PCI space */
    __inline__ static u8_t read64(u32_t pci_addr) {
	volatile u64_t *addr = 
	    (volatile u64_t *) mk_cpu_addr(PCI_SPARSE_OFFSET, pci_addr, 1);

	mb();
	return *addr;
    }

    __inline__ static void write64(word_t pci_addr, u64_t data) {
	volatile u32_t *addr = 
	    (volatile u32_t *) mk_cpu_addr(PCI_SPARSE_OFFSET, pci_addr, 1);

	*addr = data;	
	mb();
    }

    /* sjw (28/01/2003): Is this correct? */
    /* ISA/IO-space compat. methods */
    __inline__ static u8_t inb(int port) {
	volatile u32_t *addr = 
	    (volatile u32_t *) mk_cpu_addr(PCI_IO_OFFSET, port, 1);
	
	mb();
	return extbl(*addr, port & 0x3); 
    }


    /* sjw (28/01/2003): Is this correct? */
    /* ISA/IO-space compat. methods */
    __inline__ static void outb(int port, u8_t data) {
	volatile u32_t *addr = 
	    (volatile u32_t *) mk_cpu_addr(PCI_IO_OFFSET, port, 1);

	*addr = insbl(data, port & 0x3);
	mb();
    }
};


#endif /* __PLATFORM_DEVSPACE_H__ */
