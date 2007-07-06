/*********************************************************************
 *                
 * Copyright (C) 1997, 1998, 2003,  University of New South Wales
 *                
 * File path:     kdb/platform/u4600/z85230.cc
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
 * $Id: z85230.cc,v 1.3 2003/09/24 19:05:23 skoglund Exp $
 *                
 ********************************************************************/
#include "z85230.h"

typedef struct zsccdev *zsccdp;

#define ZSCCCLK	14745600	 /* clock rate 14.7456 MHz */

void wbflush(void)
{
    asm ("sync;\n\t");
}

static const int zsccbrtc[] = {
    -1,
    BRTC (ZSCCCLK, 50),
    BRTC (ZSCCCLK, 75),
    BRTC (ZSCCCLK, 110),
    BRTC (ZSCCCLK, 134),
    BRTC (ZSCCCLK, 150),
    BRTC (ZSCCCLK, 200),
    BRTC (ZSCCCLK, 300),
    BRTC (ZSCCCLK, 600),
    BRTC (ZSCCCLK, 1200),
    BRTC (ZSCCCLK, 1800),
    BRTC (ZSCCCLK, 2400),
    BRTC (ZSCCCLK, 4800),
    BRTC (ZSCCCLK, 9600),
    BRTC (ZSCCCLK, 19200),
    BRTC (ZSCCCLK, 38400)};


static void
zsccputreg (volatile zsccdp dp, unsigned char reg, unsigned char val)
{
  if (reg != zWR0) {
      dp->ucmd = zWR0_REG | reg; 
      wbflush ();
  }
  dp->ucmd = val;
  wbflush ();
}

static unsigned char
zsccgetreg (volatile zsccdp dp, unsigned char reg)
{
  if (reg != zRR0) {
    dp->ucmd = zWR0_REG | reg;
    wbflush ();
  }
  return (dp->ucmd);
}

static int
zsccinit (volatile zsccdp dp)
{
    volatile zsccdp dpa, dpb;

    dpa = dp; dpb = dpa - 1;

    /* single read to get in known state */
    (void) dpa->ucmd;
    (void) dpb->ucmd;

    /* global initialisation */
    
    zsccputreg(dpa, zWR9, zWR9_HARDRESET);

    return 0;
}


static void
zsccflush (volatile zsccdp dp)
{
    /* wait for Tx fifo to drain */
    int timeout = 10;
    while (!(zsccgetreg (dp, zRR0) & zRR0_TXEMPTY))
	if (--timeout == 0)
	    break;
}


static int
zsccprogram (volatile zsccdp dp, int baudrate)
{
    zsccflush (dp);

    baudrate &= 17;
    if (baudrate == 0)
      return 1;
    
    /*
     * See the zscc manual for details.
     */

    zsccputreg(dp, zWR4, zWR4_1STOPBIT | zWR4_CLK);
    zsccputreg(dp, zWR10, zWR10_NRZ);
    
    zsccputreg(dp, zWR14, zWR14_NOP); /* stop BRG */
    zsccputreg(dp, zWR11, zWR11_TRXCOUT| zWR11_TRXCBRG| zWR11_TCLKBRG | zWR11_RCLKBRG);
    zsccputreg(dp, zWR12,  zsccbrtc [baudrate] & 0xff);
    zsccputreg(dp, zWR13,  (zsccbrtc [baudrate] >> 8) & 0xff);
    
    zsccputreg(dp, zWR14, zWR14_BRGENABLE | zWR14_BRGSRC );
    
    zsccputreg(dp, zWR3, zWR3_RXENABLE | zWR3_RX8BITCHAR);
    zsccputreg(dp, zWR5, zWR5_TXENABLE| zWR5_RTS | zWR5_TX8BITCHAR | zWR5_DTR);
    return 0;
}


int pzscc (int op, char *dat, int chan, int data)
{
    volatile zsccdp dp = (zsccdp) dat;

    switch (op) {
    case OP_INIT:
	return zsccinit (dp);
    case OP_BAUD:
	return zsccprogram (dp, data);
    case OP_TXRDY:
	return (dp->ucmd & zRR0_TXEMPTY);
    case OP_TX:
	dp->udata = data; wbflush ();
	break;
    case OP_RXRDY:
	return (dp->ucmd & zRR0_RXAVAIL);
    case OP_RX:
	return (dp->udata ); 
    case OP_FLUSH:
	zsccflush (dp);
	break;
    case OP_RXSTOP:
	/* rx flow control */
	zsccputreg (dp, zWR5, zWR5_TXENABLE| zWR5_TX8BITCHAR | zWR5_DTR | 
		    (data ? 0 : zWR5_RTS));
	break;
    }
    return 0;
}
