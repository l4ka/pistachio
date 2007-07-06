/*********************************************************************
 *                
 * Copyright (C) 2002,  University of New South Wales
 *                
 * File path:     elf-loader/src/generic/loader.cc
 * Description:   Generic loader support 
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
 * $Id: loader.cc,v 1.9 2004/06/03 05:50:16 cvansch Exp $
 *                
 ********************************************************************/

#include <config.h>

#include <l4io.h>
#include "elf-loader.h"



/* KIP_ADDR is computed in the Makefile ... we just need to verify that it is
   correct */

extern char mod_kernel_start[];
extern char mod_kernel_end[];
extern char mod_sigma0_start[];
extern char mod_sigma0_end[];
extern char mod_sigma1_start[];
extern char mod_sigma1_end[];
extern char mod_root_start[];
extern char mod_root_end[];

L4_KernelConfigurationPage_t *kip = 0;

/* sjw (09/09/2002): This should be in a header file! */
static char kip_magic[] = {'L','4',230,'K'};

/* Note that this is only valid _after_ the kernel is loaded! */
static int validate_kip(L4_Word_t kip_addr)
{
    kip = (L4_KernelConfigurationPage_t *)kip_addr;

    printf("elf-loader: Looking for KIP at %lx\n", (unsigned long) kip);
    if(!kip) {
	printf("elf-loader:\tCan't find KIP!\n");
	return 1;
    }
    
    char *magic = (char *)&kip->magic;
    if(magic[0] != kip_magic[0] ||
       magic[1] != kip_magic[1] ||
       magic[2] != kip_magic[2] ||
       magic[3] != kip_magic[3]) {
	printf("elf-loader:\tKIP has an incorrect magic ('%c', '%c', '%c', '%c')\n",
	       magic[0], magic[1], magic[2], magic[3]);
	return 1;
    }

    return 0;
}

static int update_kip_servers(L4_KernelRootServer_t *sigma0, L4_KernelRootServer_t *sigma1, L4_KernelRootServer_t *root)
{
    kip->sigma0 = *sigma0;
    kip->sigma1 = *sigma1;
    kip->root_server = *root;

    return 0;
}

/* sjw (09/09/2002): May need to relocate images first so we don't overwrite something. */
int load_modules(L4_Word_t *entry, L4_Word_t offset)
{
    L4_KernelRootServer_t kernel = {0,}, sigma0, sigma1, root_task;

    sigma0 = sigma1 = root_task = kernel;

    install_module("root", mod_root_start, mod_root_end, &root_task, offset);
    install_module("sigma1", mod_sigma1_start, mod_sigma1_end, &sigma1, offset);
    install_module("sigma0", mod_sigma0_start, mod_sigma0_end, &sigma0, offset);
    install_module("kernel", mod_kernel_start, mod_kernel_end, &kernel, offset);

    /* If we get an invalid KIP, the arch-dependant part should do something intelligent, like
       halting */
    if(validate_kip(KIP_ADDR + offset))
	return 1;

    if(update_kip_servers(&sigma0, &sigma1, &root_task))
	return 1;

    *entry = kernel.ip;
    
    return 0;
}


