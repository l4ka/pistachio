/****************************************************************************
 *
 * Copyright (C) 2003-2004,  National ICT Australia (NICTA)
 *
 * File path:	arch/powerpc64/of1275.cc
 * Description:	OpenFirmware Interface.
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
 * $Id: of1275.cc,v 1.6 2005/01/19 13:54:33 cvansch Exp $
 *
 ***************************************************************************/

#include INC_ARCH(of1275.h)
#include INC_ARCH(string.h)
#include INC_ARCH(msr.h)
#include <stdarg.h>
#include <debug.h>

/* Open Firmware class */
of1275_client_interface_t of1275 SECTION (".init.data");

/* This function performs the actual call to Open Firmware.
 * For now, we must be in real-mode to make this call.
 */
s32_t SECTION (".init")
of1275_client_interface_t::call( void *params )
{
    register s32_t result;
    s32_t args = (word_t)params;
    asm volatile (
	"mtctr	%1; "
	"mr	3, %2;"
	"stdu	1, -1024(1);"	/* XXX save and update stack */
	"std	2,  8(1);"
	"std	3, 16(1);"
	"std	4, 24(1);"
	"std	5, 32(1);"
	"std	6, 40(1);"
	"std	7, 48(1);"
	"std	8, 56(1);"
	"std	9, 64(1);"
	"std	10,72(1);"
	"std	11,80(1);"
	"std	12,88(1);"
	"std	13,96(1);"
	"std	14,104(1);"
	"std	15,112(1);"
	"std	16,120(1);"
	"std	17,128(1);"
	"std	18,136(1);"
	"std	19,144(1);"
	"std	20,152(1);"
	"std	21,160(1);"
	"std	22,168(1);"
	"std	23,176(1);"
	"std	24,184(1);"
	"std	25,192(1);"
	"std	26,200(1);"
	"std	27,208(1);"
	"std	28,216(1);"
	"std	29,224(1);"
	"std	30,232(1);"
	"std	31,240(1);"

	"mfcr   4;"
	"std    4,248(1);"
	"mfctr  5;"
	"std    5,256(1);"
	"mfxer  6;"
	"std    6,264(1);"
	"mfdar  7;"
	"std    7,272(1);"
	"mfdsisr 8;"
	"std    8,280(1);"
	"mfsrr0 9;"
	"std    9,288(1);"
	"mfsrr1 10;"
	"std    10,296(1);"
	"mfmsr  11;"
	"std    11,304(1);"

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

	"bctrl;	    "			/* Jump to OpenFirmware */

	"mfsprg  1, 2;"			/* Restore the stack pointer */
	"ld      6,304(1);"		/* Restore the MSR */
	"mtmsrd  6;"
	"isync;"

	"ld	2, 8(1);"		/* Restore the TOC */
	"ld     13, 96(1);"            /* Restore current */
	/* Restore the non-volatiles */
	"ld	14,104(1);"
	"ld	15,112(1);"
	"ld	16,120(1);"
	"ld	17,128(1);"
	"ld	18,136(1);"
	"ld	19,144(1);"
	"ld	20,152(1);"
	"ld	21,160(1);"
	"ld	22,168(1);"
	"ld	23,176(1);"
	"ld	24,184(1);"
	"ld	25,192(1);"
	"ld	26,200(1);"
	"ld	27,208(1);"
	"ld	28,216(1);"
	"ld	29,224(1);"
	"ld	30,232(1);"
	"ld	31,240(1);"
															
	"ld      4,248(1);"
	"mtcr    4;"
	"ld      5,256(1);"
	"mtctr   5;"
	"ld      6,264(1);"
	"mtxer   6;"
	"ld      7,272(1);"
	"mtdar   7;"
	"ld      8,280(1);"
	"mtdsisr 8;"
	"ld      9,288(1);"
	"mtsrr0  9;"
	"ld      10,296(1);"
	"mtsrr1  10;"

	"addi	1, 1, 1024;"	/* XXX fix stack */
	: "=r" (result)
	: "r" (entry),
	  "r" (args)
	: "lr", "memory"
    );
    return result;
}

