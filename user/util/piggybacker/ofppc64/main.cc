/****************************************************************************
 *
 * Copyright (C) 2002-2003, Karlsruhe University
 *
 * File path:	piggybacker/ofppc64/main.cc
 * Description:	The Open Firmware PowerPC64 loader for Pistachio.
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
 * $Id: main.cc,v 1.4 2005/01/19 14:08:15 cvansch Exp $
 *
 ***************************************************************************/

#include <piggybacker/piggyback.h>
#include <piggybacker/kip.h>
#include <piggybacker/io.h>
#include <piggybacker/elf.h>
#include <piggybacker/string.h>
#include <piggybacker/ieee1275.h>
#include <piggybacker/1275tree.h>
#include <piggybacker/powerpc64/page.h>

typedef struct {
    word_t vaddr;
    word_t size;
    word_t paddr;
    word_t mode;
} of1275_map_t;

typedef union {
    struct {
	L4_Word_t base;
	L4_Word_t size;
    } r64_64;
    struct {
	L4_Word_t base;
	L4_Word32_t size;
    } r64_32;
    struct u32 {
	L4_Word32_t base;
	L4_Word32_t size;
    } r32_32;
} range_t;

kip_manager_t kip_manager;

extern "C" void 
enter_kernel( L4_Word_t r3, L4_Word_t r4, L4_Word_t r5, L4_Word_t ip );

void boot_fatal( const char *msg )
{
    puts( msg );
    puts( "Aborting ..." );
    prom_exit();
}

static L4_Word32_t cellsize = 1;
static L4_Word32_t addrsize = 1;

bool detect_of1275_memory( device_t *mem_node, L4_Word_t tot_mem )
{
    // Look for available memory.
    item_t *available = item_find( mem_node, "available" );
    if( !available )
	return false;

    puts( "[==== Detecting OpenFirmware Memory ====]" );
    range_t *ranges = (range_t *)available->data;
    int cnt = available->len / (sizeof(L4_Word32_t) * 2 * cellsize);

    // Invert the list of available regions into a list of reserved
    // regions.  We assume that Open Firmware provides a sorted list
    // of available regions.
    L4_Word_t last = 0;
    L4_Word_t me = (L4_Word_t)detect_of1275_memory;
    for( int i = 0; i < cnt; i++ )
    {
	switch( cellsize ) {
	case 1:
	    if( ((me < last) || (me > ranges->r32_32.base)) &&
		(ranges->r32_32.base > last) )
	    {
		print_hex("  reserved : ", last);
		print_hex(" - ", ranges->r32_32.base);
		puts("");

		// Protect OpenFirmware memory, unless it is our bootloader.
		kip_manager.dedicate_memory( last, ranges->r32_32.base, 
			L4_BootLoaderSpecificMemoryType, 0xe );
	    }

	    last = ranges->r32_32.base + ranges->r32_32.size;
	    ranges = (range_t *)((L4_Word_t)ranges + (sizeof(L4_Word32_t) * 2));
	    break;
	case 2:
	    if( ((me < last) || (me > ranges[i].r64_64.base)) &&
		(ranges[i].r64_64.base > last) )
	    {
		print_hex("  reserved : ", last);
		print_hex(" - ", ranges[i].r64_64.base);
		puts("");

		// Protect OpenFirmware memory, unless it is our bootloader.
		kip_manager.dedicate_memory( last, ranges[i].r64_64.base, 
			L4_BootLoaderSpecificMemoryType, 0xe );
	    }

	    last = ranges[i].r64_64.base + ranges[i].r64_64.size;
	    break;
	}
    }

    // Determine whether we have a reserved region after the last available
    // region.
    if( (me < last) && (last < tot_mem) )
    {
	kip_manager.dedicate_memory( last, tot_mem,
		L4_BootLoaderSpecificMemoryType, 0xe );
	print_hex("  reserved : ", last);
	print_hex(" - ", tot_mem);
	puts("");
    }

    return true;
}

word_t size_of_memory( char *devtree )
{
    device_t *node = device_first( devtree );

    L4_Word_t total = 0;
    item_t *item_name, *item_data;

    item_data = item_find( node, "#size-cells" );
    if (!item_data)
	boot_fatal( "No cell size found\n" );
    cellsize = *(L4_Word32_t *)item_data->data;

    item_data = item_find( node, "#address-cells" );
    if (!item_data)
	addrsize = cellsize;
    else
	addrsize = *(L4_Word32_t *)item_data->data;

    while( node->handle )
    {
	item_name = item_first( node );
	item_data = item_next( item_name );
	if( !strcmp(item_data->data, "memory") )	// name == "memory" ??
	{
	    item_t *reg = item_find( node, "reg" );
	    range_t *ranges = (range_t *)&reg->data;
	    L4_Word_t i;
	    L4_Word_t tot = 0;
	    L4_Word_t cnt = reg->len / (sizeof(L4_Word32_t) * (addrsize + cellsize));

	    for( i = 0; i < cnt; i ++ )
	    {
		switch (addrsize) {
		case 1:
		    tot += ranges->r32_32.size;
		    ranges = (range_t *)((L4_Word_t)ranges + (4 * 2));
		    break;
		case 2:
		    switch (cellsize) {
		    case 1:
			tot += ranges->r64_32.size;
			ranges = (range_t *)((L4_Word_t)ranges + (4 * 3));
			break;
		    case 2:
			tot += ranges->r64_64.size;
			ranges = (range_t *)((L4_Word_t)ranges + (4 * 4));
			break;
		    }
		    break;
		}
	    }
	    total += tot;
	}
	node = device_next( node );
    }

    return total;
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

    // Each memory descriptor contains the physical base address of the
    // memory bank, and a size.  We sum the sizes.

    word_t tot = size_of_memory( devtree );

    print_hex( "Detected memory (bytes)", tot );
    puts( "" );

    if( tot == 0ul )
	boot_fatal( "Error: didn't detect any platform memory." );

    // XXX this is broken for discontiguous physical memory
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
    puts( "[ L4 PowerPC64 ]" );

    enter_kernel( r3, r4, r5, kernel_start_ip );
}

