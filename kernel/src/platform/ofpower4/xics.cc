/****************************************************************************
 *
 * Copyright (C) 2003, University of New South Wales
 *
 * File path:	platform/ofpower4/xics.cc
 * Description:	IBM XICS interrupt controller.
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
 * $Id: xics.cc,v 1.3 2003/10/27 07:33:42 cvansch Exp $
 *
 ***************************************************************************/

#include INC_GLUE(intctrl.h)
#include INC_ARCH(rtas.h)
#include INC_ARCH(1275tree.h)
#include INC_PLAT(xics.h)
#include INC_PLAT(prom.h)
#include INC_GLUE(space.h)
#include INC_GLUE(pghash.h)
#include INC_GLUE(pgent_inline.h)

intctrl_t intctrl;

/* RTAS service tokens */
static u32_t ibm_get_xive = RTAS_SERVICE_UNKNOWN;
static u32_t ibm_set_xive = RTAS_SERVICE_UNKNOWN;
static u32_t ibm_int_on = RTAS_SERVICE_UNKNOWN;
static u32_t ibm_int_off = RTAS_SERVICE_UNKNOWN;

/* Globals */
static u32_t default_server = 0xFF;
static u32_t default_distrib_server = 0;

static word_t intr_base = 0;

static struct xics_valid_range {
    u32_t   low;
    u32_t   high;
} xics_valid;

static xics_info_t xics_info;

xics_interrupt_table_t xicp_table;


SECTION(".init") void intctrl_t::init_arch()
{
    of1275_device_t *xicp;
    u32_t *prop, len;
    word_t index;

    get_rtas()->get_token( "ibm,get-xive", &ibm_get_xive );
    get_rtas()->get_token( "ibm,set-xive", &ibm_set_xive );
    get_rtas()->get_token( "ibm,int-on", &ibm_int_on );
    get_rtas()->get_token( "ibm,int-off", &ibm_int_off );

    xicp = get_of1275_tree()->find_device_type( "PowerPC-External-Interrupt-Presentation" );

    if (!xicp)
	enter_kdebug( "No external interrupt presentation node found" );

    xics_valid.low = 0;
#ifdef CONFIG_SMP
    xics_valid.high = CONFIG_SMP_MAX_CPUS;
#else
    xics_valid.high = 32;
#endif

    if (xicp->get_prop( "ibm,interrupt-server-ranges", (char **)&prop, &len ))
    {
	xics_valid.low = *prop++;
	if (*prop < xics_valid.high)
	    xics_valid.high = *prop;
    }
    
    if ( !xicp->get_prop( "reg", (char **)&prop, &len ))
	enter_kdebug( "Cannot find interrupt reg property" );

    for ( index = xics_valid.low; index < xics_valid.high; index ++ )
    {
	xicp_table.node[index].addr = (word_t)*prop++ << 32;
	xicp_table.node[index].addr |= (word_t)*prop++;
	xicp_table.node[index].size = (word_t)*prop++ << 32;
	xicp_table.node[index].size |= (word_t)*prop++;
    }

    /* We assumed that we only have one xicp type node */

    of1275_device_t *cpu;
    /* Find the interrupt server numbers for the boot cpu. */
    for ( cpu = get_of1275_tree()->find_device_type("cpu"); cpu; cpu = cpu->next_by_type( "cpu" ) )
    {
        if ( !cpu->get_prop( "reg", (char **)&prop, &len ))
	    enter_kdebug( "Cannot find cpu reg property" );

	if ( prop[0] == boot_cpuid )
	{
	    if ( !cpu->get_prop( "ibm,ppc-interrupt-gserver#s", (char **)&prop, &len ))
		enter_kdebug( "Cannot find cpu gserver property" );

            word_t i = len / sizeof(u32_t);

	    default_server = prop[0];
            default_distrib_server = prop[i-1];	/* last element */
	    break;
        }
    }

    /* Get the base address */
    intr_base = DEVICE_AREA_START | xicp_table.node[0].addr;

    /* XXX No ISA stuff is done here yet. Can user do this ? */

    ASSERT( xicp_table.node[0].size == POWERPC64_PAGE_SIZE );

    pgent_t pg;

    TRACE_INIT( "Found XICS at %p\n", xicp_table.node[0].addr );

    /* XXX - these should be created lazily for devices!! */
    /* Create a dummy page table entry */
    pg.set_entry( get_kernel_space(), pgent_t::size_16m,
		    (addr_t)xicp_table.node[0].addr,
		    true, true, false, true, pgent_t::cache_inhibit );
    /* Insert the kernel mapping, bolted */
    get_pghash()->insert_mapping( get_kernel_space(),
		    (addr_t)(DEVICE_AREA_START | xicp_table.node[0].addr),
		    &pg, pgent_t::size_16m, true );

    for (index = xics_valid.low; index < xics_valid.high; index ++ )
    {
	xics_info.per_cpu[index] = (volatile xics_ipl_t*)
		(DEVICE_AREA_START | xicp_table.node[index].addr);
    }

    TRACE_INIT( "XICS initilized\n" );
};

