#ifndef _Z85230_H
#define _Z85230_H
/****************************************************************************
 * $Id: z85230.h,v 1.1 2003/04/30 17:20:18 sjw Exp $
 * Copyright (C) 1997, 1998 Kevin Elphinstone, Univeristy of New South
 * Wales.
 *
 * This file is part of the L4/MIPS micro-kernel distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 ****************************************************************************/

/****************************************************************************
 * Register definitions for z85230. See data book for more information
 ****************************************************************************/

#ifndef __ASSEMBLER__
struct zsccdev {
  volatile unsigned char		ucmd;
  volatile unsigned char		udata;
};
#endif

#ifndef ZSCCX
#define ZSCCX 16		/* default to 16X clocking */
#define zWR4_CLK  zWR4_X16CLK 
#endif

/* convert baudrate into a value for the BRG */
#define BRTC(clk, brate) (((clk) / (2*(brate)*ZSCCX)) - 2)

#ifdef __ASSEMBLER__
/* offsets from usart base for usart registers */
#define CMD	0
#define DATA	1
#endif


#define zWR0    0               /* reg pointers and various init */
#define zWR1    1               /* Tx and Rx int enables, WAIT/DMA */
#define zWR2    2               /* interupt vectors */
#define zWR3    3               /* Rx params and control modes */
#define zWR4    4               /* Tx and Rx modes and params */
#define zWR5    5               /* Tx params and control modes */
#define zWR6    6               /* Sync Char or SDLC address */
#define zWR7    7               /* Sync Char or SDLC flag */
#define zWR7p   7               /* Extended Feature and FIFO control */
#define zWR8    8               /* Tx buffer */
#define zWR9    9               /* Master Int control and reset commands */
#define zWR10   10              /* Misc TX and RX control bits */
#define zWR11   11              /* clock mode controls for RX and TX */
#define zWR12   12              /* Low byte of baud rate gen */
#define zWR13   13              /* high byte of baud rate gen */
#define zWR14   14              /* Misc control bits */
#define zWR15   15              /* External status int enable control */

#define zRR0    0               /* Tx and Rx buffer stat and extern stat */
#define zRR1    1               /* Special Rx Condition status */
#define zRR2B   2               /* modified Int vector */
#define zRR2A   2               /* unmodified int vector */
#define zRR3A   3               /* Int pending bits */
#define zRR4    4               /* WR4 */
#define zRR5    5               /* WR5 */
#define zRR6    6               /* SDLC FIFO count, lower byte */
#define zRR7    7               /* SDLC FIFO count and status */
#define zRR8    8               /* Rx buffer */
#define zRR9    9               /* WR3 */
#define zRR10   10              /* Misc status bits */
#define zRR11   11              /* WR10 */
#define zRR12   12              /* Lower byte of baud rate gen */
#define zRR13   13              /* Upper byte of baud rate gen */
#define zRR14   14              /* Extended feature and FIFO control */
#define zRR15   15              /* External status int info */


#define zWR0_NOP             0000
#define zWR0_REG             0000
#define zWR0_RESETTXLATCH    0300
#define zWR0_RESETTXCRC      0200
#define zWR0_RESETRXCRC      0100
#define zWR0_RESETIUS        0070
#define zWR0_ERRORRESET      0060
#define zWR0_RESETTXINT      0050
#define zWR0_INTNEXTCHAR     0040
#define zWR0_SENDABORT       0030
#define zWR0_RESETINT        0020
#define zWR0_POINTHIGH       0010

#define zWR3_RXENABLE        0001
#define zWR3_SYNCINHIBIT     0002
#define zWR3_ADDRMODE        0004
#define zWR3_RXCRCENABLE     0010
#define zWR3_HUNTMODE        0020
#define zWR3_AUTOENABLE      0040
#define zWR3_RX5BITCHAR      0000
#define zWR3_RX6BITCHAR      0100
#define zWR3_RX7BITCHAR      0200
#define zWR3_RX8BITCHAR      0300

