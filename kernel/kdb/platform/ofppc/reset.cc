/****************************************************************************
 *
 * Copyright (C) 2002, Karlsruhe University
 *
 * File path:	kdb/platform/ofppc/reset.cc
 * Description:	Dump info about the register state.
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
 * $Id: reset.cc,v 1.6 2003/12/11 13:08:35 joshua Exp $
 *
 ***************************************************************************/

#include <debug.h>
#include <kdb/kdb.h>

#include INC_ARCH(pvr.h)

#include "of1275.h"

DECLARE_CMD (cmd_reset, root, '6', "reset", "Reset system");
DECLARE_CMD (cmd_shutdown, root, '7', "shutdown", "System shutdown");

CMD(cmd_reset, cg)
{
#if defined(CONFIG_KDB_CONS_OF1275)
    if( powerpc_version_t::read().is_psim() )
	get_of1275_ci()->exit();
    else
	get_of1275_ci()->interpret( "reset-all" );
#endif

    return CMD_NOQUIT;
}

CMD(cmd_shutdown, cg)
{
#if defined(CONFIG_KDB_CONS_OF1275)
    if( powerpc_version_t::read().is_psim() )
	get_of1275_ci()->exit();
    else
	get_of1275_ci()->interpret( "shut-down" );
#endif

    return CMD_NOQUIT;
}