/* Initialise the Open Firmware client interface.
 * Grab stdin and stdout from openfirmware.
 * We are running with no relocation and the device tree copy has
 * not been initialised yet.
 */
void SECTION (".init")
of1275_client_interface_t::init( word_t entry )
{
    this->entry = entry;
    this->ci_lock.init();

    this->stdout = OF1275_INVALID_PHANDLE;
    this->stdin = OF1275_INVALID_PHANDLE;

    of1275_phandle_t chosen = this->find_device( "/chosen" );
    if( chosen == OF1275_INVALID_PHANDLE )
	return;

    this->get_prop( chosen, "stdout", &this->stdout, sizeof(of1275_phandle_t) );
    this->get_prop( chosen, "stdin", &this->stdin, sizeof(of1275_phandle_t) );

    char init_msg[] = "\n\r" TXT_FG_RED "L4 PPC64 - Open Firmware Init" TXT_NORMAL "\n\r";
    this->write(this->stdout, init_msg, sizeof(init_msg));
}

/* Find a device in the openfirmware tree
 */
of1275_phandle_t SECTION (".init")
of1275_client_interface_t::find_device( const char *name )
{
    int namelen = strlen(name) + 1;
    
    // Is the request too large?
    if( (sizeof(this->args.find_device) + namelen) > sizeof(this->args.shared) )
	return OF1275_INVALID_PHANDLE;

    this->ci_lock.lock();

    // Install all parameters in the shared data area.
    this->args.find_device.service = (word_t) ("finddevice");
    this->args.find_device.nargs = 1;
    this->args.find_device.nret = 1;
    this->args.find_device.name = (word_t)(this->args.shared + sizeof(this->args.find_device));
    this->args.find_device.phandle = OF1275_INVALID_PHANDLE;
    sstrncpy( (char *)(word_t)this->args.find_device.name, name, namelen );

    // Invoke OF.
    this->call( &this->args.find_device );

    of1275_phandle_t ret = this->args.find_device.phandle;

    this->ci_lock.unlock();
    return ret;
}

/* Request the value of a property
 */
s32_t SECTION (".init")
of1275_client_interface_t::get_prop( of1275_phandle_t phandle,
		const char *name, void *buf, s32_t buflen )
{
    int ret = -1;

    this->ci_lock.lock();

    // Initialize the argument structure, fitting all data within our
    // shared memory region.
    int namelen = strlen(name) + 1;
    this->args.get_prop.service = (word_t) ("getprop");
    this->args.get_prop.nargs = 4;
    this->args.get_prop.nret = 1;
    this->args.get_prop.phandle = phandle;
    this->args.get_prop.name = (word_t)(this->args.shared + sizeof(this->args.get_prop));
    this->args.get_prop.buf = (word_t)addr_align_up((char *)(word_t)this->args.get_prop.name + namelen, sizeof(word_t) );
    this->args.get_prop.buflen = buflen;
    this->args.get_prop.size = ret;

    // If the data fits, then invoke Open Firmware.
    word_t tot = (word_t)this->args.get_prop.buf - (s32_t)(word_t)&this->args.shared + 
	buflen;
    if( tot <= sizeof(this->args.shared) )
    {
	// Copy the name into the shared buffer.
	sstrncpy( (char *)(word_t)this->args.get_prop.name, name, namelen );

	this->call( &this->args.get_prop ); // Call OF.

	if( (this->args.get_prop.size > -1) && 
		(this->args.get_prop.size <= buflen) )
	{
	    // Copy the data into the outgoing buffer.
	    memcpy( buf, (char*)(word_t) this->args.get_prop.buf, this->args.get_prop.size );
	    ret = this->args.get_prop.size;
	}
    }

    this->ci_lock.unlock();
    return ret;
}

