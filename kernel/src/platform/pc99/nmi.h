/*********************************************************************
 *                
 * Copyright (C) 2002, 2007,  Karlsruhe University
 *                
 * File path:     platform/pc99/nmi.h
 * Description:   Driver for NMI masking hardware
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
 * $Id: nmi.h,v 1.3 2003/09/24 19:04:59 skoglund Exp $
 *                
 ********************************************************************/

#ifndef __PLATFORM__PC99__NMI_H__
#define __PLATFORM__PC99__NMI_H__

#include INC_ARCH(ioport.h)	/* for in_u8/out_u8	*/
#include INC_PLAT(rtc.h)	/* for rtc_t		*/

class nmi_t {
public:
    static void mask() 
	{
	    /* disable NMI with read from rtc port < 0x80 */
	    rtc_t<0x70>().read(0);

	    /* clear and disable IOCHK and PCI SERR# */
	    out_u8(0x61, (in_u8(0x61) & 0x03) | 0x0c);
	};
    static void unmask()
	{
	    /* clear and disable IOCHK and PCI SERR# */
	    out_u8(0x61, (in_u8(0x61) & 0x03) | 0x0c);

	    /* waste some time */
	    for (int i = 10000000; i--; ) __asm__ ("");

	    /* enable IOCHK and PCI SERR# */
	    out_u8(0x61, in_u8(0x61) & 0x03);

	    /* enable NMI with read from rtc port < 0x80 */
	    rtc_t<0x70>().read(0);
	};
};

#endif /* !__PLATFORM__PC99__NMI_H__ */
