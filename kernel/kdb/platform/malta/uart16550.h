/*********************************************************************
 *                
 * Copyright (C) 2006,  Karlsruhe University
 *                
 * File path:     kdb/platform/malta/uart16550.h
 * Description:   Malta platform UART definitions
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
 * $Id: uart16550.h,v 1.1 2006/02/23 21:07:44 ud3 Exp $
 *                
 ********************************************************************/
#ifndef __KDB__PLATFORM__MALTA__UART16550_H__
#define __KDB__PLATFORM__MALTA__UART16550_H__

/*  USART ADDRESSES */
#define USART_0_BASE    (0xb80003f8)                    /* reflects PC hardware */

/* Handy shortforms, based on old system. Touch misleading: more status */
#define USART_0_STATUS  (USART_0_BASE + USART_LSR)
#define USART_0_DATA    (USART_0_BASE + USART_RBR)

/*
 *      16550 IO register offsets - should be added to USART_n_BASE
 */
#define USART_RBR              0  /* receive data register */
#define USART_THR              0  /* transmit holding register */
#define USART_IER              1  /* interrupt enable offset */
#define USART_IIR              2  /* interupt identification register */
#define USART_FCR              2  /* FIFO control register */
#define USART_LCR              3  /* line control register */
#define USART_MCR              4  /* modem control register */
#define USART_LSR              5  /* line status register */
#define USART_MSR              6  /* modem status register */

/*
 *      The following constants are primarily for documentation purposes
 *      to show what the values sent to the 8250 Control Registers do to
 *      the chip.
 *
 *                     INTERRUPT ENABLE REGISTER
 */

#define IER_Received_Data      1
#define IER_Xmt_Hld_Reg_Empty  (1<<1)
#define IER_Recv_Line_Status   (1<<2)
#define IER_Modem_Status       (1<<3)
#define IER_Not_Used           0xF0


/*
 *                       LINE CONTROL REGISTER
 */

#define LCR_Word_Length_Mask     3
#define LCR_Stop_Bits            (1<<2)
#define LCR_Parity_Enable        (1<<3)
#define LCR_Even_Parity          (1<<4)
#define LCR_Stick_Parity         (1<<5)
#define LCR_Set_Break            (1<<6)
#define LCR_Divisor_Latch_Access (1<<7)




/* Divisor Latch - must be set to 1
 * to get to the divisor latches of
 * the baud rate generator - must
 * be set to 0 to access the
 * Receiver Buffer Register and
 * the Transmit Holding Register
 */
#define LCR_DLAB                                LCR_Divisor_Latch_Access


/*
 *                      MODEM CONTROL REGISTER
 */

#define MCR_dtr          1
#define MCR_rts          (1<<1)
#define MCR_Out_1        (1<<2)
#define MCR_Out_2        (1<<3) /* MUST BE ASSERTED TO ENABLE INTERRRUPTS */
#define MCR_Loop_Back    (1<<4)
#define MCR_Not_Used     (7<<1)




/*
 *                        LINE STATUS REGISTER
 */

#define LSR_Data_Ready      1
#define LSR_Overrun_Error   (1<<1)
#define LSR_Parity_Error    (1<<2)
#define LSR_Framing_Error   (1<<3)
#define LSR_Break_Interrupt (1<<4)
#define LSR_THR_Empty       (1<<5)  /* Transmitter Holding Register */
#define LSR_TSR_Empty       (1<<6)  /* Transmitter Shift Register */
#define LSR_Not_Used        (1<<7)


/*
 *                       MODEM STATUS REGISTER
 */

#define MSR_Delta_CTS       1
#define MSR_Delta_DSR       (1<<1)
#define MSR_TERD            (1<<2)  /* Trailing Edge Ring Detect   */
#define MSR_Delta_RLSD      (1<<3)  /* Received Line Signal Detect */
#define MSR_CTS             (1<<4)  /* Clear to Send               */
#define MSR_DSR             (1<<5)  /* Data Set Ready              */
#define MSR_RD              (1<<6)  /* Ring Detect                 */
#define MSR_RLSD            (1<<7)  /* Received Line Signal Detect */



/*
 *   Fifo CONTROL REGISTER (Write only)
 */
#define FCR_Fifo_Enable                         1       /* set to one to enable Fifos */
#define FCR_Rcvr_Reset                          (1<<1)  /* set to clear receiver FIFO */
#define FCR_Xmit_Reset                          (1<<2)  /* set to clear transmitter FIFO */
#define FCR_DMA_Mode                            (1<<3)  /* no effect  */           
#define FCR_Trigger_1                           (0<<6)  /* trigger IRQ on one byte */
#define FCR_Trigger_4                           (1<<6)  /* 4 bytes in fifo */
#define FCR_Trigger_8                           (2<<6)  /* 8 bytes */
#define FCR_Trigger_14                          (3<<6)  /* 14 bytes before IRQ occurs */


#endif /* !__KDB__PLATFORM__MALTA__UART16550_H__ */
