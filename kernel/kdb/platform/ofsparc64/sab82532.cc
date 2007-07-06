/*********************************************************************
 *                
 * Copyright (C) 2003-2004,  University of New South Wales
 *                
 * File path:     kdb/platform/ofsparc64/sab82532.cc
 * Description:   Console SPARC v9 systems with a Siemens SAB82532
 *                enhanced serial communications controller. This is
 *                used on the following ofsparc64 platforms:
 *                - Sun Ultra 10 workstation.
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
 * $Id: sab82532.cc,v 1.5 2004/05/21 01:36:06 philipd Exp $
 *                
 ********************************************************************/

#include <kdb/console.h>
#include INC_ARCH(asi.h)

#warning awiggins (22-08-03): SAB82532 should not be hardcoded here.
/* Base address (physical) of SAB82532. */
#define SAB82532_BASEPADDR 0x1FFF1400000

/**************************
* Registers in Async mode *
**************************/

/**
 *  Port A: Read-only registers.
 */

/* Receive FIFOs. */
#define SAB82532_A_RFIFO 0x00

/* Status register. */
#define SAB82532_A_STAR  0x20

#define SAB82532_STAR_XDOV (1 << 7) /* Transmit data overflow.     */
#define SAB82532_STAR_XFW  (1 << 6) /* Transmit FIFO write enable. */
#define SAB82532_STAR_RFNE (1 << 5) /* RFIFO not empty.            */
#define SAB82532_STAR_FCS  (1 << 4) /* Flow control status.        */
#define SAB82532_STAR_TEC  (1 << 3) /* TIC executing.              */
#define SAB82532_STAR_CEC  (1 << 2) /* Command Executing.          */
#define SAB82532_STAR_CTS  (1 << 1) /* Clear to send state.        */

/**
 *  Port A: Write-only registers.
 */

/* Transmit FIFO. */
#define SAB82532_A_XFIFO 0x00

/* Command Register. */
#define SAB82532_A_CMDR  0x20

#define SAB82532_CMDR_RMC  (1 << 7) /* Receive message complete. */
#define SAB82532_CMDR_RRES (1 << 6) /* Receiver reset.           */
#define SAB82532_CMDR_RFRD (1 << 5) /* Receive FIFO read enable. */
#define SAB82532_CMDR_STI  (1 << 4) /* Start timer.              */
#define SAB82532_CMDR_XF   (1 << 3) /* Transmit frame.           */
#define SAB82532_CMDR_XRES (1 << 0) /* Transmitter reset.        */

/**
 *  Port B registers.
 */ 

#define SAB82532_B_OFFSET 0x40

#define SAB82532_B_RFIFO (SAB82532_A_RFIFO + SAB82532_B_OFFSET)
#define SAB82532_B_STAR  (SAB82532_A_STAR  + SAB82532_B_OFFSET)
#define SAB82532_B_XFIFO (SAB82532_A_XFIFO + SAB82532_B_OFFSET)
#define SAB82532_B_CMDR  (SAB82532_A_CMDR  + SAB82532_B_OFFSET)

/********************
* Console functions * 
********************/

void
serialB_putc(char c)
{
    u8_t star; /* Status register */

    /* Check if SAB ready for next command and XFIFO has space. */ 
    do {
	__asm__ __volatile__(
			     "lduba\t[%1 + %2] %3, %0\t! Load status register.\n"
			     : "=r" (star)
			     : "r" (SAB82532_BASEPADDR),      // %0
			       "r" (SAB82532_B_STAR),         // %1
			       "i" (ASI_PHYS_BYPASS_EC_E_L)); // %2

    } while((star & SAB82532_STAR_CEC) || !(star & SAB82532_STAR_XFW));

    /* Put character. */
    __asm__ __volatile__(
			 "stba\t%0, [%1 + %2] %3\t! Store charactor to XFIFO.\n"
			 "stba\t%4, [%1 + %5] %3\t! Send frame command.\n"
			 : /* no outputs */
			 : "r" (c),                      // %0
			   "r" (SAB82532_BASEPADDR),     // %1
			   "r" (SAB82532_B_XFIFO),       // %2
			   "i" (ASI_PHYS_BYPASS_EC_E_L), // %3
			   "r" (SAB82532_CMDR_XF),       // %4
			   "r" (SAB82532_B_CMDR));       // %5

    if (c == '\n') {
	serialB_putc('\r');
    }

} // serialB_putc()

char
serialB_getc(bool block)
{
    char c;
    u8_t star; /* Status register.     */

    /* Check if SAB ready for next command and RFIFO not empty. */ 
    do {
	__asm__ __volatile__(
			     "lduba\t[%1 + %2] %3, %0\t! Load status register.\n"
			     : "=r" (star)
			     : "r" (SAB82532_BASEPADDR),      // %0
			       "r" (SAB82532_B_STAR),         // %1
			       "i" (ASI_PHYS_BYPASS_EC_E_L)); // %2

    } while((star & SAB82532_STAR_CEC) || 
	    (block && !(star & SAB82532_STAR_RFNE)));

    if(!block && !(star & SAB82532_STAR_RFNE)) {
	return -1;
    }

    /* Enable RFIFO read. */
    __asm__ __volatile__(
			 "stba\t%0, [%1 + %2] %3\t! Send frame command.\n"
			 : /* no outputs */
			 : "r" (SAB82532_CMDR_RFRD),      // %0
			   "r" (SAB82532_BASEPADDR),      // %1
			   "r" (SAB82532_B_CMDR),         // %2
			   "i" (ASI_PHYS_BYPASS_EC_E_L)); // %3

    /* Check if SAB ready for next command. */ 
    do {
	__asm__ __volatile__(
			     "lduba\t[%1 + %2] %3, %0\t! Load status register.\n"
			     : "=r" (star)
			     : "r" (SAB82532_BASEPADDR),      // %0
			       "r" (SAB82532_B_STAR),         // %1
			       "i" (ASI_PHYS_BYPASS_EC_E_L)); // %2

    } while(star & SAB82532_STAR_CEC);

    /* Get character. */
    __asm__ __volatile__(
			 "lduba\t[%1 + %2] %3, %0\t! Load charactor from RFIFO.\n"
			 : "=r" (c)                       // %0
			 : "r" (SAB82532_BASEPADDR),      // %1
			   "r" (SAB82532_B_RFIFO),        // %2
			   "i" (ASI_PHYS_BYPASS_EC_E_L)); // %3

    /* Signal receive completed. */
    __asm__ __volatile__(
			 "stba\t%0, [%1 + %2] %3\t! Send frame command.\n"
			 : /* no outputs */
			 : "r" (SAB82532_CMDR_RMC),       // %0
			   "r" (SAB82532_BASEPADDR),      // %1
			   "r" (SAB82532_B_CMDR),         // %2
			   "i" (ASI_PHYS_BYPASS_EC_E_L)); // %3

    return c;

} // serialB_getc()

void SECTION(".init")
serialB_init(void)
{
    /* For now port is initialised by the firmware already. */
    /* Add support for setting serial port speed later.     */

} // serialB_init()

kdb_console_t serialB_console =
    {"Serial Port B", &serialB_init, &serialB_putc, &serialB_getc};
