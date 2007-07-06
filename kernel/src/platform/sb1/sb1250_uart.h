/*********************************************************************
 *                
 * Copyright (C) 2002,  University of New South Wales
 *                
 * File path:     sb1250_uart.h
 * Description:   Defines bits we use of the sb1250 uart
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
 * $Id: sb1250_uart.h,v 1.3 2003/09/24 19:05:00 skoglund Exp $
 *                
 ********************************************************************/
#ifndef _SB1250_UART_H
#define _SB1250_UART_H

/* 
 * DUART MODE REG 1 
 */
#define OFFSET_DUART_BITS_PER_CHAR  0
#define OFFSET_DUART_PARITY_MODE    3

#define DUART_BITS_PER_CHAR_7 (2 << OFFSET_DUART_BITS_PER_CHAR)
#define DUART_BITS_PER_CHAR_8 (3 << OFFSET_DUART_BITS_PER_CHAR)

#define DUART_PARITY_MODE_ADD		(0 << OFFSET_DUART_PARITY_MODE)
#define DUART_PARITY_MODE_ADD_FIXED	(1 << OFFSET_DUART_PARITY_MODE)
#define DUART_PARITY_MODE_NONE		(2 << OFFSET_DUART_PARITY_MODE)

/*
 * DUART MODE REG 2
 */

#define DUART_STOP_BIT_LEN_1		(0ULL)
#define DUART_STOP_BIT_LEN_2		(1ULL << 3)

/*
 * DUART Baud Rate REG
 */

#define DUART_BAUD_RATE(x)			(100000000/((x)*20)-1)


/*
 * DUART Command REG
 */

#define DUART_RX_EN		(1ULL<<0)
#define DUART_RX_DIS		(1ULL<<1)
#define DUART_TX_EN		(1ULL<<2)
#define DUART_TX_DIS		(1ULL<<3)


/* 
 * DUART Status REG
 */

#define DUART_RX_RDY		(1ULL<<0)
#define DUART_RX_FFUL		(1ULL<<1)
#define DUART_TX_RDY		(1ULL<<2)
#define DUART_TX_EMT		(1ULL<<3)
#define DUART_OVRUN_ERR		(1ULL<<4)
#define DUART_PARITY_ERR	(1ULL<<5)
#define DUART_FRM_ERR		(1ULL<<6)
#define DUART_RCVD_BRK		(1ULL<<7)

/* 
 * DUART REGISTER ADDRESS
 */

#define DUART_NUM_PORTS		2
#define DUART_PHYS		0x0010060000
#define DUART_PHYS_SIZE		0x100

#define DUART_REG(chan,r)	(DUART_PHYS_SIZE*(chan) + (r))
#define DUART_REG_PHYS(chan,r)	(DUART_PHYS + DUART_REG(chan,r))


#define DUART_MODE_REG_1	0x100
#define DUART_MODE_REG_2	0x110
#define DUART_STATUS		0x120
#define DUART_CLK_SEL		0x130
#define DUART_CMD		0x150
#define DUART_RX_HOLD		0x160
#define DUART_TX_HOLD		0x170



#endif /* !_SB1250_UART_H */
