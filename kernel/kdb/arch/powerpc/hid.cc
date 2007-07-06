/****************************************************************************
 *
 * Copyright (C) 2002, Karlsruhe University
 *
 * File path:	kdb/arch/powerpc/hid.cc
 * Description:	Dump the contents of the 750's hid register.
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
 * $Id: hid.cc,v 1.4 2003/12/11 12:47:45 joshua Exp $
 *
 ***************************************************************************/

#include <debug.h>
#include <kdb/kdb.h>

#include INC_ARCH(ibm750.h)

DECLARE_CMD( cmd_ibm750_hid0, arch, 'h', "hid0", "Dump HID0" );

CMD( cmd_ibm750_hid0, cg )
{
    class ppc750_hid0_t hid0;

    hid0.read();

    printf( "emcp: %d\n", hid0.x.emcp );
    printf( "dbp : %d\n", hid0.x.dbp  );
    printf( "eba : %d\n", hid0.x.eba  );
    printf( "ebd : %d\n", hid0.x.ebd  );
    printf( "bclk : %d\n", hid0.x.bclk );
    printf( "eclk : %d\n", hid0.x.eclk );
    printf( "par : %d\n", hid0.x.par  );
    printf( "doze  : %d\n", hid0.x.doze );
    printf( "nap   : %d\n", hid0.x.nap  );
    printf( "sleep : %d\n", hid0.x.sleep );
    printf( "dpm   : %d\n", hid0.x.dpm  );
    printf( "nhr : %d\n", hid0.x.nhr  );
    printf( "ice : %d\n", hid0.x.ice  );
    printf( "dce : %d\n", hid0.x.dce  );
    printf( "ilock : %d\n", hid0.x.ilock );
    printf( "dlock : %d\n", hid0.x.dlock );
    printf( "icfi  : %d\n", hid0.x.icfi  );
    printf( "dcfi  : %d\n", hid0.x.dcfi  );
    printf( "spd   : %d\n", hid0.x.spd   );
    printf( "ifem  : %d\n", hid0.x.ifem  );
    printf( "sge   : %d\n", hid0.x.sge   );
    printf( "dcfa  : %d\n", hid0.x.dcfa  );
    printf( "btic  : %d\n", hid0.x.btic  );
    printf( "abe   : %d\n", hid0.x.abe   );
    printf( "bht   : %d\n", hid0.x.bht   );
    printf( "noopti : %d\n", hid0.x.noopti );

    return CMD_NOQUIT;
}

