/****************************************************************************
 *
 * Copyright (C) 2002-2003, Karlsruhe University
 *
 * File path:	kdb/platform/ofppc/of1275.cc
 * Description:	Routines for handling the Open Firmware client interface.
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
 * $Id: of1275.cc,v 1.4 2003/09/24 19:05:20 skoglund Exp $
 *
 ***************************************************************************/

#if defined(CONFIG_KDB_CONS_OF1275)

#include <debug.h>
#include <kdb/kdb.h>

#include INC_ARCH(string.h)

#include "ofppc.h"
#include "of1275.h"

DECLARE_CMD( cmd_dump_ci, platform, 'c', "of1275", "Open Firmware client interface" );

CMD(cmd_dump_ci, cg)
{
    printf( "stdout phandle %x\n", get_of1275_ci()->get_stdout() );
    printf( "stdin  phandle %x\n", get_of1275_ci()->get_stdin() );

    return CMD_NOQUIT;
}   


of1275_client_interface_t of1275_ci;

void of1275_client_interface_t::init( word_t entry )
{
    this->entry = (of1275_ci_entry_t)entry;
    this->ci_lock.init();

    this->stdout = OF1275_INVALID_PHANDLE;
    this->stdin = OF1275_INVALID_PHANDLE;

    of1275_phandle_t chosen = this->find_device( "/chosen" );
    if( chosen == OF1275_INVALID_PHANDLE )
	return;

    this->get_prop( chosen, "stdout", &this->stdout, sizeof(of1275_phandle_t) );
    this->get_prop( chosen, "stdin", &this->stdin, sizeof(of1275_phandle_t) );
}

word_t of1275_client_interface_t::ci( void *params )
{
    if( this->entry == NULL )
	return (word_t)-1;

    return get_of1275_space()->execute_of1275( this->entry, params );
}

of1275_phandle_t of1275_client_interface_t::find_device( const char *name )
{
    int namelen = strlen(name) + 1;
    
    // Is the request too large?
    if( (sizeof(this->args.find_device) + namelen) > sizeof(this->args.shared) )
	return OF1275_INVALID_PHANDLE;

    this->ci_lock.lock();

    // Install all parameters in the shared data area.
    this->args.find_device.service = "finddevice";
    this->args.find_device.nargs = 1;
    this->args.find_device.nret = 1;
    this->args.find_device.name = this->args.shared + sizeof(this->args.find_device);
    this->args.find_device.phandle = OF1275_INVALID_PHANDLE;
    sstrncpy( this->args.find_device.name, name, namelen );

    // Invoke OF.
    this->ci( &this->args.find_device );

    of1275_phandle_t ret = this->args.find_device.phandle;

    this->ci_lock.unlock();
    return ret;
}

int of1275_client_interface_t::get_prop( of1275_phandle_t phandle, 
	const char *name, void *buf, int buflen )
{
    int ret = -1;

    this->ci_lock.lock();

    // Initialize the argument structure, fitting all data within our
    // shared memory region.
    int namelen = strlen(name) + 1;
    this->args.get_prop.service = "getprop";
    this->args.get_prop.nargs = 4;
    this->args.get_prop.nret = 1;
    this->args.get_prop.phandle = phandle;
    this->args.get_prop.name = this->args.shared + sizeof(this->args.get_prop);
    this->args.get_prop.buf = addr_align_up(this->args.get_prop.name + namelen, sizeof(word_t) );
    this->args.get_prop.buflen = buflen;
    this->args.get_prop.size = ret;

    // If the data fits, then invoke Open Firmware.
    word_t tot = (word_t)this->args.get_prop.buf - (word_t)&this->args.shared + 
	buflen;
    if( tot <= sizeof(this->args.shared) )
    {
	// Copy the name into the shared buffer.
	sstrncpy( this->args.get_prop.name, name, namelen );

	this->ci( &this->args.get_prop ); // Call OF.

	if( (this->args.get_prop.size > -1) && 
		(this->args.get_prop.size <= buflen) )
	{
	    // Copy the data into the outgoing buffer.
	    memcpy( buf, this->args.get_prop.buf, this->args.get_prop.size );
	    ret = this->args.get_prop.size;
	}
    }

    this->ci_lock.unlock();
    return ret;
}

