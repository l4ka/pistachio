/*********************************************************************
 *
 * Copyright (C) 2003-2004,  University of New South Wales
 *
 * File path:      contrib/elf-loader/platform/innovator/io_omap1510.cc
 * Description:    Interface to basic IO
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
 * $Id: io_omap1510.cc,v 1.1 2004/05/31 02:56:50 cvansch Exp $
 *
 ********************************************************************/

#include "io.h"


#define FALSE 0
#define TRUE 1

#define SER_UART_REGDELTA 0x4   // 1 for GP uart, 4 for BT/COM uarts
#define SER_CHANNELOFFSET 0x0
#define SER_XTAL 750000
static const int CONSOLE_CHANNEL = 0; // 0 = 1st channel of device; "uart 1" connector.
                                      // 1 = 2nd channel of device; "uart 2" connector.
#define UART1_BASE 0xfffb0000 // Bluetooth UART (UART1) (label="Bluetooth Uart")
#define UART2_BASE 0xfffb0800 // COM UART (UART2) - not on front panel
#define UART0_BASE 0xfffce800 // GP/DSP UART (UART3) (label="modem Uart")

static const int UART_IOBASE = UART2_BASE /* UART2_BASE */;


#define SER 0
#define PAR 1

static const int CONSOLE_TYPE = SER;  // 0 = serial port device
                                      // 1 = parallel port device

#define SER_FIFO_ENABLE 0x07

                      
                      
#define SER_RBR 0x00  
#define SER_ISR 0x02  
#define SER_LSR 0x05  
#define SER_MSR 0x06  
                      
                      
                      
#define SER_THR 0x00  
#define SER_DLL 0x00  
#define SER_IER 0x01  
#define SER_DLM 0x01  
#define SER_FCR 0x02  
#define SER_LCR 0x03  
#define SER_MCR 0x04  
#define SER_SCR 0x07  
#define TI_TRIG  0x07 
#define TI_XOFF1 0x06 
#define MODE_DEF 0x08
#define SER_OSC_12M_SEL 0x13    // 6.5 divider for UART1 & UART2
                                // THIS IS INCORRECTLY DOCUMENTED IN
                                // OMAP1509 TRM (as 0x12 (0x48/4))

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

#define WAIT asm volatile ("mov r0, #0x1800 \n subs r0, r0, #0x1 \n bne . - 0x4");

#define REG_ARM_IDLECT2	0xfffece08
#define REG_ARM_RSTCT2	0xfffece14
#define REG_ARM_SYSST	0xfffece18
#define REG_ARM_CKCTL	0xfffece00
#define REG_DPLL1_CTL	0xfffecf00
#define REG_TC_EMIFS_CS0_CONFIG 0xfffecc10
#define REG_TC_EMIFS_CS1_CONFIG 0xfffecc14
#define REG_TC_EMIFS_CS2_CONFIG 0xfffecc18
#define REG_TC_EMIFS_CS3_CONFIG 0xfffecc1c
#define VAL_ARM_CKCTL	0x110f
#define VAL_DPLL1_CTL	0x2710
#define VAL_TC_EMIFS_CS0_CONFIG	0x002130b0
#define VAL_TC_EMIFS_CS1_CONFIG	0x0000f559
#define VAL_TC_EMIFS_CS2_CONFIG	0x000055f0
#define VAL_TC_EMIFS_CS3_CONFIG	0x00003331

static int is_par_char();
static void io_putchar_par(unsigned char c);

/******************************
 Routine:
 Description:
 ******************************/
static unsigned char inreg_ser(int reg)
{
  unsigned char val;
  val = *((volatile unsigned char *)(UART_IOBASE + (CONSOLE_CHANNEL * SER_CHANNELOFFSET) + (reg * SER_UART_REGDELTA)));
  return val;
}

/******************************
 Routine:
 Description:
 ******************************/
static void outreg_ser(int reg, unsigned char val)
{
  *((volatile unsigned char *)(UART_IOBASE + (CONSOLE_CHANNEL * SER_CHANNELOFFSET) + (reg * SER_UART_REGDELTA))) = val;
}

/******************************
 Routine:
 Description:
 ******************************/
