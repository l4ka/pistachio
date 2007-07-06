/*********************************************************************
 *                
 * Copyright (C) 2002,  Karlsruhe University
 *                
 * File path:     kdb/arch/ia64/singlestep.cc
 * Description:   Single stepping for IA-64
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
 * $Id: singlestep.cc,v 1.4 2003/09/24 19:05:06 skoglund Exp $
 *                
 ********************************************************************/
#include <kdb/kdb.h>

#include INC_GLUE(context.h)


/**
 * cmd_singlestep: execute next instrucion in instruction stream
 */
DECLARE_CMD (cmd_singlestep, root, 's', "singlestep",
	     "single step instrucion");

CMD (cmd_singlestep, cg)
{
    ia64_exception_context_t * frame =
	(ia64_exception_context_t *) kdb.kdb_param;

    frame->ipsr.ss = 1;
    return CMD_QUIT;
}


/**
 * cmd_branchstep: execute instructions until next branch is taken
 */
DECLARE_CMD (cmd_branchstep, root, 'S', "branchstep",
	     "execute until next taken branch");

CMD (cmd_branchstep, cg)
{
    ia64_exception_context_t * frame =
	(ia64_exception_context_t *) kdb.kdb_param;

    frame->ipsr.tb = 1;
    return CMD_QUIT;
}
