/*********************************************************************
 *                
 * Copyright (C) 2004-2006,  Karlsruhe University
 *                
 * File path:     mbi-loader.cc
 * Description:   MBI specific boot loader functions
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
 * $Id: mbi-loader.cc,v 1.10 2006/10/22 19:40:24 reichelt Exp $
 *                
 ********************************************************************/
#include <config.h>
#include <l4io.h>

#include "kickstart.h"
#include "mbi.h"
#include "kipmgr.h"
#include "lib.h"
#include "elf.h"
#include "bootinfo.h"

void install_memory(mbi_t * mbi, kip_manager_t* kip);

#define MAX_MBI_MODULES		32
#define STRING_BUFFER_SIZE	4096

static mbi_t mbi_copy;
static mbi_module_t mbi_modules[MAX_MBI_MODULES];
static mbi_module_t orig_mbi_modules[MAX_MBI_MODULES];
static char strings_copy[STRING_BUFFER_SIZE];

static mbi_t * mbi;

/**
 * Maximum size for bootinfo structure.
 */
static L4_Word_t max_bootinfo_size;


/* Override physical memory size from BIOS when non-zero.  Variable
   can be set with command line parameter maxmem=<num>[KMG] */
unsigned int max_phys_mem = 0;


/* Reserve this much conventional memory for the kernel
   Can be changed with command line parameter kmem=<num>[KMG] */
unsigned int additional_kmem_size = 16*1024*1024;


/* Decode all executable MBI modules. */
bool decode_all_executables = false;


/* ELF format types of root servers. */
L4_Word_t sigma0_type = 0;
L4_Word_t root_task_type = 0;


/**
 * Function used by load_modules to check for memory conflicts
 *
 * @param start First address belonging to the memory area.
 * @param end   First address after the memory area.
 *
 * @returns false on memory conflict, true otherwise.
 */
bool check_memory (L4_Word_t start, L4_Word_t end)
{
    for (L4_Word_t i = 0; i < mbi->modcount; i++)
    {
	L4_Word_t mod_start = orig_mbi_modules[i].start;
	L4_Word_t mod_end = orig_mbi_modules[i].end;

	if (mod_start < end && start < mod_end)
	{
	    printf("     Conflict with multiboot module %d (%p-%p)\n"
	           "     Please choose a different link base\n",
		   i,
		   (void *) mod_start,
		   (void *) mod_end);
	    return false;
	}
    }

    for (L4_Word_t i = 0; i < mbi->modcount; i++)
    {
	L4_Word_t mod_start = mbi->mods[i].start;
	L4_Word_t mod_end = mbi->mods[i].end;

	if (mod_start < end && start < mod_end)
	{
	    printf("     Conflict with module %d (%p-%p)\n"
	           "     Please choose a different link base\n",
		   i,
		   (void *) mod_start,
		   (void *) mod_end);
	    return false;
	}
    }

    return true;
}


/**
 * Load L4 system modules as referenced by the MBI structure
 *
 * The first three modules in the MBI modules list are ELF-loaded.  If
 * 'decode-all' is specified on the kickstart command line, other ELF
 * files in the module list are also ELF loader.  The memory range of
 * each of these modules is updated with the enclosing memory range of
 * the loaded image and the entry point is set for each successfully
 * loaded image.  Modules are loaded in the order they appear in the
 * MBI's module list.  No checks for overlapping of ELF images and
 * loaded images are performed.
 *
 * @returns     true on successful load, false otherwise.
 */
