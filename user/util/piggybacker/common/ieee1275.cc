/****************************************************************************
 *
 * Copyright (C) 2002-2003, Karlsruhe University
 *
 * File path:	piggybacker/common/ieee1275.cc
 * Description:	Provides access to the Open Firmware client access callback.
 * 		Ripped out of the kernel.
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
 * $Id: ieee1275.cc,v 1.5 2005/01/19 14:01:00 cvansch Exp $
 *
 ***************************************************************************/

#include <piggybacker/ieee1275.h>
#include <piggybacker/macros.h>

word_t call_addr = 0;


__unused prom_handle_t prom_stdout = INVALID_PROM_HANDLE;
__unused prom_handle_t prom_stdin = INVALID_PROM_HANDLE;
__unused prom_handle_t prom_chosen = INVALID_PROM_HANDLE;
__unused prom_handle_t prom_options = INVALID_PROM_HANDLE;
__unused prom_handle_t prom_memory = INVALID_PROM_HANDLE;
__unused prom_handle_t prom_mmu = INVALID_PROM_HANDLE;

#define ToPointer32(x)	(unsigned int)((unsigned long)x & 0xffffffff)

int prom_get_path( prom_handle_t phandle, const char *service, char *path, int pathlen )
{
	struct {
		unsigned int service;
		int nargs;
		int nret;
		unsigned int phandle;
		unsigned int path;
		int pathlen;
		int size;
	} args;

	args.service = ToPointer32(service);
	args.nargs = 3;
	args.nret = 1;
	args.phandle = ToPointer32(phandle);
	args.path = ToPointer32(path);
	args.pathlen = pathlen;
	args.size = -1;
	prom_entry( &args );
	if( args.size > -1 )
		path[args.size] = '\0';
	return args.size;
}

int prom_package_to_path( prom_handle_t phandle, char *path, int pathlen )
{
	return prom_get_path( phandle, "package-to-path", path, pathlen );
}

int prom_instance_to_path( prom_handle_t ihandle, char *path, int pathlen )
{
	return prom_get_path( ihandle, "instance-to-path", path, pathlen );
}

prom_handle_t prom_instance_to_package( prom_handle_t ihandle )
{
    struct {
	unsigned int service;
	int nargs;
	int nret;
	unsigned int ihandle;
	int phandle;
    } args;

    args.service = ToPointer32("instance-to-package");
    args.nargs = 1;
    args.nret = 1;
    args.ihandle = ToPointer32(ihandle);
    args.phandle = ToPointer32(INVALID_PROM_HANDLE);
    prom_entry( &args );
    return (prom_handle_t)(word_t)args.phandle;
}

int prom_next_prop( prom_handle_t node, const char *prev_name, char *name )
{
	struct {
		unsigned int service;
		int nargs;
		int nret;
		unsigned int node;
		unsigned int prev_name;
		unsigned int name;
		int flag;
	} args;

	args.service = ToPointer32("nextprop");
	args.nargs = 3;
	args.nret = 1;
	args.node = ToPointer32(node);
	args.prev_name = ToPointer32(prev_name);
	args.name = ToPointer32(name);
	args.flag = -1;
	prom_entry( &args );
	return args.flag;
}

int prom_read( prom_handle_t phandle, void *buf, int len )
{
	struct {
		unsigned int service;
		int nargs;
		int nret;
		unsigned int phandle;
		unsigned int buf;
		int len;
		int actual;
	} args;

	args.service = ToPointer32("read");
	args.nargs = 3;
	args.nret = 1;
	args.phandle = ToPointer32(phandle);
	args.buf = ToPointer32(buf);
	args.len = len;
	args.actual = -1;
	prom_entry( &args );
	return args.actual;
}

int prom_write( prom_handle_t phandle, const void *buf, int len )
{
	struct {
		unsigned int service;
		int nargs;
		int nret;
		unsigned int phandle;
		unsigned int buf;
		int len;
		int actual;
	} args;

	args.service = ToPointer32("write");
	args.nargs = 3;
	args.nret = 1;
	args.phandle = ToPointer32(phandle);
	args.buf = ToPointer32(buf);
	args.len = len;
	args.actual = -1;
	prom_entry( &args );
	return args.actual;
}

void prom_puts( const char *msg )
{
        unsigned start, current, len;
	char nl[] = "\r\n";

	current = 0;
	do {
		start = current;
		/*  Look for a newline.  */
		while( msg[current] && (msg[current] != '\n') )
			current++;

		/*  If we have a string, print it.  */
		len = current-start;
		if( len > 0 )
			prom_write( prom_stdout, msg, len );
		/*  Print a newline.  */
		prom_write( prom_stdout, nl, sizeof(nl) );

		/*  Skip the newline.  */
		if( msg[current] == '\n' )
			current++;
	} while( msg[current] );
}

