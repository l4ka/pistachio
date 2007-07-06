/****************************************************************************
 *
 * Copyright (C) 2003, University of New South Wales
 *
 * File path:	platform/ofpower3/opic.cc
 * Description:	OpenPIC interrupt controller.
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
 * $Id: opic.cc,v 1.3 2003/10/27 07:34:05 cvansch Exp $
 *
 ***************************************************************************/

#include <linear_ptab.h>

#include INC_GLUE(intctrl.h)
#include INC_ARCH(1275tree.h)
#include INC_PLAT(opic.h)
#include INC_ARCH(segment.h)
#include INC_GLUE(pgent_inline.h)

intctrl_t intctrl;

open_pic_t *opic = NULL;

SECTION(".init") void intctrl_t::init_arch()
{
    of1275_device_t *root;
    u32_t *prop, *cells, len;
    word_t n, address;

    root = get_of1275_tree()->find( "/" );
    printf( "OpenPIC init\n" );

    /* Find the Open PIC if present */
    if ( !root->get_prop( "platform-open-pic", (char **)&prop, &len ) )
    {
	printf( "*** no open-pic interrupt controller found\n" );
	return;
    }

    root->get_prop( "#address-cells", (char **)&cells, &len );

    n = *cells;

    for( address=0; n > 0; n-- )
	address = (address << 32) + *prop++;

    printf( "OpenPIC found at: %p\n", address);
    opic = (open_pic_t*)(address | DEVICE_AREA_START);

    pgent_t pg;
    /* XXX - we should lookup mapping first */
#ifdef CONFIG_POWERPC64_LARGE_PAGES
    pgent_t::pgsize_e size = pgent_t::size_16m;
#else
    pgent_t::pgsize_e size = pgent_t::size_4k;
#endif

    /* Insert mappings for hash page table */
    for ( word_t i = 0; i < (sizeof(open_pic_t)); i += page_size(size))
    {
	/* Create a page table entry, noexecute, nocache */
	pg.set_entry( get_kernel_space(), size,
			(addr_t)(address + i), true, true, false, true, pgent_t::cache_inhibit );

	get_pghash()->insert_mapping( get_kernel_space(),
			(addr_t)(((word_t)opic) + i), &pg, size, true );
    }

    open_pic_feature0_t f = opic->get_feature0();

    char *version;
    switch (f.x.version)
    {
    case 1: version = "1.0"; break;
    case 2: version = "1.2"; break;
    case 3: version = "1.3"; break;
    default: version = "??"; break;
    }

    printf( "OpenPIC version %s (%d CPUS, %d IRQ sources)\n", version,
				    f.x.last_cpu + 1, f.x.last_source + 1 );

    word_t freq  = opic->get_timer_frequency();
    printf( "OpenPIC timer frequency = %d.%06d MHz\n", freq / 1000000, freq % 1000000 );
}

