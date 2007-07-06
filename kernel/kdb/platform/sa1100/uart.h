/*********************************************************************
 *                
 * Copyright (C) 1999-2002,  Karlsruhe University
 *                
 * File path:     platform/sa1100/uart.h
 * Description:   Definitions for the SA-1100 UART
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
 * $Id: uart.h,v 1.2 2003/09/24 19:05:22 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __PLATFORM__SA1100__UART_H__
#define __PLATFORM__SA1100__UART_H__

/*
 * Base address of UART
 */
#define L4_UART_PBASE	0x80050000
#define L4_UART_BASE	UART_VBASE	/* location in virtual space */

/* Control registers */
#define L4_UART_UTCR0	*((volatile u32_t *) (L4_UART_BASE + 0x00))
#define L4_UART_UTCR1	*((volatile u32_t *) (L4_UART_BASE + 0x04))
#define L4_UART_UTCR2	*((volatile u32_t *) (L4_UART_BASE + 0x08))
#define L4_UART_UTCR3	*((volatile u32_t *) (L4_UART_BASE + 0x0c))

/* Data register */
#define L4_UART_UTDR	*((volatile u32_t *) (L4_UART_BASE + 0x14))

/* Status registers */
#define L4_UART_UTSR0	*((volatile u32_t *) (L4_UART_BASE + 0x1c))
#define L4_UART_UTSR1	*((volatile u32_t *) (L4_UART_BASE + 0x20))



/*
 * Bits defined in control register 0.
 */
#define L4_UART_PE	(1 << 0)	/* Parity enable */
#define L4_UART_OES	(1 << 1)	/* Odd/even parity select */
#define L4_UART_SBS	(1 << 2)	/* Stop bit select */
#define L4_UART_DSS	(1 << 3)	/* Data size select */
#define L4_UART_SCE	(1 << 4)	/* Sample clock rate enable */
#define L4_UART_RCE	(1 << 5)	/* Receive clk. rate edge select */
#define L4_UART_TCE	(1 << 6)	/* Transmit clk. rate edge select */


/*
 * Bits defined in control register 3.
 */
#define L4_UART_RXE	(1 << 0)	/* Receiver enable */
#define L4_UART_TXE	(1 << 1)	/* Transmitter enable */
#define L4_UART_BRK	(1 << 2)	/* Break */
#define L4_UART_RIO	(1 << 3)	/* Receive FIFO interrupt enable */
#define L4_UART_TIE	(1 << 4)	/* Transmit FIFO interrupt enable */
#define L4_UART_LBM	(1 << 5)	/* Loopback mode */


/*
 * Baud rate devisror (contained in control registers 1 and 2).
 */
#define L4_UART_GET_BRD() 				\
    ( (((u32_t) L4_UART_UTCR1 & 0x0f) << 8) + 		\
      (u8_t) L4_UART_UTCR2 )

#define L4_UART_SET_BRD(brd) 				\
    ( *(u32_t *) L4_UART_UTCR1 = brd & 0xff,		\
      *(u32_t *) L4_UART_UTCR2 = (brd >> 8) & 0x0f )	\

#define L4_BRD_TO_BAUDRATE(brd)		(3686400 / ((brd+1) << 4))
#define L4_BAUDRATE_TO_BRD(rate)	(3686400 / (rate << 4) - 1)


/*
 * Bits defined in status register 0.
 */
#define L4_UART_TFS	(1 << 0)	/* Transmit FIFO service req. */
#define L4_UART_RFS	(1 << 1)	/* Receive FIFO service req. */
#define L4_UART_RID	(1 << 2)	/* Receiver idle */
#define L4_UART_RBB	(1 << 3)	/* Receiver begin of break */
#define L4_UART_REB	(1 << 4)	/* Receiver end of break */
#define L4_UART_EIF	(1 << 5)	/* Error in FIFO */


/*
 * Bits defined in status register 0.
 */
#define L4_UART_TBY	(1 << 0)	/* Transmitter busy flag */
#define L4_UART_RNE	(1 << 1)	/* Receiver FIFO not empty */
#define L4_UART_TNF	(1 << 2)	/* Transmitter FIFO not full */
#define L4_UART_PRE	(1 << 3)	/* Parity error */
#define L4_UART_FRE	(1 << 4)	/* Framing error */
#define L4_UART_ROR	(1 << 5)	/* Receive FIFO overrun */


#endif /* !__PLATFORM__SA1100__UART_H__ */
