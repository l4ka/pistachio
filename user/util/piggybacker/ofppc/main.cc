/****************************************************************************
 *
 * Copyright (C) 2002-2003, Karlsruhe University
 *
 * File path:	piggybacker/ofppc/main.cc
 * Description:	The Open Firmware PowerPC loader for Pistachio.
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
 * $Id: main.cc,v 1.15 2004/01/16 13:39:53 joshua Exp $
 *
 ***************************************************************************/

#include <piggybacker/piggyback.h>
#include <piggybacker/kip.h>
#include <piggybacker/io.h>
#include <piggybacker/elf.h>
#include <piggybacker/string.h>
#include <piggybacker/ieee1275.h>
#include <piggybacker/1275tree.h>
#include <piggybacker/powerpc/page.h>

typedef struct
{
    word_t vaddr;
    word_t size;
    word_t paddr;
    word_t mode;
} of1275_map_t;

typedef struct
{
    word_t base;
    word_t size;
} range_t;

kip_manager_t kip_manager;

extern "C" void 
enter_kernel( L4_Word_t r3, L4_Word_t r4, L4_Word_t r5, L4_Word_t ip );

void boot_fatal( const char *msg )
{
    puts( msg );
    puts( "Aborting ..." );
    while( 1 ) ;
}

bool detect_of1275_memory( device_t *mem_node, L4_Word_t tot_mem )
{
    // Look for available memory.
    item_t *available = item_find( mem_node, "available" );
    if( !available )
	return false;

    range_t *ranges = (range_t *)available->data;
    int cnt = available->len / sizeof(range_t);

    // Invert the list of available regions into a list of reserved
    // regions.  We assume that Open Firmware provides a sorted list
    // of available regions.
    L4_Word_t last = 0;
    L4_Word_t me = (L4_Word_t)detect_of1275_memory;
    for( int i = 0; i < cnt; i++ )
    {
	if( (me < last) || (me > ranges[i].base) )
	{
	    // Protect OpenFirmware memory, unless it is our bootloader.
	    kip_manager.dedicate_memory( last, ranges[i].base, 
		    L4_BootLoaderSpecificMemoryType, 0xe );
	}

	last = ranges[i].base + ranges[i].size;
    }

    // Determine whether we have a reserved region after the last available
    // region.
    if( (me < last) && (last < tot_mem) )
	kip_manager.dedicate_memory( last, tot_mem,
		L4_BootLoaderSpecificMemoryType, 0xe );

    return true;
}

void detect_platform_memory( char *devtree )
{
    device_t *node, *list_head = device_first( devtree );
    device_t *mem_node;

    // Look for the platform's memory device node handle.
    node = device_find( list_head, "/chosen" );
    if( !node )
	boot_fatal( "Error: unable to find the OpenFirmware /chosen node." );

    item_t *memory = item_find( node, "memory" );
    if( !memory )
	boot_fatal( "Error: unable to find the OpenFirmware /chosen/memory "
		"property." );
    L4_Word32_t mem_handle = *(L4_Word32_t *)memory->data;

    // Lookup the device node for the memory node handle.
    mem_node = device_find_handle( list_head, mem_handle );
    if( !mem_node )
	boot_fatal( "Error: unable to lookup the OpenFirmware memory node "
		"handle." );

    // Grab the "reg" property from the memory node, which contains an
    // array of memory descriptors corresponding to the memory banks.
    item_t *reg = item_find( mem_node, "reg" );
    if( !reg )
	boot_fatal( "Error: unable to find the OpenFirmware memory reg "
		"property." );

    // Each memory descriptor contains the physical base address of the
    // memory bank, and a size.  We sum the sizes.

    L4_Word_t *ranges = (L4_Word_t *)&reg->data;
    L4_Word_t i, cnt = reg->len / sizeof(L4_Word_t);
    L4_Word_t tot = 0;

    for( i = 1; i < cnt; i += 2 )
	tot += ranges[i];

    if( tot == 0 )
	boot_fatal( "Error: didn't detect any platform memory." );

    print_hex( "Detected memory (bytes)", tot );
    puts( "" );

    // Update the kip to reflect the amount of installed physical memory.
    kip_manager.setup_main_memory( 0, tot );
    kip_manager.dedicate_memory( 0, tot, L4_ConventionalMemoryType, 0 );

    // Update the kip to reflect Open Firmware's claims on memory.
    if( !detect_of1275_memory(mem_node, tot) )
	puts( "Warning: unable to detect Open Firmware's memory "
		"requirements.\n" );
}

void start_kernel( L4_Word_t r3, L4_Word_t r4, L4_Word_t r5 )
{
    elf_ehdr_t *ehdr = (elf_ehdr_t *)get_kernel_start();
    elf_phdr_t *phdr = (elf_phdr_t *)((L4_Word_t)ehdr + ehdr->e_phoff);

    // Convert the kernel's start address into a physical address.
    L4_Word_t kernel_start_ip = ehdr->e_entry - phdr->p_vaddr + phdr->p_paddr;

    print_hex( "Kernel physical entry point", kernel_start_ip );
    puts( "" );
    puts( "" );
    puts( "[ L4 PowerPC ]" );

    enter_kernel( r3, r4, r5, kernel_start_ip );
}

void map_ram()
    /* Create a 1:1 256MB data mapping for the lower 256MB of memory.
     */
{
    ppc_bat_t bat;

    bat.raw.lower = 0;
    bat.raw.upper = 0;
    bat.x.pp = BAT_PP_READ_WRITE;
    bat.x.vs = 1;
    bat.x.bl = BAT_BL_256M;

    asm volatile ("isync");
    ppc_set_dbat3l( bat.raw.lower );
    ppc_set_dbat3u( bat.raw.upper );
    asm volatile ("isync");
}

extern "C" void loader_main( L4_Word_t r3, L4_Word_t r4, L4_Word_t of1275_entry)
    /* The entry point for the loader's C code.
     */
{
    prom_init( of1275_entry );
    puts( "[==== Pistachio PowerPC Open Firmware Boot Loader ====]" );

    map_ram();
    kip_manager.init();

    // Install the modules.
    kip_manager.install_sigma0( get_sigma0_start(), get_sigma0_end() );
    kip_manager.install_root_task( get_root_task_start(), get_root_task_end() );
    kip_manager.install_kernel( get_kernel_start(), get_kernel_end() );

    // Generate the device tree.
    L4_Word_t devtree_start = kip_manager.first_avail_page();
    L4_Word_t devtree_size = build_device_tree( (char *)devtree_start );
    L4_Word_t devtree_end = wrap_up( devtree_start + devtree_size, PAGE_SIZE );

    // Print info.
    print_hex( "Device tree page", devtree_start );
    print_hex( ", length", devtree_size );
    puts( "" );

    // Locate the kip and update.
    if( !kip_manager.find_kip(get_kernel_start()) ) 
	boot_fatal( "Error: unable to locate the kernel interface page!" );

    detect_platform_memory( (char *)devtree_start );

    kip_manager.dedicate_memory( devtree_start, devtree_end, 
	    L4_BootLoaderSpecificMemoryType, 0xf );
    kip_manager.update_kip();	// Do this last!

    start_kernel( r3, r4, of1275_entry );
}

extern word_t call_addr;

L4_Word32_t prom_entry ( void * args )
{
    return ((L4_Word32_t(*)( void * ))call_addr)( args );
}
