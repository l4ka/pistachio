/*********************************************************************
 *                
 * Copyright (C) 2002-2003,   University of New South Wales
 *                
 * File path:     platform/tsunami/devspace.h
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
 * $Id: devspace.h,v 1.3 2003/09/24 19:05:01 skoglund Exp $
 *                
 ********************************************************************/

#ifndef __PLATFORM_DEVSPACE_H__
#define __PLATFORM_DEVSPACE_H__

#include INC_ARCH(page.h)

class devspace_t {
 private:
    /* Various PCI spaces */
    /* No sparse I/O on the Tsunami */
    enum pci_space {
	PCI_DENSE_OFFSET = (AS_KSEG_START + 0x10000000000UL),
	PCI_IO_OFFSET    = (AS_KSEG_START + 0x101fc000000UL)
    };

 public:
    __inline__ static void mb() {
	__asm__ __volatile__("mb": : :"memory");
    }

    __inline__ static u8_t dense_read8(word_t pci_addr) {
	volatile u8_t *val = (volatile u8_t *) (PCI_DENSE_OFFSET + pci_addr);

	return *val;
    }

    __inline__ static void dense_write8(word_t pci_addr, u8_t data) {
	volatile u8_t *val = (volatile u8_t *) (PCI_DENSE_OFFSET + pci_addr);

	*val = data;
    }

    __inline__ static u8_t inb(int port) {
	volatile u8_t *val = (volatile u8_t *) (PCI_IO_OFFSET + port);

	mb();
	return *val;
    }


    __inline__ static void outb(int port, u8_t data) {
	volatile u8_t *val = (volatile u8_t *) (PCI_IO_OFFSET + port);

	*val = data;
	mb();
    }
};


#endif /* __PLATFORM_DEVSPACE_H__ */
