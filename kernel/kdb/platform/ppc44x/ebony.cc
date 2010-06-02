/*
 * (C) Copyright 2000-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * This source code is dual-licensed.  You may use it under the terms of the
 * GNU General Public License version 2, or under the license below.
 *
 * This source code has been made available to you by IBM on an AS-IS
 * basis.  Anyone receiving this source is licensed under IBM
 * copyrights to use it in any way he or she deems fit, including
 * copying it, modifying it, compiling it, and redistributing it either
 * with or without modifications.  No license under IBM patents or
 * patent applications is to be implied by the copyright license.
 *
 * Any user of this software should understand that IBM cannot provide
 * technical support for this software and will not be responsible for
 * any consequences resulting from the use of this software.
 *
 * Any person who transfers this source code or any derivative work
 * must include the IBM copyright notice, this paragraph, and the
 * preceding two paragraphs in the transferred software.
 *
 * COPYRIGHT   I B M   CORPORATION 1995
 * LICENSED MATERIAL  -  PROGRAM PROPERTY OF I B M
 */

#include "fdt.h"
#include "powerpc.h"

/*-----------------------------------------------------------------------
 * High Level Configuration Options
 *----------------------------------------------------------------------*/
#define CONFIG_EBONY		1	    /* Board is ebony		*/
#define CONFIG_440GP		1	    /* Specifc GP support	*/
#define CONFIG_440		1	    /* ... PPC440 family	*/
#define CONFIG_4xx		1	    /* ... PPC4xx family	*/
#define CONFIG_BOARD_EARLY_INIT_F 1	    /* Call board_early_init_f	*/
/*-----------------------------------------------------------------------
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_SDRAM_BASE	    0x00000000	    /* _must_ be 0		*/
#define CONFIG_SYS_FLASH_BASE	    0xff800000	    /* start of FLASH		*/
#define CONFIG_SYS_PCI_MEMBASE	    0x80000000	    /* mapped pci memory	*/
#define CONFIG_SYS_PERIPHERAL_BASE 0xe0000000	    /* internal peripherals	*/
#define CONFIG_SYS_ISRAM_BASE	    0xc0000000	    /* internal SRAM		*/
#define CONFIG_SYS_PCI_BASE	    0xd0000000	    /* internal PCI regs	*/

#define CONFIG_SYS_NVRAM_BASE_ADDR (CONFIG_SYS_PERIPHERAL_BASE + 0x08000000)
#define CONFIG_SYS_FPGA_BASE	    (CONFIG_SYS_PERIPHERAL_BASE + 0x08300000)
/*-----------------------------------------------------------------------
 * Serial Port
 *----------------------------------------------------------------------*/
#undef	CONFIG_SERIAL_SOFTWARE_FIFO
#define CONFIG_SYS_EXT_SERIAL_CLOCK	(1843200 * 6)	/* Ext clk @ 11.059 MHz */
#define CONFIG_SYS_CLK_FREQ	33333333    /* external freq to pll	*/
#define CONFIG_BAUDRATE		115200
#define CONFIG_SERIAL_MULTI
#define CONFIG_SYS_BAUDRATE_TABLE  \
    {300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400}

#define CNTRL_DCR_BASE 0x0b0
#define CPC0_CR0		(CNTRL_DCR_BASE+0x3b)	/* Control 0 register */
#define CPC0_CR1		(CNTRL_DCR_BASE+0x3a)	/* Control 1 register */

#define UART0_BASE	(CONFIG_SYS_PERIPHERAL_BASE + 0x00000200)

#define CR0_MASK        0x3fff0000
#define CR0_EXTCLK_ENA  0x00600000
#define CR0_UDIV_POS    16
#define UDIV_SUBTRACT	1
#define UART0_SDR	CPC0_CR0
#define MFREG(a, d)	d = mfdcr(a)
#define MTREG(a, d)	mtdcr(a, d)


#define UART_RBR    0x00
#define UART_THR    0x00
#define UART_IER    0x01
#define UART_IIR    0x02
#define UART_FCR    0x02
#define UART_LCR    0x03
#define UART_MCR    0x04
#define UART_LSR    0x05
#define UART_MSR    0x06
#define UART_SCR    0x07
#define UART_DLL    0x00
#define UART_DLM    0x01

/*-----------------------------------------------------------------------------+
  | Line Status Register.
  +-----------------------------------------------------------------------------*/