static int is_ser_char()
{
  if (*FPGA_DIP_SWITCH_REGISTER == DONT_USE_SERIAL_VALUE) return FALSE;
  if (inreg_ser(SER_LSR) & SER_LSR_DR) {
    return TRUE;
  }
  else {
    return FALSE;
  }
}

/******************************
 Routine:
 Description:
 ******************************/
static void io_putchar_ser(unsigned char c)
{
  if ('\n' == c) {
    io_putchar_ser('\r');
  }
  while (0 == (inreg_ser(SER_LSR) & SER_LSR_THRE)){}
  outreg_ser(SER_THR, c);
}


void print_byte_ser(unsigned char c)
{
  while (0 == (inreg_ser(SER_LSR) & SER_LSR_THRE)){}
  outreg_ser(SER_THR, c);
}


/******************************
 Routine:
 Description:
 ******************************/
unsigned char io_getchar_ser(void)
{
  while (!is_ser_char()){}
  return inreg_ser(SER_RBR);
}

/******************************
 Routine:
 Description:
 ******************************/
static int is_char()
{
  if (SER == CONSOLE_TYPE) {
    return is_ser_char();
  }
  else {
    return is_par_char();
  }
}

/******************************
 Routine:
 Description:

   timeout_sec
        -1 -- wait indefinitely for a key press.
         0 -- don't wait at all, if a key value
              is present now return it, otherwise
              return 0.
         t -- wait up to this long for a key press
              to return, otherwise give up and return
              0.
 ******************************/
unsigned char io_getc(int timeout_msec)
{
  #define LOOPS_PER_MSEC 200 // tuned on omap1510
  int time_remaining = LOOPS_PER_MSEC*timeout_msec;
  while (time_remaining--) {
    if (is_char()) {
      return io_getchar_con();
    }
  }
  return 0;
}

/******************************
 Routine:
 Description:
 ******************************/
unsigned char io_getchar_con()
{
  if (SER == CONSOLE_TYPE) {
    return io_getchar_ser();
  }
  else {
    return io_getchar_par();
  }
}

/******************************
 Routine:
 Description:
 ******************************/
void io_putchar(unsigned char c)
{

  if (SER == CONSOLE_TYPE) {
    if (*FPGA_DIP_SWITCH_REGISTER == DONT_USE_SERIAL_VALUE) return;
    io_putchar_ser(c);
  }
  else {
    io_putchar_par(c);
  }
}

/******************************
 Routine:
 Description:
 ******************************/
static int is_par_char()
{
  return FALSE; // TBSL *debug* temp.
}

/******************************
 Routine:
 Description:
 ******************************/
static void io_putchar_par(unsigned char c)
{
  if ('\n' == c) {
    io_putchar_par('\r');
  }
  // TBSL *debug* finish later.
}

/******************************
 Routine:
 Description:
 ******************************/
unsigned char io_getchar_par(void)
{
  while (!is_par_char()) {}
  return 0; // *debug* temp, finish later.
}

/******************************
 Routine:
 Description:
******************************/
static void misc_init()
{
  //ARM Clock Module Setup
  *((volatile unsigned short *) REG_ARM_IDLECT2) = 0x40; 
  *((volatile unsigned short *) REG_ARM_RSTCT2) = 0x01;
  *((volatile unsigned short *) REG_ARM_IDLECT2) = 0x06;
  *((volatile unsigned short *) REG_ARM_SYSST) = 0x1000; WAIT
  *((volatile unsigned short *) REG_ARM_CKCTL) = VAL_ARM_CKCTL;
  //setup DPLL1 Control Register
  *((volatile unsigned short *) REG_DPLL1_CTL) = VAL_DPLL1_CTL;
  if ((VAL_DPLL1_CTL & 0x10) != 0)
  {
  	//wait until bit goes high.
  	while ( (*((volatile unsigned short *) REG_DPLL1_CTL) & 0x1) == 0 ) {}
  }
  //setup TC EMIFS configuration. CS0 value based on 168MHz
  *((volatile unsigned int *) REG_TC_EMIFS_CS0_CONFIG) = VAL_TC_EMIFS_CS0_CONFIG;
  *((volatile unsigned int *) REG_TC_EMIFS_CS1_CONFIG) = VAL_TC_EMIFS_CS1_CONFIG;
  *((volatile unsigned int *) REG_TC_EMIFS_CS2_CONFIG) = VAL_TC_EMIFS_CS2_CONFIG;
  *((volatile unsigned int *) REG_TC_EMIFS_CS3_CONFIG) = VAL_TC_EMIFS_CS3_CONFIG; WAIT
}

