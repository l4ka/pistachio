/*********************************************************************
 *
 * Copyright (C) 2004,  National ICT Australia (NICTA)
 *
 * File path:     platform/innovator/offsets.h
 * Description:   Offsets for TI Innovator
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
 * $Id: reg.h,v 1.2 2004/06/04 03:27:22 cvansch Exp $
 *
 ********************************************************************/

#ifndef __PLATFORM__INNOVATOR__REGS_H__
#define __PLATFORM__INNOVATOR__REGS_H__

#define SER_UART_REGDELTA	0x4	/* 1 for GP uart, 4 for BT/COM uarts */

#define SER_XTAL	750000

#define UART1_BASE	0xfffb0000	/* Bluetooth UART (UART1) (label="Bluetooth Uart") */
#define UART2_BASE	0xfffb0800	/* COM UART (UART2) - not on front panel */
#define UART0_BASE	0xfffce800	/* GP/DSP UART (UART3) (label="modem Uart") */

//#define SER		0
//#define PAR		1

#define SER_FIFO_ENABLE	0x07

#define SER_RBR		0x00
#define SER_ISR		0x02
#define SER_LSR		0x05
#define SER_MSR		0x06
#define SER_THR		0x00
#define SER_DLL		0x00
#define SER_IER		0x01
#define SER_DLM		0x01
#define SER_FCR		0x02
#define SER_LCR		0x03
#define SER_MCR		0x04
#define SER_SCR		0x07
#define TI_TRIG		0x07
#define TI_XOFF1	0x06
#define MODE_DEF	0x08
#define SER_OSC_12M_SEL	0x13	/* 6.5 divider for UART1 & UART2
				 * THIS IS INCORRECTLY DOCUMENTED IN
				 * OMAP1509 TRM (as 0x12 (0x48/4))
				 */

#define SER_LCR_DLAB 0x80
#define SER_LSR_THRE 0x20
#define SER_LSR_BI   0x10
#define SER_LSR_DR   0x01
/*
 * Enable receive and transmit FIFOs.
 *
 * FCR<7:6> 00 trigger level = 1 byte
 * FCR<5:4> 00 reserved
 * FCR<3>   0  mode 1 - interrupt on fifo threshold
 * FCR<2>   1  clear xmit fifo
 * FCR<1>   1  clear recv fifo
 * FCR<0>   1  turn on fifo mode
 */

#define WAIT				\
    asm volatile (			\
	"mov	r0, #0x1800	\n"	\
	"subs	r0, r0, #0x1	\n"	\
	"bne	. - 0x4		\n"	\
    );

#define OMAP_ARM_CKCTL			0xfffece00
#define OMAP_ARM_IDLECT1		0xfffece04
#define OMAP_ARM_IDLECT2		0xfffece08
#define OMAP_ARM_RSTCT1			0xfffece10
#define OMAP_ARM_RSTCT2			0xfffece14
#define OMAP_ARM_SYSST			0xfffece18
#define OMAP_DPLL1_CTL			0xfffecf00
#define OMAP_TC_EMIFS_CS0_CONFIG	0xfffecc10
#define OMAP_TC_EMIFS_CS1_CONFIG	0xfffecc14
#define OMAP_TC_EMIFS_CS2_CONFIG	0xfffecc18
#define OMAP_TC_EMIFS_CS3_CONFIG	0xfffecc1c
#define VAL_ARM_SW_RST			0x0008
#define VAL_ARM_CKCTL			0x110f
#define VAL_DPLL1_CTL			0x2710
#define VAL_TC_EMIFS_CS0_CONFIG		0x002130b0
#define VAL_TC_EMIFS_CS1_CONFIG		0x0000f559
#define VAL_TC_EMIFS_CS2_CONFIG		0x000055f0
#define VAL_TC_EMIFS_CS3_CONFIG		0x00003331

#define REG_OMAP_CKCTL	    *((volatile word_t *) io_to_virt (OMAP_ARM_CKCTL))
#define REG_OMAP_IDLECT1    *((volatile word_t *) io_to_virt (OMAP_ARM_IDLECT1))
#define REG_OMAP_IDLECT2    *((volatile word_t *) io_to_virt (OMAP_ARM_IDLECT2))

#endif /* __PLATFORM__INNOVATOR__REGS_H__ */