prom_handle_t prom_find_device( const char *name )
{
	struct {
		unsigned int service;
		int nargs;
		int nret;
		unsigned int name;
		int phandle;
	} args;

	args.service = ToPointer32("finddevice");
	args.nargs = 1;
	args.nret = 1;
	args.name = ToPointer32(name);
	args.phandle = ToPointer32(INVALID_PROM_HANDLE);
	prom_entry( &args );
	return (prom_handle_t)(word_t)args.phandle;
}

int prom_get_prop( prom_handle_t phandle, const char *name, void *buf, int buflen )
{
	struct {
		unsigned int service;
		int nargs;
		int nret;
		unsigned int phandle;
		unsigned int name;
		unsigned int buf;
		int buflen;
		int size;
	} args;

	args.service = ToPointer32("getprop");
	args.nargs = 4;
	args.nret = 1;
	args.phandle = ToPointer32(phandle);
	args.name = ToPointer32(name);
	for (int i=0; i<buflen; i++)
	    ((char*)buf)[i] = 0;
	args.buf = ToPointer32(buf);
	args.buflen = buflen;
	args.size = -1;
	prom_entry( &args );
	return args.size;
}

int prom_get_prop_len( prom_handle_t phandle, const char *name )
{
    struct {
		unsigned int service;
		int nargs;
		int nret;
		unsigned int phandle;
		unsigned int name;
		int size;
	} args;

	args.service = ToPointer32("getproplen");
	args.nargs = 2;
	args.nret = 1;
	args.phandle = ToPointer32(phandle);
	args.name = ToPointer32(name);
	args.size = -1;
	prom_entry( &args );
	return args.size;
}

prom_handle_t prom_nav_tree( prom_handle_t node, const char *which )
{
	struct {
		unsigned int service;
		int nargs;
		int nret;
		unsigned int node;
		int out;
	} args;

	args.service = ToPointer32(which);
	args.nargs = 1;
	args.nret = 1;
	args.node = ToPointer32(node);
	args.out = ToPointer32(INVALID_PROM_HANDLE);
	prom_entry( &args );
	if( args.out == 0 )
		args.out = ToPointer32(INVALID_PROM_HANDLE);
	return (prom_handle_t)(word_t)args.out;
}

__noreturn void prom_exit( void )
{
	struct {
		unsigned int service;
		int nargs;
		int nret;
	} args;

	args.service = ToPointer32("exit");
	args.nargs = 0;
	args.nret = 0;
	prom_entry( &args );

	/*  We should never reach this point.  */
	while( 1 ) ;
}

__noreturn void prom_fatal( const char *msg )
{
	char caption[] = "\r\nfatal error: ";

	if( prom_stdout != INVALID_PROM_HANDLE ) {
		prom_write( prom_stdout, caption, sizeof(caption) );
		prom_puts( msg );
	}
	prom_exit();
}

void prom_enter( void )
{
	struct {
		unsigned int service;
		int nargs;
		int nret;
	} args;

	args.service = ToPointer32("enter");
	args.nargs = 0;
	args.nret = 0;
	prom_entry( &args );
}

int prom_interpret( const char *forth )
{
	struct {
		unsigned int service;
		int nargs;
		int nret;
		unsigned int forth;
		int result;
	} args;

	args.service = ToPointer32("interpret");
	args.nargs = 1;
	args.nret = 1;
	args.forth = ToPointer32(forth);
	args.result = -1;
	prom_entry( &args );
	return args.result;
}

prom_handle_t prom_open( char *device )
{
    struct {
	unsigned int service;
	int nargs;
	int nret;
	unsigned int name;
	unsigned int handle;
    } args;

    args.service = ToPointer32("open");
    args.nargs = 1;
    args.nret = 1;
    args.name = ToPointer32(device);
    args.handle = ToPointer32(INVALID_PROM_HANDLE);
    prom_entry( &args );
    return (prom_handle_t)(word_t)args.handle;
}

word_t prom_instantiate_rtas( word_t rtas_base_address )
{
    prom_handle_t prom_rtas;

    prom_rtas = prom_open( "/rtas" );
    if( prom_rtas == INVALID_PROM_HANDLE )
	return 0;

    struct {
	unsigned int service;
	int nargs;
	int nret;
	unsigned int method;
	unsigned int handle;
	unsigned int rtas_base_address;
	int result;
	unsigned int rtas_call;
    } args;

    args.service = ToPointer32("call-method");
    args.nargs = 3;
    args.nret = 2;
    args.method = ToPointer32("instantiate-rtas");
    args.handle = ToPointer32(prom_rtas);
    args.rtas_base_address = ToPointer32(rtas_base_address);
    args.result = -1;
    args.rtas_call = 0;
    prom_entry( &args );
    return args.rtas_call;
}

void prom_get_rom_range( int ranges[], unsigned len, int *cnt )
{
	prom_handle_t rom;

	rom = prom_find_device( "/rom" );
	if( rom == INVALID_PROM_HANDLE )
		*cnt = 0;
	else {
		*cnt = prom_get_prop( rom, "ranges", ranges, len );
		*cnt /= sizeof(int);	// XXX CVS - was word_t ??
	}
}