int CPU_is_OMAP1510(void)
{
        /*
          A 1510 shows an ID code of 0x1b47002f.  
          A 1509 shows 0.
        */

#define OMAP1510_ID_CODE_REG 0xfffed404
    if (*(volatile unsigned int *)OMAP1510_ID_CODE_REG) {
            return 1;
    }
    else {
            return 0;
    }
}

/******************************
 Routine:
 Description:
******************************/
static void uart_init()
{
  #define BAUD 115200
  #define XTAL 1497600
  #define MUX_CTRL0_REG 0xfffe1000
  unsigned long reg_val;
  unsigned short baud_val;
  unsigned char val;
  unsigned short i;
  if (SER == CONSOLE_TYPE) {
    if (CPU_is_OMAP1510() == 0) {
      reg_val = *((volatile unsigned long *) MUX_CTRL0_REG);
      reg_val = reg_val & 0xfffdffdf; // clear a few specific bits.
      reg_val = reg_val | 0x07010200; // set a few specific bits.
      // Enables GP UART gating, BT UART gating, COM UART gating
      // uWire DTR: 01 -> DTR for GP UART
      // Bluetooth RTS output enabled
      *((volatile unsigned long *)MUX_CTRL0_REG) = reg_val;
      // Pause to let things settle.
      for (i=0; i<2000; i++)
          /* do nothing */;
    }
    
    outreg_ser(SER_IER,0x0);   // Mask all interrupts causes and disable sleep mode and low power mode.
    outreg_ser(MODE_DEF,0x07); // Disable UART (default state)
    outreg_ser(SER_LCR,0xbf);  // Turn FCR into EFR access
    
    val = inreg_ser(SER_FCR);  // This is EFR since LCR=0xBF
    val = val | 0x10;          // Enhanced enable (enables additional bits)
    val = val | 0x40;          // Enable auto-RTS (RTS high when FIFO full)
    // val = val | 0x80;       // Enable auto-CTS (halt xmit when CTS high)
    // Don't enable auto-CTS, as it prevents bootup without host connected
    outreg_ser(SER_FCR,val);   // This is EFR since LCR=0xBF
    
    outreg_ser(SER_LCR,0x83);  // Divisor latch enable, 8N1
    
    val = inreg_ser(SER_MCR);
    val = val | 0x40;          // Enable access to TCR and TLR registers
    outreg_ser(SER_MCR,val);
    
    outreg_ser(TI_XOFF1,0x0d); // Reg#6 ?? only when LCR==0xBF, else it is TCR
                               // TCR set to default (halt RX at 52 bytes,
                               //                     start RX at 0 bytes)
    outreg_ser(TI_TRIG,0xcf);  // Reg#7, XOFF2 when LCR=0xBF, else it is SPR/TLR
                               // TLR set to 48bytes on RX FIFO, 60 on TX FIFO

    outreg_ser(SER_MCR,val & ~0x40);	// Disable access to TCR and TLR

    outreg_ser(SER_FCR,0xe7);  // FCR set to 60/32/TXclear/RXclear/Enable
    
    /* baud_val = XTAL/BAUD; */
    if (BAUD == 115200)
        baud_val = 1;
    else                                // For rates less than 115200
        baud_val = SER_XTAL/BAUD;

    outreg_ser(SER_DLL,(baud_val & 0x00ff));    // baud rate. lobyte
    outreg_ser(SER_DLM,(baud_val >> 8) & 0xff00); // baud rate. hibyte
    
    outreg_ser(SER_LCR,0x03);  // Divisor latch disable, 8N1
	
    if (BAUD == 115200)
        outreg_ser(SER_OSC_12M_SEL, 0x01);      // LCR[7] must be 0 to do this
    outreg_ser(MODE_DEF,0x00); // UART mode
  }
  else {
    // TBSL *debug* finish later.
  }
	
}

/******************************
 Routine:
 Description:
   Note: See io.h for description.
 ******************************/
void io_init(void)
{
  misc_init();
  uart_init();
}