bool load_modules (void)
{
    // Is the modules info in the MBI valid?
    if (mbi->flags.mods)
    {
        for (L4_Word_t i = 0; i < mbi->modcount; i++)
            mbi->mods[i].entry = 0;

        /* We need at least three modules: kernel, sigma0, roottask */
        if (mbi->modcount >= 3)
        {
#define LOADIT(idx, name, type)                                             \
            do {                                                            \
                printf(name);                                               \
                if (!elf_load (mbi->mods[idx].start, mbi->mods[idx].end,    \
                               &mbi->mods[idx].start, &mbi->mods[idx].end,  \
                               &mbi->mods[idx].entry, type, check_memory))  \
                {                                                           \
                    FAIL();                                                 \
                }                                                           \
            } while(0)

            LOADIT(0, " kernel  ", NULL);
            LOADIT(1, " sigma0  ", &sigma0_type);
            LOADIT(2, " roottask", &root_task_type);
        }
        else
        {
            FAIL();
        }

	if (decode_all_executables)
	{
	    // Also decode other ELF files in module list.
	    for (L4_Word_t i = 3; i < mbi->modcount; i++)
		elf_load (mbi->mods[i].start, mbi->mods[i].end,
			  &mbi->mods[i].start, &mbi->mods[i].end,
			  &mbi->mods[i].entry, NULL, check_memory);
	}

	return true;
    }
    else
        FAIL();

    return false;
}


/**
 * Find free memory.
 *
 * @param size	The size of the region to find.
 * @param kip	A pointer to the kip manager.  All memory descriptors should
 * 		be defined before calling this function.
 *
 * Find a region of memory which doesn't overlap the kickstarter, the 
 * multiboot modules, nor the memory regions defined in the kip.
 *
 * @returns The start address of the available memory region, otherwise 0.
 */
L4_Word_t find_free_mem_region (L4_Word_t size, kip_manager_t *kip)
{
    L4_Word64_t phys_start, phys_end;

    phys_end = kip->get_phys_mem_max();
    for (phys_start = 0; phys_start < (phys_end - size); phys_start += size)
    {
	if( !mbi->is_mem_region_free(phys_start, size) )
	    continue;
	if( !kip->is_mem_region_free(phys_start, size) )
	    continue;
	return phys_start;
    }

    return 0;
}


/**
 * Prepare user-level copy of the mbi.
 *
 * @param kip	The kip manager pointer.  All memory descriptors should
 * 		be defined before calling this function.
 *
 * Prepares the mbi for use by user-level applications.  It allocates
 * contiguous memory for the mbi, and then copies the mbi to this
 * memory region.  All modules and strings are copied to the region too.
 * The new memory region is defined as a memory descriptor in the
 * kip, as a bootloader specific type.
 *
 * @returns The pointer to the newly allocated mbi.
 */
mbi_t * install_mbi (kip_manager_t* kip)
{
    // Make a copy of the mbi, and protect it.  First calculate its size.
    L4_Word_t mbi_size = mbi->get_size();
    if( mbi_size % 4096 )
	mbi_size = (mbi_size + 4096) & ~(4096-1);

    L4_Word_t target_mbi = find_free_mem_region( mbi_size, kip );
    if( target_mbi == 0 )
	FAIL();
    mbi->copy( (mbi_t *)target_mbi );

    // Protect the mbi.
    kip->dedicate_memory( target_mbi, target_mbi - 1 + mbi_size, 
	    L4_BootLoaderSpecificMemoryType, 
	    kip_manager_t::desc_init_table );

    return (mbi_t *)target_mbi;
}


/**
 * Allocate a new bootinfo structure. 
 *
 * @param kip		Pointer to KIP manager
 *
 * The function allocates memory for a bootinfo structure.
 * The amount of space allocated is the minimum page size supported by
 * the architecture/kernel.  The bootinfo structure is additionally
 * recorded in a memory descriptor in the KIP.
 *
 * @returns pointer to newly allocated bootinfo structure, or NULL if
 * unable to allocate memory to hold the structure.
 */
void * create_bootinfo (kip_manager_t * kip)
{
    max_bootinfo_size = kip->get_min_pagesize ();
    L4_Word_t bi = find_free_mem_region (max_bootinfo_size, kip);

    if (!bi)
	return NULL;

    // Protect bootinfo structure
    kip->dedicate_memory (bi,
			  bi + max_bootinfo_size - 1,
			  L4_BootLoaderSpecificMemoryType, 
			  kip_manager_t::desc_init_table);

    return (void *) bi;
}