/* Open an Open Firmware device
 */
of1275_ihandle_t SECTION (".init") of1275_client_interface_t::open( const char *name )
{
    int namelen = strlen(name) + 1;
    
    // Is the request too large?
    if( (sizeof(this->args.open) + namelen) > sizeof(this->args.shared) )
	return OF1275_INVALID_PHANDLE;

    this->ci_lock.lock();

    this->args.open.service = (word_t) ("open");
    this->args.open.nargs = 1;
    this->args.open.nret = 1;
    this->args.open.name = (word_t)(this->args.shared + sizeof(this->args.open));
    this->args.open.ihandle = OF1275_INVALID_PHANDLE;

    // Copy the name into the shared buffer.
    sstrncpy( (char *)(word_t)this->args.open.name, name, namelen );

    this->call( &this->args.open );
    of1275_ihandle_t ret = this->args.open.ihandle;

    this->ci_lock.unlock();
    return ret;
}

/* Write to an Open Firmware file descriptor
 */
s32_t SECTION (".init")
of1275_client_interface_t::write( of1275_phandle_t phandle,
		const void *buf, s32_t len )
{
    int ret = -1;

    // Adjust the amount of data to write as necessary.
    if( (len + sizeof(this->args.write)) > sizeof(this->args.shared) )
	len = sizeof(this->args.shared) - sizeof(this->args.write);

    this->ci_lock.lock();

    // Initialize the argument structure, fitting all data within our
    // shared data region.
    this->args.write.service = (word_t) ("write");
    this->args.write.nargs = 3;
    this->args.write.nret = 1;
    this->args.write.phandle = phandle;
    this->args.write.buf = (word_t)(this->args.shared + sizeof(this->args.write));
    this->args.write.len = len;
    this->args.write.actual = -1;
    memcpy( (char *)(word_t)this->args.write.buf, buf, len );

    // Invoke OF.
    this->call( &this->args.write );
    ret = this->args.write.actual;

    this->ci_lock.unlock();
    return ret;
}

/* Read from an OpenFirmware file descriptor
 */
s32_t SECTION (".init")
of1275_client_interface_t::read( of1275_phandle_t phandle, void *buf, s32_t len )
{
    int ret = -1;

    this->ci_lock.lock();

    // Adjust the size of the requested data to fit our shared buffer size.
    if( (len + sizeof(this->args.read)) > sizeof(this->args.shared) )
	len = sizeof(this->args.shared) - sizeof(this->args.read);

    // Initialize the argument structure, fitting all data within our
    // shared data region.
    this->args.read.service = (word_t) ("read");
    this->args.read.nargs = 3;
    this->args.read.nret = 1;
    this->args.read.phandle = phandle;
    this->args.read.buf = (word_t)(this->args.shared + sizeof(this->args.read));
    this->args.read.len = len;
    this->args.read.actual = -1;

    // Call OF.
    this->call( &this->args.read );

    // If possible, copy the input data to the outgoing buffer.
    ret = this->args.read.actual;
    if( (ret >= 0) && (ret <= len) )
	memcpy( buf, (char *)(word_t)this->args.read.buf, len );
    else
	ret = -1;

    this->ci_lock.unlock();
    return ret;
}

/* Call an OpenFirmware method
 */
