/*********************************************************************
 *                
 * Copyright (C) 2006,  Karlsruhe University
 *                
 * File path:     glue/v4-mips32/init.cc
 * Description:   Initialization for MIPS32
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
 * $Id: init.cc,v 1.1 2006/02/23 21:07:46 ud3 Exp $
 *                
 ********************************************************************/

#include INC_API(kernelinterface.h)
#include INC_API(schedule.h)

#include INC_ARCH(cache.h)
#include INC_ARCH(tlb.h)

#include INC_GLUE(intctrl.h)
#include INC_GLUE(timer.h)

#include <kmemory.h>
#include <mapping.h>
#include <debug.h>

extern "C" void init_cpu();

EXTERN_KMEM_GROUP (kmem_mdb);
EXTERN_KMEM_GROUP (kmem_tcb);
EXTERN_KMEM_GROUP (kmem_space);


void init_mempool() {

#warning change this once memory descriptors are available

    /* missing MBI... 
     * kernel:		0x00000000 - 0x00400000 ( 4MB ) 
     * kmem-heap:   0x00400000 - 0x01400000 ( 16MB )
     * sigma0		0x02000000 - 0x02100000 ( 1MB )
     * sigma1		0x02100000 - 0x02200000 ( 1MB )
     * root 		0x02200000 - 0x02300000 ( 1MB )
     */

    /* Define the user's virtual address space. */
    get_kip ()->memory_info.insert( memdesc_t::conventional, 
                                    true,
                                    (addr_t)0, 
                                    (addr_t)(KSEG0_BASE)
	);

    /* Register physical memory. */
    get_kip ()->memory_info.insert( memdesc_t::conventional, 
                                    false, 
                                    (addr_t)0x02000000, 
                                    (addr_t)0x07000000 
	);


    /* some I/O ports */
    get_kip ()->memory_info.insert( memdesc_t::shared, 
                                    false, 
                                    (addr_t)0x18000000, 
                                    (addr_t)0x18001000 
	);
}


void get_processor_infos() {

    unsigned prid = (unsigned)read_c0_prid();
    unsigned config1 = (unsigned)read_c0_config1();
	
    unsigned tlb_size = ( ( config1 >> 25 ) & 0x3f ) + 1;
    unsigned da = ( ( config1 >> 7 ) & 0x7 );
    unsigned dl = ( ( config1 >> 10 ) & 0x7 );
    unsigned ds = ( ( config1 >> 13 ) & 0x7 );
    unsigned ia = ( ( config1 >> 16 ) & 0x7 );
    unsigned il = ( ( config1 >> 19 ) & 0x7 );
    unsigned is = ( ( config1 >> 22 ) & 0x7 );

    // processor
    switch( ( prid >> 8 ) & 0xff ) {
    case mips_cpu_t::mips_4Kc:
        get_mips_cpu()->set_procid( mips_cpu_t::mips_4Kc );
        break;
    case mips_cpu_t::mips_4Kp:
        get_mips_cpu()->set_procid( mips_cpu_t::mips_4Kp );
        break;
    default:
        get_mips_cpu()->set_procid( mips_cpu_t::mips_unknown );
        break;
    }

    // tlb size
    get_mips_cpu()->set_tlb_size( tlb_size );

	
    // data cache configuration
    switch( da ) {
    case mips_cpu_t::assoc_dm:
    case mips_cpu_t::assoc_2_way:
    case mips_cpu_t::assoc_3_way:
    case mips_cpu_t::assoc_4_way:
        get_mips_cpu()->set_dcache_assoc( (mips_cpu_t::cache_assoc_e)da );
        break;
    default:
        get_mips_cpu()->set_dcache_assoc( mips_cpu_t::assoc_unknown );
        break;
    }
    switch( dl ) {
    case mips_cpu_t::size_not_present:
    case mips_cpu_t::size_16_bytes:
        get_mips_cpu()->set_dcache_ls( (mips_cpu_t::cache_ls_e)dl );
        break;
    default:
        get_mips_cpu()->set_dcache_ls( mips_cpu_t::size_unknown );
        break;
    }
    switch( ds ) {
    case mips_cpu_t::spw_64:
    case mips_cpu_t::spw_128:
    case mips_cpu_t::spw_256:
        get_mips_cpu()->set_dcache_spw( (mips_cpu_t::cache_spw_e)ds );
        break;
    default:
        break;
    }


    // instruction cache configuration
    switch( ia ) {
    case mips_cpu_t::assoc_dm:
    case mips_cpu_t::assoc_2_way:
    case mips_cpu_t::assoc_3_way:
    case mips_cpu_t::assoc_4_way:
        get_mips_cpu()->set_icache_assoc( (mips_cpu_t::cache_assoc_e)ia );
        break;
    default:
        get_mips_cpu()->set_icache_assoc( mips_cpu_t::assoc_unknown );
        break;
    }
    switch( il ) {
    case mips_cpu_t::size_not_present:
    case mips_cpu_t::size_16_bytes:
        get_mips_cpu()->set_icache_ls( (mips_cpu_t::cache_ls_e)il );
        break;
    default:
        get_mips_cpu()->set_icache_ls( mips_cpu_t::size_unknown );
        break;
    }
    switch( is ) {
    case mips_cpu_t::spw_64:
    case mips_cpu_t::spw_128:
    case mips_cpu_t::spw_256:
        get_mips_cpu()->set_icache_spw( (mips_cpu_t::cache_spw_e)is );
        break;
    default:
        break;
    }


}