#define zWR4_PARENABLE       0001
#define zWR4_PAREVEN         0002
#define zWR4_PARODD          0000
#define zWR4_SYNCENABLE      0000
#define zWR4_1STOPBIT        0004
#define zWR4_15STOPBIT       0010
#define zWR4_2STOPBIT        0014
#define zWR4_8BITSYNC        0000
#define zWR4_16BITSYNC       0020
#define zWR4_SDLCMODE        0040
#define zWR4_EXTSYNC         0060
#define zWR4_X1CLK           0000
#define zWR4_X16CLK          0100
#define zWR4_X32CLK          0200
#define zWR4_X64CLK          0300

#define zWR5_TXCRC           0001
#define zWR5_RTS             0002
#define zWR5_SDLCCRC         0004
#define zWR5_TXENABLE        0010
#define zWR5_SENDBREAK       0020
#define zWR5_TX5BITCHAR      0000
#define zWR5_TX6BITCHAR      0040
#define zWR5_TX7BITCHAR      0100
#define zWR5_TX8BITCHAR      0140
#define zWR5_DTR             0200

#define zWR9_HARDRESET       0300
#define zWR9_ARESET          0200
#define zWR9_BRESET          0100
#define zWR9_INTACK          0040
#define zWR9_STATUS          0020
#define zWR9_MIE             0010
#define zWR9_DLC             0004
#define zWR9_NV              0002
#define zWR9_VIS             0001

#define zWR10_8BITSYNC       0001
#define zWR10_LOOPMODE       0002
#define zWR10_ABORT          0004
#define zWR10_MARK           0010
#define zWR10_GOACTIVE       0020
#define zWR10_NRZ            0000
#define zWR10_NRZI           0040
#define zWR10_FM1            0100
#define zWR10_FM0            0140
#define zWR10_CRCPRESET      0200

#define zWR11_TRXCXTAL       0000
#define zWR11_TRXCTCLK       0001
#define zWR11_TRXCBRG        0002
#define zWR11_TRXCDPLL       0003
#define zWR11_TRXCOUT        0004
#define zWR11_TCLKRTXC       0000
#define zWR11_TCLKTRXC       0010
#define zWR11_TCLKBRG        0020
#define zWR11_TCLKDPLL       0030
#define zWR11_RCLKRTXC       0000
#define zWR11_RCLKTRXC       0040
#define zWR11_RCLKBRG        0100
#define zWR11_RCLKDPLL       0140
#define zWR11_RTXCXTAL       0200

#define zWR14_BRGENABLE      0001
#define zWR14_BRGSRC         0002
#define zWR14_DTRFUNC        0004
#define zWR14_AUTOECHO       0010
#define zWR14_LOOPBACK       0020
#define zWR14_NOP            0000
#define zWR14_SEARCH         0040
#define zWR14_RESETCLOCK     0100
#define zWR14_DISABLEDPLL    0140
#define zWR14_SRCBRG         0200
#define zWR14_SRCRTxC        0240
#define zWR14_FMMODE         0300
#define zWR14_NRZIMODE       0340

#define zRR0_RXAVAIL         0001
#define zRR0_ZEROCOUNT       0002
#define zRR0_TXEMPTY         0004
#define zRR0_DCD             0010
#define zRR0_SYNCHUNT        0020
#define zRR0_CTS             0040
#define zRR0_TXUNDERRUN      0100
#define zRR0_BREAKABORT      0200

/* operation codes for device specific driver */
/* operation codes for device specific driver */
#define OP_INIT         1
#define OP_TX           2
#define OP_RX           3
#define OP_RXRDY        4
#define OP_TXRDY        5
#define OP_BAUD         6
#define OP_RXSTOP       7
#define OP_FLUSH        8
#define OP_RESET        9
#define OP_OPEN         10
#define OP_CLOSE        11

#endif
