/*********************************************************************
 *                
 * Copyright (C) 2002-2003, 2007,  Karlsruhe University
 *                
 * File path:     platform/simics/rtc.h
 * Description:   driver for Real Time Clock
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
 * $Id: rtc.h,v 1.3 2003/09/24 19:05:00 skoglund Exp $
 *                
 ********************************************************************/

#ifndef __PLATFORM__SIMICS__RTC_H__
#define __PLATFORM__SIMICS__RTC_H__

#include INC_ARCH(ioport.h)	/* for in_u8/out_u8	*/


/**
 * Driver for Real Time Clock
 * @param base	the base address of the control registers
 *
 * The template parameter BASE enables compile-time resolution of the
 * RTC's control register addresses.
 *
 * Assumptions:
 * - BASE can be passed as port to in_u8/out_u8
 * - The RTC's data register is located at BASE+1
 *
 * Uses:
 * - out_u8, in_u8
 */

template <u16_t base> class rtc_t {
public:

    /**
     *	Read RTC register
     *	@param reg	register to read
     *
     *	@returns the content of RTC register REG.
     */
    static u8_t read(const u8_t reg) {
	/* select register */
	out_u8(base, reg);
	/* read value */
	return in_u8(base+1);
    };
    
    /**
     *	Write RTC register
     *	@param reg	register to write
     *	@param val	value to be written
     *
     *	Sets the content of RTC register REG to VAL.
     */
    static void write(const u8_t reg, const u8_t val) {
	out_u8(base, reg);
	out_u8(base+1, val);
    };
};

/**
 * Waits for a 1 second tick of the realtime clock 
 */
INLINE void wait_for_second_tick()
{
    rtc_t<0x70> rtc;

    // wait that update bit is off
    while (rtc.read(0x0a) & 0x80);

    // read second value
    word_t secstart = rtc.read(0);

    // now wait until seconds change
    while (secstart == rtc.read(0));
}

#endif /* !__PLATFORM__SIMICS__RTC_H__ */
