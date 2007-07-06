/*********************************************************************
 *                
 * Copyright (C) 2003,  Karlsruhe University
 *                
 * File path:     arch/ia64/itanium/perf_branchtrace.h
 * Description:   Itanium Branch Trace Buffer support
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
 * $Id: itanium_perf_branchtrace.h,v 1.2 2003/09/24 19:04:28 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__IA64__ITANIUM__PERF_BRANCHTRACE_H__
#define __ARCH__IA64__ITANIUM__PERF_BRANCHTRACE_H__

#include INC_ARCH(perf.h)


/**
 * Wrapper class for accessing the Itanium branch trace buffer
 * configuration register.
 */
class pmc_branch_trace_config_t
{

    union {
	struct {
	    word_t plm		: 4;
	    word_t __ig1	: 2;
	    word_t pm		: 1;
	    word_t tar		: 1;
	    word_t tm		: 2;
	    word_t ptm		: 2;
	    word_t ppm		: 2;
	    word_t bpt		: 1;
	    word_t bac		: 1;
	    word_t __ig2	: 48;
	};
	word_t raw;
    };

public:

    enum taken_e {
	none		= 0,
	not_taken	= 1,
	taken		= 2,
	all		= 3,
    };

    enum predicted_target_e {
	no_targets		= 0,
	mispredicted_targets 	= 1,
	predicted_targets	= 2,
	all_targets		= 3,
    };

    enum predicted_pred_e {
	no_preds		= 0,
	mispredicted_preds 	= 1,
	predicted_preds		= 2,
	all_preds		= 3,
    };

    pmc_branch_trace_config_t (void)
	{ raw = get_pmc (12); }

    pmc_branch_trace_config_t (pmc_t::priv_e priv,
			       bool priv_mon,
			       bool target_addr,
			       taken_e taken_mask,
			       predicted_target_e predicted_target_mask,
			       predicted_pred_e predicted_pred_mask,
			       bool tac_predictions,
			       bool bac_predictions)
	{
	    plm		= (word_t) priv;
	    pm		= priv_mon;
	    tar		= target_addr;
	    tm		= (word_t) taken_mask;
	    ptm		= (word_t) predicted_target_mask;
	    ppm		= (word_t) predicted_pred_mask;
	    bpt		= tac_predictions;
	    bac		= bac_predictions;
	}

    void disable (void)
	{ plm = 0; set_pmc (12, raw); }
    
    void activate (void)
	{ set_pmc (12, raw); }
};


/**
 * Wrapper class for accessing the Itanium branch trace buffer
 * registers.
 */
class pmd_branch_trace_t
{
    union {
	struct {
	    word_t b		: 1;
	    word_t mp		: 1;
	    word_t tr_slot	: 2;
	    word_t tr_address	: 60;
	};
	word_t raw;
    };

public:

    /**
     * Initialize a branch trace value with the indicated register
     * number.
     * @param n PMD register number
     */
    pmd_branch_trace_t (word_t n)
	{ raw = get_pmd (n); }

    /**
     * @return true if branch trace is invalid, false otherwiswe
     */
    bool is_invalid (void)
	{ return b == 0 && mp == 0; }

    /**
     * @return true if branch trace is a mispredicted branch, false
     * otherwise
     */
    bool is_mispredicted (void)
	{ return b == 1 && mp == 1; }

    /**
     * @return true if branch trace is a correctly predicted branch,
     * false otherwise
     */
    bool is_predicted (void)
	{ return b == 1 && mp == 0; }

    /**
     * @return true if branch trace is a branch target, false otherwise
     */
    bool is_target (void)
	{ return b == 0 && mp == 1; }

    /**
     * @return address of branch instruction (or branch target)
     */
    addr_t address (void)
	{ return (addr_t) (tr_address << 4); }

    /**
     * @return slot number of branch instruction, or 0 if branch trace
     * is a target
     */
    word_t slot (void)
	{ return tr_slot; }

    /**
     * @return canonical address of branch instruction or branch
     * target (i.e., instruction address as seen in objdump)
     */
    addr_t canonical_address (void)
	{ return (addr_t) ((tr_address << 4) + (tr_slot * 6)); }
};


/**
 * Wrapper class for accessing the Itanium branch trace buffer index
 * register.
 */
class pmd_branch_trace_index_t
{
    union {
	struct {
	    word_t bbi		: 3;
	    word_t full		: 1;
	    word_t __ig		: 60;
	};
	word_t raw;
    };

public:

    pmd_branch_trace_index_t (void)
	{ raw = get_pmd (16); }

    void clear (void)
	{ raw = 0; set_pmd (16, 0); }

    word_t num_entries (void)
	{ return full ? 8 : bbi; }

    word_t oldest_idx (void)
	{ return full ? 8 + bbi: 8; }

    word_t newest_idx (void)
	{ return 8 + (((8 * full) + (bbi - 1)) & 7); }

    word_t next_idx (word_t n)
	{ return 8 + ((n + 1) & 7); }
};


#endif /* !__ARCH__IA64__ITANIUM__PERF_BRANCHTRACE_H__ */