L4_Word_t tree[0x10000];

extern "C" void loader_main( L4_Word_t r3, L4_Word_t r4, L4_Word_t of1275_entry)
    /* The entry point for the loader's C code.
     */
{
    prom_init( of1275_entry );
    puts( "[==== Pistachio PowerPC64 Open Firmware Boot Loader ====]\n" );

    kip_manager.init();

    // Install the modules.
    kip_manager.install_sigma0( get_sigma0_start(), get_sigma0_end() );
    kip_manager.install_root_task( get_root_task_start(), get_root_task_end() );
    kip_manager.install_kernel( get_kernel_start(), get_kernel_end() );

    // Generate the device tree.
    L4_Word_t devtree_start = kip_manager.first_avail_page();
    devtree_start = (L4_Word_t)&tree;
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
    register L4_Word32_t result;

    asm volatile (
	"mtctr	%1; "
	"mr	3, %2;"
	"stdu	1, -1024(1);"	/* XXX save and update stack */
	"std	2,  -8(1);"
	"std	3, -16(1);"
	"std	4, -24(1);"
	"std	5, -32(1);"
	"std	6, -40(1);"
	"std	7, -48(1);"
	"std	8, -56(1);"
	"std	9, -64(1);"
	"std	10,-72(1);"
	"std	11,-80(1);"
	"std	12,-88(1);"
	"std	13,-96(1);"
	"std	14,-104(1);"
	"std	15,-112(1);"
	"std	16,-120(1);"
	"std	17,-128(1);"
	"std	18,-136(1);"
	"std	19,-144(1);"
	"std	20,-152(1);"
	"std	21,-160(1);"
	"std	22,-168(1);"
	"std	23,-176(1);"
	"std	24,-184(1);"
	"std	25,-192(1);"
	"std	26,-200(1);"
	"std	27,-208(1);"
	"std	28,-216(1);"
	"std	29,-224(1);"
	"std	30,-232(1);"
	"std	31,-240(1);"

	"mfcr   4;"
	"std    4,-248(1);"
	"mfctr  5;"
	"std    5,-256(1);"
	"mfxer  6;"
	"std    6,-264(1);"
	"mfdar  7;"
	"std    7,-272(1);"
	"mfdsisr 8;"
	"std    8,-280(1);"
	"mfsrr0 9;"
	"std    9,-288(1);"
	"mfsrr1 10;"
	"std    10,-296(1);"
	"mfmsr  11;"
	"std    11,-304(1);"

	/* Unfortunately, the stack pointer is also clobbered, so it is saved
	 * in the SPRG2 which allows us to restore our original state after
	 * PROM returns.
	 */
	"mtsprg  2,1;"

	"mfmsr   11;"                     /* grab the current MSR */
	"li      12,1;"
	"rldicr  12,12,63,(63-63);"
	"andc    11,11,12;"
	"li      12,1;"
	"rldicr  12,12,61,(63-61);"
	"andc    11,11,12;"
	"mtmsrd  11;"
	"isync;"

	"bctrl;	    "

	"mfsprg  1, 2;"                /* Restore the stack pointer */
	"ld      6,-304(1);"             /* Restore the MSR */
	"mtmsrd  6;"
	"isync;"

	"ld	2, -8(1);"		/* Restore the TOC */
	"ld     13, -96(1);"            /* Restore current */
	/* Restore the non-volatiles */
	"ld	14,-104(1);"
	"ld	15,-112(1);"
	"ld	16,-120(1);"
	"ld	17,-128(1);"
	"ld	18,-136(1);"
	"ld	19,-144(1);"
	"ld	20,-152(1);"
	"ld	21,-160(1);"
	"ld	22,-168(1);"
	"ld	23,-176(1);"
	"ld	24,-184(1);"
	"ld	25,-192(1);"
	"ld	26,-200(1);"
	"ld	27,-208(1);"
	"ld	28,-216(1);"
	"ld	29,-224(1);"
	"ld	30,-232(1);"
	"ld	31,-240(1);"

	"ld      4,-248(1);"
	"mtcr    4;"
	"ld      5,-256(1);"
	"mtctr   5;"
	"ld      6,-264(1);"
	"mtxer   6;"
	"ld      7,-272(1);"
	"mtdar   7;"
	"ld      8,-280(1);"
	"mtdsisr 8;"
	"ld      9,-288(1);"
	"mtsrr0  9;"
	"ld      10,-296(1);"
	"mtsrr1  10;"

	"addi	1, 1, 1024;"	/* XXX fix stack */
	: "=r" (result)
	: "r" (call_addr),
	  "r" (args)
	: "lr", "memory"
    );

    return result;
}