void SECTION (".init")
of1275_client_interface_t::call_method( of1275_phandle_t phandle, const char *method,
		s32_t *results, int nret, int nargs, ...)
{
    va_list args;
    int namelen = strlen(method) + 1;
    
    // Is the request too large?
    if( (sizeof(this->args.call) + namelen) > sizeof(this->args.shared) )
	return;
    // Is there enough space for the args and rets?
    if( ((int)sizeof(this->args.call.args) < (nargs+nret)))
	return;

    this->ci_lock.lock();

    // Pack the arguments into our shared data region.
    this->args.call.service = (word_t) ("call-method");
    this->args.call.nargs = nargs+2;
    this->args.call.nret = nret;
    this->args.call.method = (word_t)(this->args.shared + sizeof(this->args.call));
    this->args.call.phandle = phandle;

    // Copy the name into the shared buffer.
    sstrncpy( (char *)(word_t)this->args.call.method, method, namelen );

    va_start(args, nargs);
    for (int i=0; i < nargs; i++)
    {
	this->args.call.args[i] = va_arg(args, s32_t);
    }
    va_end(args);

    // Invoke OF.
    this->call( &this->args.call );

    for (int i=0; i < nret; i++)
    {
	results[i] = this->args.call.args[nargs+i];
    }

    this->ci_lock.unlock();
}

/* Exit to OpenFirmware
 */
void SECTION (".init")
of1275_client_interface_t::exit()
{
    this->ci_lock.lock();

    // Pack the arguments into our shared data region.
    this->args.simple.service = (word_t) ("exit");
    this->args.simple.nargs = 0;
    this->args.simple.nret = 0;

    // Invoke OF.
    this->call( &this->args.simple );

    // Hopefully the Open Firmware will never return to us ...
    this->ci_lock.unlock();
}

/* Convert an instance number to a package
 */
of1275_phandle_t SECTION (".init")
of1275_client_interface_t::instance_to_package( s32_t val )
{
    of1275_phandle_t ret;
    this->ci_lock.lock();

    // Pack the arguments into our shared data region.
    this->args.simple_arg.service = (word_t) ("instance-to-package");
    this->args.simple_arg.nargs = 1;
    this->args.simple_arg.nret = 1;
    this->args.simple_arg.val = val;
    this->args.simple_arg.ret = -1;

    // Invoke OF.
    this->call( &this->args.simple_arg );

    ret = this->args.simple_arg.ret;

    this->ci_lock.unlock();

    return ret;
}

/* Claim memory from Open Firmware
 */
s32_t SECTION (".init")
of1275_client_interface_t::claim( addr_t virt, u32_t size, u32_t align )
{
    int ret = 0;
    word_t msr = ppc64_get_msr();

    if ((msr & MSR_IR) && (msr & MSR_DR))
    {
	this->ci_lock.lock();

	// Initialize the argument structure, fitting all data within our
	// shared data region.
	this->args.claim.service = (word_t) ("claim");
	this->args.claim.nargs = 3;
	this->args.claim.nret = 1;
	this->args.claim.virt = (word_t)virt;
	this->args.claim.size = size;
	this->args.claim.align = align;
	this->args.claim.retval = 0;

	// Invoke OF.
	this->call( &this->args.write );
	ret = this->args.claim.retval;

	this->ci_lock.unlock();
    }
    return ret;
}

/* OpenFirmware put string
 */
void SECTION (".init")
of1275_client_interface_t::puts(char * str, int len)
{
    this->write(this->stdout, str, len);
}

/*
 */
void SECTION (".init")
of1275_client_interface_t::quiesce()
{
}

/*
 */
void SECTION (".init")
of1275_client_interface_t::enter()
{
}

/*
 */
s32_t SECTION (".init")
interpret( const char *forth )
{
    return 0;
}


/* Exit to OpenFirmware
 */
void SECTION (".init")
prom_exit( char * msg )
{
    of1275.puts( msg, strlen(msg) );
    of1275.puts( "\n\r", 2 );
    of1275.exit();
}


/* Put string
 */
void SECTION (".init")
prom_puts( char * msg )
{
    of1275_client_interface_t *of = get_of1275();
    of = PTRRELOC(of);

    of->puts( msg, strlen(PTRRELOC(msg)) );
}

/* Print Hex
 */
void SECTION (".init")
prom_print_hex( char *s, word_t val )
{
    char buf[20];

    hex( val, buf );
    prom_puts( s );
    prom_puts( ": " );
    prom_puts( (char*)buf );
}

