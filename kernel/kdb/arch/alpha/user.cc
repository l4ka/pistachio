/*********************************************************************
 *                
 * Copyright (C) 2002, 2004,  University of New South Wales
 *                
 * File path:     kdb/arch/alpha/user.cc
 * Description:   User debugger support
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
 * $Id: user.cc,v 1.5 2004/03/17 19:12:23 skoglund Exp $
 *                
 ********************************************************************/

#include <debug.h>
#include <kdb/console.h>

/* sjw (26/08/2002): Put these somewhere decent */
#define KDB_PUTC 0
#define KDB_GETC 1
#define KDB_ENTER 2

/* this is invoked by the enterIF handler when the user does a gentrap palcall */
word_t handle_user_trap(word_t type, word_t arg)
{
    word_t dummy;

    switch(type) {
    case KDB_PUTC:
	putc(arg);
	return 0;

    case KDB_GETC:
	return getc();

    case KDB_ENTER:
	printf("%s\n", arg);
	kdebug_entry(&dummy);
	return 0;
	
    default:
	break;
    }

    return -1UL;
}
