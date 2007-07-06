/*********************************************************************
 *
 * Copyright (C) 2002-2004,  Karlsruhe University
 *
 * File path:     kernel/kdb/generic/tid_format.cc
 * Description:   Implementation of thread ID format configuration
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
 * $Id: tid_format.cc,v 1.2 2004/12/09 01:23:42 cvansch Exp $
 *
 ********************************************************************/
#include <debug.h>
#include <kdb/kdb.h>
#include <kdb/cmd.h>
#include <kdb/input.h>
#include <kdb/tid_format.h>

/**
 * Current thread ID format
 */
kdb_tid_format_t kdb_tid_format = { X: { value:0, human:1, version:0, sep:0 }  };

/**
 * Change memory dump word size.
 */
DECLARE_CMD (cmd_tid_format, config, 't', "tid_format",
	     "change output format of thread IDs");

CMD(cmd_tid_format, cg)
{
    kdb_tid_format.raw = 0;

    char base_format = get_choice ("  Basic output format", "Global id/Tcb address/Both", 'g');
    if ('g' == base_format || 'b' == base_format) {

	if ('g' == base_format)
	    kdb_tid_format.X.value = TID_FORMAT_VALUE_GID;
	else
	    kdb_tid_format.X.value = TID_FORMAT_VALUE_BOTH;

	switch (get_choice ("    Version field format", "Inline/Separate/Off", 'i')) {
	case 'i':
	    kdb_tid_format.X.version = TID_FORMAT_VERSION_INLINE;
	    break;
	case 's':
	    kdb_tid_format.X.version = TID_FORMAT_VERSION_SEP;
	    break;
	case 'o':
	    kdb_tid_format.X.version = TID_FORMAT_VERSION_OFF;
	    break;
	}

	do {
	    printf ("    Add separator before <n> lower-most bits of thread number (0-%lu) [0]: ",
		    L4_GLOBAL_THREADNO_BITS - 1);
	    kdb_tid_format.X.sep = get_dec (NULL, 0, NULL);
	} while (kdb_tid_format.X.sep >= L4_GLOBAL_THREADNO_BITS);
    } else
	kdb_tid_format.X.value = TID_FORMAT_VALUE_TCB;

    kdb_tid_format.X.human = (get_choice ("  Special threads in human readable form", "Yes/No", 'n') == 'y');

    return CMD_NOQUIT;
}