/**
 * Entry point from ASM into C kernel
 */
extern "C" void SECTION(".init") startup_system() {
    
    {
#warning clean up early serial port initialization
        // Early serial port initialization for Simics 3.x - clean up!
        #define inb(port) (*((volatile unsigned char*)(port)))
        #define outb( port, data ) ((*((volatile unsigned char*)(port))) = data)
        
        #define USART_0_BASE    (0xb80003f8)                    /* reflects PC hardware */
        
        #define COMPORT USART_0_BASE
        #define out_u8 outb
        #define in_u8 inb
        
        #define RATE 115200
        
        #define IER	(COMPORT+1)
        #define EIR	(COMPORT+2)
        #define LCR	(COMPORT+3)
        #define MCR	(COMPORT+4)
        #define LSR	(COMPORT+5)
        #define MSR	(COMPORT+6)
        #define DLLO	(COMPORT+0)
        #define DLHI	(COMPORT+1)
        
        out_u8(LCR, 0x80);		/* select bank 1	*/
        for (volatile int i = 10000000; i--; );
        out_u8(DLLO, (((115200/RATE) >> 0) & 0x00FF));
        out_u8(DLHI, (((115200/RATE) >> 8) & 0x00FF));
        out_u8(LCR, 0x03);		/* set 8,N,1		*/
        out_u8(IER, 0x00);		/* disable interrupts	*/
        out_u8(EIR, 0x07);		/* enable FIFOs	*/
        out_u8(IER, 0x01);		/* enable RX interrupts	*/
        in_u8(IER);
        in_u8(EIR);
        in_u8(LCR);
        in_u8(MCR);
        in_u8(LSR);
        in_u8(MSR);
    }


    get_processor_infos();
    
    init_cpu();
	

#if 0
    // Untested.  No cache model in Simics.
    init_cache();	
    
    // jump into cached memory region
    __asm__ __volatile__(
    	".set noreorder\n\t"	
    	"la $8, kseg0_ip\n\t"
    	"lui $9, 0x2000\n\t"
    	"sub $8, $8, $9\n\t"
    	"jr $8\n\t"
    	"nop\n\t"
    	"kseg0_ip:\n\t"
    	".set reorder"
    	:::"$8", "$9"
    );
#endif
    
    init_hello();
    
    get_tlb()->init();

    get_interrupt_ctrl()->init_intctrl();

#warning change this once memory descriptors are available
    /* missing MBI... 
     * kernel:		0x00000000 - 0x00400000 ( 4MB ) 
     * kmem-heap:   0x00400000 - 0x01400000 ( 16MB )
     * sigma0		0x02000000 - 0x02100000 ( 1MB )
     * sigma1		0x02100000 - 0x02200000 ( 1MB )
     * root 		0x02200000 - 0x02300000 ( 1MB )
     */
    kmem.init( (void*)0xA0400000, (void*)0xA1400000 ); // XXX


    /* initialize kernel interface page */
    get_kip()->memory_info.n = 0;
    init_mempool();

    get_kip()->init();


#warning change this once memory descriptors are available
    get_kip()->sigma0.sp = 0x0;
    get_kip()->sigma0.ip = 0x02000000;
    get_kip()->sigma0.mem_region.set((void*)0x17, (void*)0x17); // XXX

    get_kip()->root_server.sp = 0x0;
    get_kip()->root_server.ip = 0x02200000;
    get_kip()->root_server.mem_region.set((void*)0x17, (void*)0x17); // XXX
	

    init_mdb();

    get_asid_cache()->init();
    get_asid_cache()->set_valid(CONFIG_ASIDS_START, CONFIG_MAX_NUM_ASIDS-1);

    init_kernel_space();

    get_kip()->kdebug_init();

        
    /* initialize the scheduler */
    get_current_scheduler()->init();
        
    /* initialize the kernel's timer source */
    get_timer()->init_global();
    get_timer()->init_cpu();
        
        
    /* get the thing going - we should never return */
    get_current_scheduler()->start();
        
    ASSERT( !"should never reach here" );    
    spin_forever( 1 );
}
