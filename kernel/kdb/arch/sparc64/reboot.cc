/*********************************************************************
 *                
 * Copyright (C) 2003, University of New South Wales
 *                
 * File path:    kdb/arch/sparc64/reboot.cc
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
 * $Id: reboot.cc,v 1.4 2004/05/21 02:34:54 philipd Exp $
 *                
 ********************************************************************/

#include <debug.h>
#include <kdb/kdb.h>

/**
 *  Reboot the machine.
 *  Notes: Need to revisit this to make it work without any Open Firmware
 *  mappings in the TLB.
 */
DECLARE_CMD(cmd_reboot, root, '6', "reset", "Reset system");

CMD(cmd_reboot, cg)
{
    printf("Sorry you have to reset it yourself :)\n");

#warning awiggins (17-09-03): Since we killed Open Firmware mappings sir does not work!
    //asm("sir\t0\t! Software initiated reset\n");
}
