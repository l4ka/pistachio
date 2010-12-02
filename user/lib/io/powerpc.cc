/*********************************************************************
 *                
 * Copyright (C) 2010,  Karlsruhe Institute of Technology
 *                
 * Filename:      powerpc.cc
 * Author:        Jan Stoess <stoess@kit.edu>
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
 ********************************************************************/
#include <config.h>
#include <l4/types.h>
#include <l4/types.h>
#include <l4/powerpc/kdebug.h>
#include <l4/space.h>

extern "C" int __l4_getc( void );
extern "C" int getc( void ) __attribute__ ((weak, alias("__l4_getc")));

extern "C" void __l4_putc( int c );
extern "C" void putc( int c ) __attribute__ ((weak, alias("__l4_putc")));

/****************************************************************************
 *
 *  Console I/O using UART or PSIM interface.
 *
 ***************************************************************************/
#if defined(CONFIG_COMPORT)

#include <l4/sigma0.h>
#include "powerpc.h"
#include "fdt.h"
#include "1275tree.h"

static volatile L4_Word8_t *comport = CONFIG_COMPORT;

#define DTREE_KIP_SUBTYPE	0xf
#define DTREE_KIP_TYPE	        (L4_BootLoaderSpecificMemoryType + (DTREE_KIP_SUBTYPE << 4))

#define SIGMA0_DEVICE_RELOC     0xf0000000
void *__l4_dtree = 0;
static L4_Word8_t __attribute__((aligned(4096))) comport_page[4096];
   

#define IER     (comport+1)
#define EIR     (comport+2)
#define LCR     (comport+3)
#define MCR     (comport+4)
#define LSR     (comport+5)
#define MSR     (comport+6)
#define DLLO    (comport+0)
#define DLHI    (comport+1)