int of1275_client_interface_t::write( of1275_phandle_t phandle, 
	const void *buf, int len )
{
    int ret = -1;

    // Adjust the amount of data to write as necessary.
    if( (len + sizeof(this->args.write)) > sizeof(this->args.shared) )
	len = sizeof(this->args.shared) - sizeof(this->args.write);

    this->ci_lock.lock();

    // Initialize the argument structure, fitting all data within our
    // shared data region.
    this->args.write.service = "write";
    this->args.write.nargs = 3;
    this->args.write.nret = 1;
    this->args.write.phandle = phandle;
    this->args.write.buf = this->args.shared + sizeof(this->args.write);
    this->args.write.len = len;
    this->args.write.actual = -1;
    memcpy( this->args.write.buf, buf, len );

    // Invoke OF.
    this->ci( &this->args.write );
    ret = this->args.write.actual;

    this->ci_lock.unlock();
    return ret;
}

int of1275_client_interface_t::read( of1275_phandle_t phandle, 
	void *buf, int len )
{
    int ret = -1;

    this->ci_lock.lock();

    // Adjust the size of the requested data to fit our shared buffer size.
    if( (len + sizeof(this->args.read)) > sizeof(this->args.shared) )
	len = sizeof(this->args.shared) - sizeof(this->args.read);

    // Initialize the argument structure, fitting all data within our
    // shared data region.
    this->args.read.service = "read";
    this->args.read.nargs = 3;
    this->args.read.nret = 1;
    this->args.read.phandle = phandle;
    this->args.read.buf = this->args.shared + sizeof(this->args.read);
    this->args.read.len = len;
    this->args.read.actual = -1;

    // Call OF.
    this->ci( &this->args.read );

    // If possible, copy the input data to the outgoing buffer.
    ret = this->args.read.actual;
    if( (ret >= 0) && (ret <= len) )
	memcpy( buf, this->args.read.buf, len );
    else
	ret = -1;

    this->ci_lock.unlock();
    return ret;
}

void of1275_client_interface_t::exit()
{
    this->ci_lock.lock();

    // Pack the arguments into our shared data region.
    this->args.simple.service = "exit";
    this->args.simple.nargs = 0;
    this->args.simple.nret = 0;

    // Invoke OF.
    this->ci( &this->args.simple );

    // Hopefully the Open Firmware will never return to us ...
    this->ci_lock.unlock();
}

void of1275_client_interface_t::enter()
{
    this->ci_lock.lock();

    // Pack the arguments into our shared data region.
    this->args.simple.service = "enter";
    this->args.simple.nargs = 0;
    this->args.simple.nret = 0;

    // Invoke OF.
    this->ci( &this->args.simple );

    this->ci_lock.unlock();
}

int of1275_client_interface_t::interpret( const char *forth )
{
    int ret = -1;
    int forth_len = strlen(forth) + 1;

    if( (forth_len + sizeof(this->args.interpret)) > sizeof(this->args.shared))
	return ret;

    this->ci_lock.lock();

    // Pack the arguments into our shared data region.
    this->args.interpret.service = "interpret";
    this->args.interpret.nargs = 1;
    this->args.interpret.nret = 1;
    this->args.interpret.forth = this->args.shared + sizeof(this->args.interpret);
    this->args.interpret.result = -1;
    sstrncpy( this->args.interpret.forth, forth, forth_len );

    // Invoke OF
    this->ci( &this->args.interpret );
    ret = this->args.interpret.result;

    this->ci_lock.unlock();
    return ret;
}

void of1275_client_interface_t::quiesce()
{
    this->ci_lock.lock();

    // Pack the arguments into our shared data region.
    this->args.simple.service = "quiesce";
    this->args.simple.nargs = 0;
    this->args.simple.nret = 0;

    // Invoke OF.
    this->ci( &this->args.simple );

    // Prevent any further invocations of Open Firmware.
    this->entry = NULL;

    this->ci_lock.unlock();
}

#endif	/* CONFIG_KDB_CONS_OF1275 */