/**
 * Check if a valid multiboot info structure is present.
 */
bool mbi_probe (void)
{
    mbi_t * _mbi = mbi_t::prepare();

    if (_mbi == NULL)
	return false;

    // Make a safe copy of the MBI structure itself.
    memcopy (&mbi_copy, _mbi, sizeof (mbi_t));
    mbi = &mbi_copy;

    return true;
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
L4_Word_t mbi_init (void)
{
    kip_manager_t kip;

    void * bi = NULL;
    bool use_bootinfo = true;
    bool use_mbi = true;

    // The KIP is somewhere in the kernel (module 0)
    if (!kip.find_kip(mbi->mods[0].start, mbi->mods[0].end))
    {
        // Bail out if we couldn't find a KIP
        FAIL();
    }

    // Command line strings tend to occupy the same space that we want
    // to use.  Make a copy of all the strings.
    char * sptr = strings_copy;
    L4_Word_t nfree = STRING_BUFFER_SIZE;
    L4_Word_t len;

#define COPY_STRING(str)			\
    do {					\
	len = strlen (str) + 1;			\
	if (len > nfree)			\
	{					\
	    printf ("String buffer overrun\n");	\
	    FAIL ();				\
	}					\
	strcpy (sptr, str);			\
	str = sptr;				\
	nfree -= len;				\
	sptr += len;				\
    } while (0)

    if (mbi->flags.cmdline)
    {
	char * p;
	COPY_STRING (mbi->cmdline);

#define PARSENUM(name, var, msg, massage...)			\
        if ((p = strstr(mbi->cmdline, name"=")) != NULL)	\
        {							\
            var = strtoul(p+strlen(name)+1, &p, 10);		\
            if (*p == 'K') var*=1024;				\
            if (*p == 'M') var*=1024*1024;			\
            if (*p == 'G') var*=1024*1024*1024;			\
            massage;						\
            printf(msg,						\
                   var >= 1<<30 ? var>>30 :			\
                   var >= 1<<20 ? var>>20 :			\
                   var >= 1<<10 ? var>>10 : var,		\
                   var >= 1<<30 ? "G" :				\
                   var >= 1<<20 ? "M" :				\
                   var >= 1<<10 ? "K" : "");			\
        }

#define PARSEBOOL(name, var, msg)				\
	if ((p = strstr (mbi->cmdline, name"=")) != NULL)	\
	{							\
	    p = strchr (p, '=') + 1;				\
	    if (strncmp (p, "yes", 3) == 0 ||			\
		strncmp (p, "on", 2) == 0 ||			\
		strncmp (p, "enable", 6) == 0)			\
	    {							\
		if (! var) printf ("Enabling %s\n", msg);	\
		var = true;					\
	    }							\
	    else if (strncmp (p, "no", 2) == 0 ||		\
		     strncmp (p, "off", 3) == 0 ||		\
		     strncmp (p, "disable", 7) == 0)		\
	    {							\
		if (var) printf ("Disabling %s\n", msg);	\
		var = false;					\
	    }							\
	}

        PARSENUM("maxmem",
                 max_phys_mem,
                 "Limiting physical memory to %d%sB\n");
        PARSENUM("kmem",
                 additional_kmem_size,
                 "Reserving %d%sB for kernel memory\n",
                 additional_kmem_size &= ~(kip.get_min_pagesize()-1));

	PARSEBOOL ("bootinfo", use_bootinfo, "generic bootinfo");
	PARSEBOOL ("mbi", use_mbi, "multiboot info");
	PARSEBOOL ("decode-all", decode_all_executables,
		   "decoding of all executables");
    }

    if (mbi->flags.mods)
    {
	if (mbi->modcount > MAX_MBI_MODULES)
	{
	    printf("WARNING: Restricting number of modules to %d (was %d)\n",
		   MAX_MBI_MODULES, mbi->modcount);
	    mbi->modcount = MAX_MBI_MODULES;
	}

        // Copy all mods array members into new mods array
        for (L4_Word_t i = 0; i < mbi->modcount; i++)
	{
	    COPY_STRING (mbi->mods[i].cmdline);
	    orig_mbi_modules[i] = mbi_modules[i] = mbi->mods[i];
	}
        mbi->mods = mbi_modules;

        /* Install the roottask's command line as the kernel command
           line in the MBI. By convention, the roottask is the third
           module. */
        if (mbi->modcount > 2)
            mbi->cmdline = mbi->mods[2].cmdline;
    }

    // Load the first three modules as ELF images into memory
    if (!load_modules())
    {
        // Bail out if loading failed
        printf("Failed to load all necessary modules\n");
        FAIL();
    }

    // Update with location of KIP in loader kernel
    if (!kip.find_kip(mbi->mods[0].start, mbi->mods[0].end))
        FAIL();

    // Set up the memory descriptors in the KIP
    install_memory(mbi, &kip);

    // Install sigma0's memory region and entry point in the KIP
    kip.install_sigma0(mbi->mods[1].start, mbi->mods[1].end,
                       mbi->mods[1].entry, sigma0_type);
    // Install the root_task's memory region and entry point in the KIP
    kip.install_root_task(mbi->mods[2].start, mbi->mods[2].end,
			  mbi->mods[2].entry, root_task_type);

    // Protect all user-level modules.
    for (L4_Word_t i = 3; i < mbi->modcount; i++)
	kip.dedicate_memory (mbi->mods[i].start, mbi->mods[i].end - 1,
			     L4_BootLoaderSpecificMemoryType, 
			     kip_manager_t::desc_boot_module);

#if defined(L4_32BIT) || defined(ALSO_BOOTINFO32)
    if (root_task_type == 1)
    {
	BI32::L4_BootRec_t * rec = NULL;

	if (use_bootinfo)
	{
	    // Allocate a bootinfo structure
	    bi = create_bootinfo (&kip);

	    if (bi)
	    {
		// Initialize it
		rec = BI32::init_bootinfo ((BI32::L4_BootInfo_t *) bi);

		// Record MBI modules
		rec = BI32::record_bootinfo_modules
				((BI32::L4_BootInfo_t *) bi,
				 rec, mbi, orig_mbi_modules,
				 decode_all_executables ? mbi->modcount : 3);
	    }
	}

	// Move the MBI into a dedicated memory region
	if (use_mbi)
	    mbi = install_mbi (&kip);

	if (bi && use_mbi)
	{
	    // Make sure that we record MBI location after we have
	    // installed it
	    rec = BI32::record_bootinfo_mbi ((BI32::L4_BootInfo_t *) bi,
					     rec, mbi);
	}
    }
#endif

#if defined(L4_64BIT) || defined(ALSO_BOOTINFO64)
    if (root_task_type == 2)
    {
	BI64::L4_BootRec_t * rec = NULL;

	if (use_bootinfo)
	{
	    // Allocate a bootinfo structure
	    bi = create_bootinfo (&kip);

	    if (bi)
	    {
		// Initialize it
		rec = BI64::init_bootinfo ((BI64::L4_BootInfo_t *) bi);

		// Record MBI modules
		rec = BI64::record_bootinfo_modules
				((BI64::L4_BootInfo_t *) bi,
				 rec, mbi, orig_mbi_modules,
				 decode_all_executables ? mbi->modcount : 3);
	    }
	}

	// Move the MBI into a dedicated memory region
	if (use_mbi)
	    mbi = install_mbi (&kip);

	if (bi && use_mbi)
	{
	    // Make sure that we record MBI location after we have
	    // installed it
	    rec = BI64::record_bootinfo_mbi ((BI64::L4_BootInfo_t *) bi,
					     rec, mbi);
	}
    }
#endif

    // Install the bootinfo or MBI into the KIP
    kip.update_kip (bi ? (L4_Word_t) bi :
		    use_mbi ? (L4_Word_t) mbi : 0);
    
    return mbi_modules[0].entry;
}
