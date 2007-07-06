/*********************************************************************
 *                
 * Copyright (C) 2004,  National ICT Australia (NICTA)
 *                
 * File path:     kdb/platform/pleb/reboot.cc
 * Description:   SA1100 system reset
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
 * $Id: reboot.cc,v 1.2 2004/06/04 06:43:13 cvansch Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include <kdb/kdb.h>

#include INC_API(space.h)
#include INC_API(tcb.h)
#include INC_PLAT(reg.h)
#include INC_CPU(io.h)

/*
 * Reboot the box
 */
DECLARE_CMD (cmd_reboot, root, '6', "reset", "Reset system");

CMD (cmd_reboot, cg)
{
    tcb_t *tcb;
    space_t *s = NULL;

    tcb = s->get_tcb (kdb.kdb_param);
    s = tcb->get_space ();

    *((word_t *) io_to_virt (OMAP_ARM_RSTCT1)) = VAL_ARM_SW_RST;
    return CMD_NOQUIT;
}