void prom_quiesce( void )
{
	struct {
		unsigned int service;
		int nargs;
		int nret;
	} args;

	args.service = ToPointer32("quiesce");
	args.nargs = args.nret = 0;
	prom_entry( &args );
}

prom_callback_t prom_set_callback( prom_callback_t new_cb )
{
	struct {
		unsigned int service;
		int nargs;
		int nret;
		unsigned int new_cb;
		int old_cb;
	} args;

	args.service = ToPointer32("set-callback");
	args.nargs = 1;
	args.nret = 1;
	args.new_cb = ToPointer32(new_cb);
	args.old_cb = 0;
	prom_entry( &args );
	return (prom_callback_t)(word_t)args.old_cb;
}

/* Claim memory */
int prom_claim( unsigned long virt, unsigned long size,
		unsigned long align )
{
    L4_Word_t msr = get_msr();
    /* Only claim if in virtual mode */
    if ( (msr & MSR_IR) && (msr && MSR_DR) )
    {
	struct {
		unsigned int service;
		int nargs;
		int nret;
		unsigned int virt;
		unsigned int size;
		unsigned int align;
		int retval;
	} args;

	args.service = ToPointer32("claim");
	args.nargs = 3;
	args.nret = 1;
	args.virt = (unsigned int)virt;
	args.size = (unsigned int)size;
	args.align = (unsigned int)align;
	args.retval = 0;
	prom_entry( &args );

	if (args.retval == -1)
	    prom_puts("claim error\n");

	return args.retval;
    }
    return virt;
}

/* Map memory */
int prom_map( void *phys, void *virt, L4_Word32_t size )
{
	L4_Word_t msr = get_msr();
	/* Only map if in virtual mode */
	if ( (msr & MSR_IR) && (msr && MSR_DR) )
	{
		struct {
			unsigned int service;
			int nargs;
			int nret;
			unsigned int method;
	    unsigned int handle;
	    int mode;
	    unsigned int size;
	    unsigned int virt;
	    unsigned int phys;
	    int result;
	    int retval;
	} args;

	args.service = ToPointer32("call-method");
	args.nargs = 6;
	args.nret = 2;
	args.method = ToPointer32("map");
	args.handle = ToPointer32(prom_mmu);
	args.mode = -1;
	args.size = wrap_up( size, 0x1000 );	// XXX PAGE_SIZE
	args.virt = ToPointer32(virt);
	args.phys = ToPointer32(phys);
	args.result = 0;
	args.retval = 0;
	prom_entry( &args );

	if (args.result != 0)
	    prom_puts("map error\n");

	prom_claim( (L4_Word_t)virt, wrap_up( size, 0x1000 ), 0 );  // XXX PAGE_SIZE

	return args.retval;
    }

    return 0;
}

int prom_unmap( void *phys, void *virt, L4_Word32_t size )
{
    L4_Word_t msr = get_msr();
    /* Only unmap if in virtual mode */
    if ( (msr & MSR_IR) && (msr && MSR_DR) )
    {
    }

    return 0;
}

/**************************************************************************/

void prom_init( word_t r5 )
{
	char path[256];
	int len;
	int temp;

	call_addr = (L4_Word32_t)r5;

	prom_chosen = prom_find_device( "/chosen" );
	if( prom_chosen == INVALID_PROM_HANDLE )
		prom_exit();

	if( prom_get_prop(prom_chosen, "stdout", &temp, 4) != 4 )
		prom_exit();
	prom_stdout = (word_t*)(word_t)temp;
	if( prom_get_prop(prom_chosen, "stdin", &temp, 4) != 4 )
		prom_fatal( "unable to open /chosen/stdin" );
	prom_stdin = (word_t*)(word_t)temp;
	if( prom_get_prop(prom_chosen, "memory", &temp, 4) != 4 )
		prom_fatal( "unable to open /chosen/memory" );
	prom_memory = (word_t*)(word_t)temp;
	if( prom_get_prop(prom_chosen, "mmu", &temp, 4) == 4 )
		prom_mmu = (word_t*)(word_t)temp;
	else
		prom_mmu = (void*)0;

	prom_options = prom_find_device( "/options" );
	if( prom_options == INVALID_PROM_HANDLE )
		prom_fatal( "unable to find /options" );

	len = prom_instance_to_path( prom_memory, path, sizeof(path) );
	if( len != -1 )
		prom_memory = prom_find_device( path );
	if( (len == -1) || (prom_memory == INVALID_PROM_HANDLE) )
		prom_fatal( "unable to open the memory node" );

}

void get_prom_range( word_t *start, word_t *size )
{
	prom_handle_t rom;
	int ranges[2];

	rom = prom_find_device( "/rom" );
	if( rom == INVALID_PROM_HANDLE )
		prom_fatal( "unable to open /rom" );

	if( prom_get_prop(rom, "ranges", ranges, sizeof(ranges)) !=
			sizeof(ranges) )
		prom_fatal( "unable to read /rom/ranges" );

	*start = ranges[0];
	*size = ranges[1];
}

