/****************************************************************************
 *
 * Copyright (C) 2003,  National ICT Australia (NICTA)
 *
 * File path:	arch/powerpc64/rtas.cc
 * Description:	OpenFirmware Real Time Abstraction Service (RTAS) Interface.
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
 * $Id: rtas.cc,v 1.6 2004/06/04 03:40:20 cvansch Exp $
 *
 ***************************************************************************/

#include INC_API(kernelinterface.h)
#include INC_ARCH(of1275.h)
#include INC_ARCH(rtas.h)
#include INC_PLAT(prom.h)
#include INC_GLUE(hwspace.h)
#include <stdarg.h>
#include <debug.h>

/* The RTAS structure */
rtas_t rtas;

extern addr_t kip_get_phys_mem( kernel_interface_page_t *kip );
extern char _end_kernel_phys[];

extern "C" void __call_rtas( void * arg );

void rtas_args_t::setup( u32_t token, u32_t nargs, u32_t nret )
{
    ASSERT((nargs+nret) < 16);

    this->token = token;
    this->nargs = nargs;
    this->nret  = nret;
    this->rets  = (rtas_arg_t *)&(this->args[nargs]);

    for (word_t i = 0; i < nret; i++)
	this->rets[i] = 0;
}

void rtas_args_t::set_arg( u32_t num, rtas_arg_t value )
{
    this->args[num] = value;
}

rtas_arg_t rtas_args_t::get_ret( u32_t num )
{
    return this->rets[num];
}

/* Initialise the RTAS
 * Note, we are running with relocation off.
 */
void SECTION(".init") rtas_t::init_arch( void )
{
    of1275_phandle_t prom_rtas;

    this->base = 0;
    this->entry = 0;
    this->lock.init();

    prom_puts( "Initialising IBM RTAS extensions\n\r" );

    rtas_dev = get_of1275_tree()->find( "/rtas" );
    prom_rtas = get_of1275()->find_device( "/rtas" );

    // Sanity check 1275tree
    ASSERT((u32_t)prom_rtas == rtas_dev->get_handle());

    if (prom_rtas != OF1275_INVALID_PHANDLE)
    {
	u32_t rtas_size;
	word_t rtas_alloc_size;
	word_t phys_start;
	word_t total_mem;
	kernel_interface_page_t * kip = get_kip();
	bool found = false;

	get_of1275()->get_prop( prom_rtas, "rtas-size", &rtas_size, sizeof(rtas_size));

	this->size = rtas_size;

	rtas_alloc_size = (word_t)addr_align_up ((addr_t)(word_t)rtas_size, POWERPC64_PAGE_SIZE);
	
	total_mem = (word_t)kip_get_phys_mem(kip);

	for( phys_start = (word_t)_end_kernel_phys; 
		phys_start < (total_mem - rtas_alloc_size); 
		phys_start += KB(4))
	{
	    if( this->try_location(phys_start, rtas_alloc_size) )
	    {
		s32_t results[2];

		found = true;
		// Insert a KIP memory descriptor to protect the page hash.
		kip->memory_info.insert( memdesc_t::reserved, false,
		    (addr_t)phys_start, (addr_t)(phys_start + rtas_alloc_size) );

		this->base = phys_start;
		prom_rtas = get_of1275()->open( "/rtas" );

		get_of1275()->call_method( prom_rtas, "instantiate-rtas",
				    results, 2, 1, (u32_t)this->base);

		this->entry = results[1];
		break;
	    }
	}
	if (!found)
	{
	    prom_puts( "No physical area large enough for RTAS found\n\r" );
	    get_of1275()->exit();
	}
    }
    else
    {
	prom_puts( "RTAS extensions not found\n\r" );
	get_of1275()->exit();
    }

    if (this->entry == 0)
    {
	prom_puts( "RTAS instatiate failed\n\r" );
	get_of1275()->exit();
    } else
    {
	prom_print_hex( "RTAS installed at", this->base );
	prom_print_hex( ", size", this->size );
	prom_puts( "\n\r" );
	prom_print_hex( "RTAS entry", this->entry );
	prom_puts( "\n\r" );
    }
}


