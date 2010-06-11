/*********************************************************************
 *                
 * Copyright (C) 2010,  Karlsruhe Institute of Technology
 *                
 * Filename:      powerpc-io.cc
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
#include <l4/sigma0.h>
#include <l4/kip.h>
#include <l4/types.h>
#include <l4/powerpc/kdebug.h>


extern "C" int __l4_getc( void );
extern "C" int getc( void ) __attribute__ ((weak, alias("__l4_getc")));

extern "C" void __l4_putc( int c );
extern "C" void putc( int c ) __attribute__ ((weak, alias("__l4_putc")));

#if defined CONFIG_HAVE_DEVICE_TREE
/****************************************************************************
 *
 *  Use this switch to avoid the lib go fishing for device trees (FDT/1275)
 *  Instead, declare a variable.
 *
 ***************************************************************************/
void *__l4_powerpc_device_tree = NULL; 
#endif

/****************************************************************************
 *
 *  Console I/O using UART or PSIM interface.
 *
 ***************************************************************************/
#if defined(CONFIG_COMPORT)
static int io_init( void )
{
    of1275_device_t *dev;
    char *alias;
    L4_Word_t *reg, len;

    
   /* 1275 tree */
    of1275_tree_t *of1275_tree =  L4_Sigma0_GetSpecial(OF1275_KIP_TYPE, NULL, 4096);
    
    if( of1275_tree == 0 )
	return 0;

    dev = of1275_tree->find( "/aliases" );
    if( dev == 0 )
	return 0;
    if( !dev->get_prop("com", &alias, &len) )
	return 0;

    dev = of1275_tree->find( alias );
    if( dev == 0 )
	return 0;
    if( !dev->get_prop("reg", (char **)&reg, &len) )
	return 0;

    if( (len != 3*sizeof(L4_Word_t)) || !reg[1] )
	return 0;

    /* Request the device page from sigma0.
     */

    L4_ThreadId_t sigma0 = L4_GlobalId( L4_ThreadIdUserBase(L4_GetKernelInterface()), 1);
    if( sigma0 == L4_Myself() )
	return 0;

    // Install it as the 2nd page in our address space.  
    // Hopefully it is free!
    L4_Fpage_t target = L4_Fpage( 4096, 4096 );	

    L4_Fpage_t fpage = L4_Fpage( reg[1], 4096 );
    fpage.X.rwx = L4_ReadWriteOnly;
    fpage = L4_Sigma0_GetPage( sigma0, fpage, target );
    if( L4_IsNilFpage(fpage) )
	return 0;

    __l4_com_registers = (char *)L4_Address(target);
    __l4_com_registers[3] = 0;	// Some initialization ...

    return 1;
}

extern "C" int __l4_getc( void )
{
    if( __l4_com_registers == 0 )
    {
	if( __l4_io_enabled )
	    return 0;
	if( !__l4_io_init() )
	    return 0;
    }

    return __l4_com_registers[0];
}

extern "C" void __l4_putc( int c )
{
    if( __l4_com_registers == 0 )
    {
	if( __l4_io_enabled )
	    return;
	if( !__l4_io_init() )
	    return;
    }

    __l4_com_registers[0] = c;
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
