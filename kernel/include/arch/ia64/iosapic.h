/*********************************************************************
 *                
 * Copyright (C) 2003-2004,  Karlsruhe University
 *                
 * File path:     arch/ia64/iosapic.h
 * Description:   Access to the ia64 I/O SAPIC
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
 * $Id: iosapic.h,v 1.5 2004/04/22 17:40:44 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__IA64__IOSAPIC_H__
#define __ARCH__IA64__IOSAPIC_H__

#include INC_ARCH(ia64.h)

typedef union {
    struct {
	u32_t version	: 8;
	u32_t		: 8;
	u32_t max_redir	: 8;
	u32_t reserved0	: 8;
    } __attribute__ ((packed));
    u32_t raw;
} iosapic_version_t;
    
class iosapic_redir_t
{
public:
    enum trigger_mode_e {
	edge		= 0,
	level		= 1
    };

    enum pin_polarity_e {
	high_active	= 0,
	low_active	= 1
    };

    enum delivery_status_e {
	idle		= 0,
	pending		= 1
    };

    enum delivery_mode_e {
	INT		= 0,
	INT_redir	= 1,
	PMI		= 2,
	NMI		= 4,
	INIT		= 5,
	ExtINT		= 7
    };


    union {
	struct {
	    u64_t vector		:  8;	//  0
	    u64_t delivery_mode		:  3;	//  8
	    u64_t 			:  1;	// 11
	    u64_t delivery_status	:  1;	// 12
	    u64_t pin_polarity		:  1;	// 13
	    u64_t remote_irr		:  1;	// 14
	    u64_t trigger_mode		:  1;	// 15
	    u64_t mask			:  1;	// 16
	    u64_t			: 31;	// 17
	    union {
		struct {
		    u8_t dest_eid	:  8;	// 48
		    u8_t dest_id	:  8;	// 56
		};
		u16_t dest_id_eid	: 16;
	    };
	};

	u64_t raw64;
	u32_t raw32[2];
    };

    void set (word_t vec, pin_polarity_e pol, trigger_mode_e trig,
	      bool masked, word_t id_eid)
	{
	    vector = vec;
	    delivery_mode = INT;
	    pin_polarity = pol;
	    trigger_mode = trig;
	    mask = masked ? 1 : 0;
	    dest_id_eid = id_eid;
	}

    void set_cpu (word_t id_eid)
	{ dest_id_eid = id_eid; }

    void mask_irq (void)
	{ mask = 1; }

    void unmask_irq (void)
	{ mask = 0; }

    bool is_edge_triggered (void) 
	{ return trigger_mode == edge; }

} __attribute__ ((packed));


class iosapic_t
{
    enum reg_off_e {
	io_select		= 0x00,
	io_window		= 0x10,
	irq_assert		= 0x20,
	io_eoi			= 0x40,
    };

    enum reg_idx_e {
	reg_id			= 0x00,
	reg_version		= 0x01,
	reg_regdir_base		= 0x10,
    };

    u32_t get (int reg)
	{
	    *(volatile u32_t *) this = reg;
	    ia64_mf ();
	    return *(volatile u32_t *) ((u64_t) this + io_window);
	}

    void set (int reg, u32_t val)
	{
	    *(volatile u32_t *) this = reg;
	    ia64_mf ();
	    *(volatile u32_t *) ((u64_t) this + io_window) = val;
	    ia64_mf ();
	}

public:

    /**
     * Get redirection table entry for I/O SAPIC.
     * @param which		index for redirection table
     * @return redirection entry
     */
    iosapic_redir_t get_redir (int which)
	{
	    iosapic_redir_t ret;
	    ret.raw32[0] = get (reg_regdir_base + 2 * which);
	    ret.raw32[1] = get (reg_regdir_base + 2 * which + 1);
	    return ret;
	}

    /**
     * Set redirection table entry for I/O SAPIC.
     * @param which		index for redirection table
     * @param redir		new redirection entry
     * @param low_only		only set lower part of entry
     */
    void set_redir (int which, iosapic_redir_t redir, bool low_only = false)
	{
	    set (reg_regdir_base + 2 * which, redir.raw32[0]);
	    if (! low_only)
		set (reg_regdir_base + 2 * which + 1, redir.raw32[1]);
	}

    /**
     * Signal end-of-interrupt (EOI) to I/O SAPIC.
     * @param vector		vector to signal EOI on
     */
    void eoi (int vector)
	{
	    *(volatile u32_t *) ((u64_t) this + io_eoi) = vector;
	    ia64_mf ();
	}

    /**
     * Generate interrupt on I/O SAPIC.
     * @param vector		interrupt vector to assert
     */
    void assert (int vector)
	{
	    *(volatile u32_t *) ((u64_t) this + irq_assert) = vector;
	    ia64_mf ();
	}

    /**
     * Get version information for I/O SAPIC.
     * @return version info for I/O SAPIC
     */
    iosapic_version_t get_version (void)
	{
	    iosapic_version_t ver;
	    ver.raw = get (reg_version);
	    return ver;
	}

    /**
     * Get identifier for I/O SAPIC.
     * @return identifier for I/O SAPIC
     */
    u16_t get_id (void)
	{
	    return get (reg_id);
	}
};


#endif /* !__ARCH__IA64__IOSAPIC_H__ */
