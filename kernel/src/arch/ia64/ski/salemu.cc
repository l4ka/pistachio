/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     arch/ia64/ski/salemu.cc
 * Description:   SAL emulation for HP's SKI emulator
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
 * $Id: salemu.cc,v 1.3 2003/09/24 19:05:29 skoglund Exp $
 *                
 ********************************************************************/
#include INC_ARCH(sal.h)

extern "C" long
sal_emulator (long index, unsigned long in1, unsigned long in2,
	      unsigned long in3, unsigned long in4, unsigned long in5,
	      unsigned long in6, unsigned long in7)
{
    register long r9 asm ("r9") = 0;
    register long r10 asm ("r10") = 0;
    register long r11 asm ("r11") = 0;
    long status;

    /*
     * Don't do a "switch" here since that gives us code that
     * isn't self-relocatable.
     */
    status = 0;
    if (index == SAL_FREQ_BASE) {
	switch (in1) {
	case sal_freq_base_t::platform:
	    r9 = 200000000;
	    break;

	case sal_freq_base_t::itc:
	    /*
	     * Is this supposed to be the cr.itc frequency
	     * or something platform specific?  The SAL
	     * doc ain't exactly clear on this...
	     */
	    r9 = 700000000;
	    break;

	case sal_freq_base_t::rtc:
	    r9 = 1;
	    break;
	default:
	    status = -1;
	    break;
	}
    } else if (index == SAL_SET_VECTORS) {
	;
    } else if (index == SAL_GET_STATE_INFO) {
	;
    } else if (index == SAL_GET_STATE_INFO_SIZE) {
	;
    } else if (index == SAL_CLEAR_STATE_INFO) {
	;
    } else if (index == SAL_MC_RENDEZ) {
	;
    } else if (index == SAL_MC_SET_PARAMS) {
	;
    } else if (index == SAL_CACHE_FLUSH) {
	;
    } else if (index == SAL_CACHE_INIT) {
	;
    } else if (index == SAL_UPDATE_PAL) {
	;
    } else {
	status = -1;
    }
    asm volatile ("" :: "r"(r9), "r"(r10), "r"(r11));
    return status;
}