#define asyncLSRDataReady1            0x01
#define asyncLSROverrunError1         0x02
#define asyncLSRParityError1          0x04
#define asyncLSRFramingError1         0x08
#define asyncLSRBreakInterrupt1       0x10
#define asyncLSRTxHoldEmpty1          0x20
#define asyncLSRTxShiftEmpty1         0x40
#define asyncLSRRxFifoError1          0x80



/*
 * Minimal serial functions needed to use one of the SMC ports
 * as serial console interface.
 */

int serial_init_dev(unsigned long base)
{
    unsigned long reg;
    unsigned long udiv;
    unsigned short bdiv;
    unsigned long tmp;
    L4_Word8_t val;

    MFREG(UART0_SDR, reg);
    reg &= ~CR0_MASK;

    reg |= CR0_EXTCLK_ENA;
    udiv = 1;
    tmp  = CONFIG_BAUDRATE * 16;
    bdiv = (CONFIG_SYS_EXT_SERIAL_CLOCK + tmp / 2) / tmp;

    reg |= (udiv - UDIV_SUBTRACT) << CR0_UDIV_POS;	/* set the UART divisor */

    /*
     * Configure input clock to baudrate generator for all
     * available serial ports here
     */
    MTREG(UART0_SDR, reg);

    
    out_8((L4_Word8_t *)base + UART_LCR, 0x80);	/* set DLAB bit */
    out_8((L4_Word8_t *)base + UART_DLL, bdiv);	/* set baudrate divisor */
    out_8((L4_Word8_t *)base + UART_DLM, bdiv >> 8); /* set baudrate divisor */
    out_8((L4_Word8_t *)base + UART_LCR, 0x03);	/* clear DLAB; set 8 bits, no parity */
    out_8((L4_Word8_t *)base + UART_FCR, 0x00);	/* disable FIFO */
    out_8((L4_Word8_t *)base + UART_MCR, 0x00);	/* no modem control DTR RTS */
    val = in_8((L4_Word8_t *)base + UART_LSR);	/* clear line status */
    val = in_8((L4_Word8_t *)base + UART_RBR);	/* read receive buffer */
    out_8((L4_Word8_t *)base + UART_SCR, 0x00);	/* set scratchpad */
    out_8((L4_Word8_t *)base + UART_IER, 0x00);	/* set interrupt enable reg */


    return (0);
}


void serial_putc_dev(unsigned long base, const char c)
{
	int i;

	if (c == '\n')
		serial_putc_dev(base, '\r');

	/* check THRE bit, wait for transmiter available */
	for (i = 1; i < 3500; i++) {
		if ((in_8((L4_Word8_t *)base + UART_LSR) & 0x20) == 0x20)
			break;
		//udelay (100);
	}

	out_8((L4_Word8_t *)base + UART_THR, c);	/* put character out */
}

void serial_puts_dev (unsigned long base, const char *s)
{
	while (*s)
		serial_putc_dev (base, *s++);
}

int serial_getc_dev (unsigned long base)
{
	unsigned char status = 0;

	while (1) {
		status = in_8((L4_Word8_t *)base + UART_LSR);
		if ((status & asyncLSRDataReady1) != 0x0)
			break;

		if ((status & ( asyncLSRFramingError1 |
				asyncLSROverrunError1 |
				asyncLSRParityError1  |
				asyncLSRBreakInterrupt1 )) != 0) {
			out_8((L4_Word8_t *)base + UART_LSR,
			      asyncLSRFramingError1 |
			      asyncLSROverrunError1 |
			      asyncLSRParityError1  |
			      asyncLSRBreakInterrupt1);
		}
	}

	return (0x000000ff & (int) in_8((L4_Word8_t *)base));
}


class cons_t {
public:
    bool verbose;


    bool init()
	{
            serial_init_dev(UART0_BASE);
            serial_init_dev(UART1_BASE);
            return true;
        }

    void putc(int c)
	{
            serial_putc_dev(UART0_BASE,c);
            serial_putc_dev(UART1_BASE,c);
	}
};

cons_t cons;

bool initialize_console(fdt_t *fdt)
{
    return cons.init();
}

extern "C" void putc(int c)
{
    cons.putc(c);
}

kdb_console_t kdb_consoles[] = {
#if defined(CONFIG_KDB_CONS_COM)
    { "serial", init_serial, putc_serial, getc_serial },
#endif
    KDB_NULL_CONSOLE
};
