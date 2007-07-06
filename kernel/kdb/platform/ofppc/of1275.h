/****************************************************************************
 *
 * Copyright (C) 2002-2003, Karlsruhe University
 *
 * File path:	kdb/platform/ofppc/of1275.h
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
 * $Id: of1275.h,v 1.4 2003/09/24 19:05:20 skoglund Exp $
 *
 ***************************************************************************/

#ifndef __KDB__PLATFORM__OFPPC__OF1275_H__
#define __KDB__PLATFORM__OFPPC__OF1275_H__

#if defined(CONFIG_KDB_CONS_OF1275)

#include <debug.h>
#include <sync.h>

typedef word_t (*of1275_ci_entry_t)( void * );
typedef word_t of1275_phandle_t;
typedef word_t of1275_ihandle_t;

#define OF1275_INVALID_PHANDLE	((of1275_phandle_t)-1)

class of1275_client_interface_t
{
protected:
    of1275_ci_entry_t entry;
    of1275_phandle_t stdout;
    of1275_phandle_t stdin;

    spinlock_t ci_lock;

    union 
    {
	char shared[512];
	struct {
	    const char *service;
	    int nargs;
	    int nret;
	    of1275_phandle_t phandle;
	    char *name;
	    void *buf;
	    int buflen;
	    int size;
	} get_prop;
	struct {
	    char *service;
	    int nargs;
	    int nret;
	    char *name;
	    of1275_phandle_t phandle;
	} find_device;
	struct {
	    char *service;
	    int nargs;
	    int nret;
	    of1275_phandle_t phandle;
	    void *buf;
	    int len;
	    int actual;
	} write;
	struct {
	    char *service;
	    int nargs;
	    int nret;
	    of1275_phandle_t phandle;
	    void *buf;
	    int len;
	    int actual;
	} read;
	struct {
	    char *service;
	    int nargs;
	    int nret;
	    char *forth;
	    int result;
	} interpret;
	struct {
	    const char *service;
	    int nargs;
	    int nret;
	} simple;
    } args;

    word_t ci( void *params );

public:
    of1275_phandle_t get_stdout() { return this->stdout; }
    of1275_phandle_t get_stdin()  { return this->stdin;  }

    void init( word_t entry );
    of1275_phandle_t find_device( const char *name );
    int get_prop( of1275_phandle_t phandle, const char *name, void *buf, int buflen );
    int write( of1275_phandle_t phandle, const void *buf, int len );
    int read( of1275_phandle_t phandle, void *buf, int len );

    void exit();
    void quiesce();
    void enter();
    int interpret( const char *forth );
};

INLINE of1275_client_interface_t *get_of1275_ci()
{
    extern of1275_client_interface_t of1275_ci;
    return &of1275_ci;
}

#endif	/* CONFIG_KDB_CONS_OF1275 */

#endif /* __KDB__PLATFORM__OFPPC__OF1275_H__ */
