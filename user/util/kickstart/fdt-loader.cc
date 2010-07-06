/*********************************************************************
 *                
 * Copyright (C) 1999-2010,  Karlsruhe University
 * Copyright (C) 2008-2009,  Volkmar Uhlig, IBM Corporation
 *                
 * File path:     fdt-loader.cc
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
 * $Id$
 *                
 ********************************************************************/
#include <config.h>
#include <l4io.h>
#include "fdt.h"
#include "elf.h"
#include "kipmgr.h"
#include "kickstart.h"

void flush_dcache_range(L4_Word_t start, L4_Word_t end);
void install_memory(fdt_t * fdt, kip_manager_t* kip);

class module_t {
public:
    L4_Word_t elfimage;
    L4_Word_t start;
    L4_Word_t end;
    L4_Word_t entry;
    L4_Word_t type;
};

#define MAX_MODULES	32
static module_t modules[MAX_MODULES];


bool fdt_probe (void)
{
    return get_fdt_ptr()->is_valid();
}

bool check_memory (L4_Word_t start, L4_Word_t end)
{
    return true;
}

static bool fdt_load_image(fdt_t *fdt, char *name, module_t &module)
{
    fdt_property_t *image = fdt->find_property_node(name);
    if (!image)
    {
        printf("Could'nt find FDT entry %s\n", name);
	return false;
    }
    if (image->len != sizeof(L4_Word_t)) {
	printf("Invalid FDT entry size, (expected %d, found %d)\n",
	       sizeof(L4_Word_t), image->len);
	return false;
    }

    module.elfimage = image->get_word(0);

    printf("image address %s: %lx\n", name, module.elfimage);

    bool res = elf_load(module.elfimage, 0, &module.start, &module.end, 
			&module.entry, &module.type, check_memory);

    if (res)
	flush_dcache_range(module.start, module.end);
    
    return res;
}


/**
 * Init function that understands multiboot info structure.
 *
 * The procedure goes as follows:
 * - Find/prepare an MBI structure
 * - ELF-load the first three modules (kernel,sigma0,roottask)
 * - Find the KIP in the kernel
 * - Install memory descriptors from the MBI in the KIP
 * - Install initial servers (sigma0,roottask) in the KIP
 * - Store the bootinfo value in the KIP
 * - Flush caches
 * - Launch the kernel
 *
 * @returns entry point for kernel
 */
L4_Word_t fdt_init (void)
{
    kip_manager_t kip;
    fdt_t *fdt = get_fdt_ptr();

    fdt_load_image(fdt, "/l4/kernel", modules[0]);
    fdt_load_image(fdt, "/l4/sigma0", modules[1]);
    fdt_load_image(fdt, "/l4/roottask", modules[2]);

    if (!kip.find_kip(modules[0].start, modules[0].end)) {
	printf("Couldn't find KIP...\n");
	FAIL();
    }

    // install the memory as provided in the FDT
    install_memory(fdt, &kip);

    // Install sigma0's memory region and entry point in the KIP
    kip.install_sigma0(modules[1].start, modules[1].end,
                       modules[1].entry, modules[1].type);

    // Install the root_task's memory region and entry point in the KIP
    kip.install_root_task(modules[2].start, modules[2].end,
			  modules[2].entry, modules[2].type);

    kip.dedicate_memory((L4_Word64_t) fdt, (L4_Word64_t) fdt + fdt->size, L4_BootLoaderSpecificMemoryType, 0xf );

    // MUST BE LAST: store the fdt in the bootinfo field and update
    // all descriptors
    kip.update_kip((L4_Word_t)fdt);

    return modules[0].entry;
}