bool rtas_t::get_token( const char *service, u32_t *token )
{
    u32_t len;
    char *data;

    if (rtas_dev->get_prop( service, &data, &len ))
    {
	if (len == sizeof(u32_t))
	{
	    *token = *(u32_t*)data;
	    return true;
	}
    }
    return false;
}

/* These must be static global */
static rtas_args_t rtas_args;

word_t rtas_t::rtas_call( u32_t token, u32_t nargs, u32_t nret, word_t *outputs, ... )
{
    va_list list;
    rtas_args.setup( token, nargs, nret );

    va_start(list, outputs);
    for (word_t i = 0; i < nargs; i++)
	rtas_args.set_arg( i, (rtas_arg_t)(va_arg(list, word_t) & 0xffffffff));
    va_end(list);

    this->lock.lock();

    __call_rtas((void *)virt_to_phys(&rtas_args));

    this->lock.unlock();

    if (nret > 1 && outputs != NULL)
        for (word_t i = 0; i < nret-1; ++i)
	    outputs[i] = rtas_args.get_ret(i+1);

    return (word_t)((nret > 0) ? rtas_args.get_ret(0) : 0);
}

word_t rtas_t::rtas_call( word_t *data, u32_t token, u32_t nargs, u32_t nret )
{
    rtas_args.setup( token, nargs, nret );

    for (word_t i = 0; i < nargs; i++)
	rtas_args.set_arg( i, data[i] & 0xffffffff );

    this->lock.lock();

    __call_rtas((void *)virt_to_phys(&rtas_args));

    this->lock.unlock();

    if (nret > 1 )
        for (word_t i = 0; i < nret-1; ++i)
	    data[nargs + i] = rtas_args.get_ret(i+1);

    return (word_t)((nret > 0) ? rtas_args.get_ret(0) : 0);
}

void rtas_t::machine_restart( void )
{
    u32_t reboot_token;

    get_token( "system-reboot", &reboot_token );
    rtas_call( reboot_token, 0, 1, NULL );

    /* We should never get here */
    asm volatile (".long 0x00000000;");
}

void rtas_t::machine_power_off( void )
{
    u32_t poweroff_token;

    get_token( "power-off", &poweroff_token );
    rtas_call( poweroff_token, 0, 1, NULL );

    /* We should never get here */
    asm volatile (".long 0x00000000;");
}

void rtas_t::machine_halt( void )
{
    u32_t poweroff_token;

    get_token( "power-off", &poweroff_token );
    rtas_call( poweroff_token, 0, 1, NULL );

    /* We should never get here */
    asm volatile (".long 0x00000000;");
}

SECTION(".init") bool rtas_t::try_location( word_t phys_start, word_t size )
{
    kernel_interface_page_t *kip = get_kip();
    word_t phys_end = phys_start + size;

    if ((word_t)get_kip()->sigma0.mem_region.high > phys_start)
	return false;
    if ((word_t)get_kip()->sigma1.mem_region.high > phys_start)
	return false;
    if ((word_t)get_kip()->root_server.mem_region.high > phys_start)
	return false;


    // Walk through the KIP's memory descriptors and search for any
    // reserved memory regions that collide with our intended memory
    // allocation.
    for( word_t i = 0; i < kip->memory_info.get_num_descriptors(); i++ )
    {
	memdesc_t *mdesc = kip->memory_info.get_memdesc( i );
	if( (mdesc->type() == memdesc_t::conventional) || mdesc->is_virtual() )
	    continue;

	word_t low = (word_t)mdesc->low();
	word_t high = (word_t)mdesc->high();

	if( (phys_start < low) && (phys_end > high) )
	    return false;
	if( (phys_start >= low) && (phys_start < high) )
	    return false;
	if( (phys_end > low) && (phys_end <= high) )
	    return false;
    }

    return true;
}

