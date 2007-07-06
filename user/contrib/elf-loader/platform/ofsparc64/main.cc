/*********************************************************************
 *
 * Copyright (C) 2003, University of New South Wales
 *
 * File path:    contrib/elf-loader/platform/ofsparc64/main.cc
 * Description:  Main file for elf-loader on sparc v9 OpenBoot
 *               (Open Firmware) platforms.
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
 * $Id: main.cc,v 1.4 2004/05/21 02:34:59 philipd Exp $
 *
 ********************************************************************/

#include <kip.h>
#include <arch.h>
#include <elf-loader.h>
#include <openfirmware/memory.h>
#include <openfirmware/console.h>
#include <openfirmware/device_tree.h>
#include <openfirmware/openfirmware.h>

#warning awiggins (14-08-03): This needs to be fixed up to make KIP_ADDR phys!
void * kip_paddr = (void *)(KIP_ADDR - 0xFFFFF80000000000);

int
main(void)
{
    ofw_devtree_t devtree; // Device tree.
    L4_Word_t entry;       // Kernel entry point.

    ofw_setup_console();

    printf("\nelf-loader:\tStarting.\n");

    kip_manager.init(kip_paddr);

    ofw_setup_physmem();
    ofw_setup_virtmem();

#warning awiggins (08-09-03): Fix this hardcoding up!
    ofw_map_virtmem(0, 1 << 27);

    /* Load modules up. */

    if(load_modules(&entry, 0)) {
	ofw_error("main:\tSomething went wrong loading modules");
    }

    /* Setup Open Firmware device tree. */
    //L4_Word_t devtree_start = kip_manager.first_avail_page();
    //L4_Word_t devtree_size  = devtree.build((char *)devtree_start);
    //L4_Word_t devtree_end   = wrap_up(devtree_start + devtree_size, PAGE_SIZE);

    //kip_manager.add_memdesc(0, devtree_start, devtree_end, 
    //			  L4_BootLoaderSpecificMemoryType,
    //			  OFWMemorySubType_DeviceTree);

    //printf("elf-loader:\tOpen firmware device tree located at 0x%lx - 0x%lx.\n");

    /* Setup KIP. */
    kip_manager.update();

    /* Jump to kernel. */

    void (*func)(unsigned long) = (void (*)(unsigned long))entry;

    printf("elf-loader:\tJumping to kernel at 0x%lx\n", entry);

    func(0);

} // main()

extern "C" __attribute__ ((weak)) void *
memcpy (void * dst, const void * src, unsigned int len)
{
    unsigned char *d = (unsigned char *) dst;
    unsigned char *s = (unsigned char *) src;

    while (len-- > 0)      
	*d++ = *s++;

    return dst;

} // memcpy()