static void io_init( void )
{
    static bool io_initialized = false;

    if (io_initialized)
        return;

    io_initialized = true;

#if CONFIG_COMPORT == 1
    /* PSIM via 1275 tree */
    char *alias;
    L4_Word_t *reg, len;
    of1275_device_t *dev;
    of1275_tree_t *of1275_tree =  (of1275_tree_t *) L4_Sigma0_GetSpecial(DTREE_KIP_TYPE, 0, 4096);

    if( of1275_tree == 0 )
        return;

    dev = of1275_tree->find( "/aliases" );
    if( dev == 0 )
        return;
    if( !dev->get_prop("com", &alias, &len) )
        return;

    dev = of1275_tree->find( alias );
    if( dev == 0 )
        return;
    if( !dev->get_prop("reg", (char **)&reg, &len) )
        return;

    if( (len != 3*sizeof(L4_Word_t)) || !reg[1] )
        return;

    /* 
     * Request the device page from sigma0.
     */
    L4_ThreadId_t sigma0 = L4_GlobalId( L4_ThreadIdUserBase(L4_GetKernelInterface()), 1);
    if( sigma0 == L4_Myself() )
        return;

    // Install it as the 2nd page in our address space.  
    // Hopefully it is free!
    L4_Fpage_t target = L4_Fpage( 4096, 4096 );	
#if 0
    L4_Fpage_t fpage = L4_Fpage( reg[1], 4096 );
    fpage.X.rwx = L4_ReadWriteOnly;
    fpage = L4_Sigma0_GetPage( sigma0, fpage, target );
#else
    L4_Fpage_t fpage = L4_Fpage(0x40000000,4096);
    fpage.X.rwx = L4_ReadWriteOnly;
    fpage = L4_Sigma0_GetPage( sigma0, fpage, 0x1, target );
#endif
    if( L4_IsNilFpage(fpage) )
        return;

    comport = (volatile L4_Word8_t *)L4_Address(target);
    comport[3] = 0;	// Some initialization ...

#else

#if CONFIG_COMPORT == 0
    
    /*  FDT  */
    fdt_property_t *prop;
    fdt_node_t *node;
    fdt_t *fdt;    
    bool io_direct = true; 
        
    if (!(__l4_dtree))
    {
        __l4_dtree = L4_Sigma0_GetSpecial(DTREE_KIP_TYPE, 0, 4096);
        io_direct = false;
    }

    if (!(fdt = (fdt_t *) __l4_dtree))
        return;

    
    if (!(fdt->is_valid()))
        return;
    
    if (!(node = fdt->find_subtree("/aliases")))
        return;

    if (! (prop = fdt->find_property_node(node, "serial0")) )
        return;


    if (!(node = fdt->find_subtree(prop->get_string())))
        return;

    if (! (prop = fdt->find_property_node(node, "virtual-reg")) )
        return;
 

    if (io_direct) 
    {
        comport = (volatile L4_Word8_t *) prop->get_word(0);
    }
    else
    {
        
        if (! (prop = fdt->find_property_node(node, "reg")) )
            return;

        L4_Word_t comport_ofs =  prop->get_word(0) & 4095;
        L4_Word_t comport_phys = SIGMA0_DEVICE_RELOC + comport_ofs;
        comport = comport_page + comport_ofs;
        
        
        L4_Flush(L4_Fpage( (L4_Word_t) comport, 4096) + L4_FullyAccessible);

        /* 
         * Request the device page from sigma0.
         */


        L4_ThreadId_t sigma0 = L4_GlobalId( L4_ThreadIdUserBase(L4_GetKernelInterface()), 1);
        if( sigma0 == L4_Myself() )
            return;
        
        // Hopefully it is free!
        L4_Fpage_t target = L4_Fpage( (L4_Word_t) comport, 4096 );	
#if 0
        L4_Fpage_t fpage = L4_Fpage( comport_phys, 4096 );
        fpage.X.rwx = L4_ReadWriteOnly;
        fpage = L4_Sigma0_GetPage( sigma0, fpage, target );
#else
        L4_Fpage_t fpage = L4_Fpage(0x40000200,4096);
        fpage.X.rwx = L4_ReadWriteOnly;
        fpage = L4_Sigma0_GetPage( sigma0, fpage, 0x1, target );
#endif
        if( L4_IsNilFpage(fpage) )
            return;

        
    }
#endif /* CONFIG_COMPORT == 0 */
    
    if (comport)
    {
        outb(LCR, 0x80);          /* select bank 1        */
        for (volatile int i = 10000000; i--; );
        outb(DLLO, (((115200/CONFIG_COMSPEED) >> 0) & 0x00FF));
        outb(DLHI, (((115200/CONFIG_COMSPEED) >> 8) & 0x00FF));
        outb(LCR, 0x03);          /* set 8,N,1            */
        outb(IER, 0x00);          /* disable interrupts   */
        outb(EIR, 0x07);          /* enable FIFOs */
        inb(IER);
        inb(EIR);
        inb(LCR);
        inb(MCR);
        inb(LSR);
        inb(MSR);

    }
#endif
}


extern "C" int __l4_getc( void )
{
    io_init();
    
    if ( comport )
    {
#if CONFIG_COMPORT == 1
            return comport[0];
#else
            while ((inb(comport+5) & 0x01) == 0);
            return inb(comport);
    }
#endif
    return 0;
}


extern "C" void __l4_putc( int c )
{
    io_init();

    if ( comport )
    {
#if CONFIG_COMPORT == 1
        comport[0] = c;
#else
        while (!(inb(comport+5) & 0x20));
        outb(comport,c);
        while (!(inb(comport+5) & 0x40));
        if (c == '\n')
            __l4_putc('\r');
#endif
    }
}
#else	/* CONFIG_COMPORT */

extern "C" int __l4_getc()
{
    return L4_KDB_ReadChar_Blocked();
}

extern "C" void __l4_putc( int c )
{
    L4_KDB_PrintChar( c );
    if( c == '\n' )
	L4_KDB_PrintChar( '\r' );
}

#endif	/* CONFIG_COMPORT */
