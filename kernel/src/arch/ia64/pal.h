/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     arch/ia64/pal.h
 * Description:   PAL call wrapper functions
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
 * $Id: pal.h,v 1.13 2003/09/24 19:04:28 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__IA64__PAL_H__
#define __ARCH__IA64__PAL_H__

#include INC_ARCH(rr.h)
#include INC_GLUE(hwspace.h)
#include INC_ARCH(mc.h)

extern addr_t ia64_pal_code;
extern addr_t ia64_pal_entry;


/**
 * Index values to identify PAL procedures
 */
enum pal_idx_e {
    PAL_CACHE_INFO	= 2,
    PAL_CACHE_SUMMARY	= 4,
    PAL_DEBUG_INFO	= 11,
    PAL_FREQ_BASE	= 13,
    PAL_FREQ_RATIOS	= 14,
    PAL_HALT		= 28,
    PAL_HALT_LIGHT	= 29,
    PAL_HALT_INFO	= 257,
    PAL_MC_CLEAR_LOG	= 21,
    PAL_MC_ERROR_INFO	= 25,
    PAL_MC_REGISTER_MEM	= 27,
    PAL_PERF_MON_INFO	= 15,
    PAL_PTCE_INFO	= 6,
    PAL_SHUTDOWN	= 40,
    PAL_VM_INFO		= 7,
    PAL_VM_PAGE_SIZE	= 34,
    PAL_VM_SUMMARY	= 8,
    PAL_VM_TR_READ	= 261,
};


/**
 * pal_status_e: return status of PAL calls
 */
enum pal_status_e {
    PAL_OK		=  0,
    PAL_UNIMPLEMENTED	= -1,
    PAL_INVALID_ARG	= -2,
    PAL_ERROR		= -3
};


/**
 * pal_return_t: four word structure containing return values of PAL calls
 */
class pal_return_t
{
public:
    union {
	pal_status_e status;
	word_t raw[4];
    };
};


extern "C" pal_return_t
call_pal_static (pal_idx_e idx, word_t a1, word_t a2, word_t a3);

extern "C" pal_return_t
call_pal_stacked (pal_idx_e idx, word_t a1, word_t a2, word_t a3);

extern "C" pal_return_t
call_pal_static_phys (pal_idx_e idx, word_t a1, word_t a2, word_t a3);

extern "C" pal_return_t
call_pal_stacked_phys (pal_idx_e idx, word_t a1, word_t a2, word_t a3);



/*
 * PAL_CACHE_INFO
 */

class pal_cache_info_t
{
public:
    enum type_e {
	none	= 0,
	code	= 1,
	data	= 2
    };

    union {
	struct {
	    word_t unified		: 1;
	    word_t attributes		: 2;
	    word_t __rsv1 		: 5;
	    word_t associativity	: 8;
	    word_t line_size		: 8;
	    word_t stride		: 8;
	    word_t store_latency	: 8;
	    word_t load_latency		: 8;
	    word_t store_hints		: 8;
	    word_t load_hints		: 8;

	    word_t cache_size		: 32;
	    word_t alias_boundary	: 8;
	    word_t tag_ls_bit		: 8;
	    word_t tag_ms_bit		: 8;
	    word_t __rsv2 		: 8;
	};
	word_t raw[2];
    };
};

INLINE pal_status_e pal_cache_info (word_t level,
				    pal_cache_info_t::type_e type,
				    pal_cache_info_t * info)
{
    pal_return_t ret = call_pal_static (PAL_CACHE_INFO,
					level, (word_t) type, 0);
    info->raw[0] = ret.raw[1];
    info->raw[1] = ret.raw[2];
    return ret.status;
}


/*
 * PAL_CACHE_SUMMARY
 */

class pal_cache_summary_t
{
public:
    union {
	struct {
	    u64_t cache_levels;
	    u64_t unique_caches;
	};
	word_t raw[2];
    };
};

INLINE pal_status_e pal_cache_summary (pal_cache_summary_t * summary)
{
    pal_return_t ret = call_pal_static (PAL_CACHE_SUMMARY, 0, 0, 0);
    summary->raw[0] = ret.raw[1];
    summary->raw[1] = ret.raw[2];
    return ret.status;
}


/*
 * PAL_DEBUG_INFO
 */

INLINE pal_status_e pal_debug_info (word_t * iregs, word_t * dregs)
{
    pal_return_t ret = call_pal_static (PAL_DEBUG_INFO, 0, 0, 0);
    *iregs = ret.raw[1];
    *dregs = ret.raw[2];
    return ret.status;
}


/*
 * PAL_FREQ_BASE
 */

INLINE pal_status_e pal_freq_base (word_t * base_freq)
{
    pal_return_t ret = call_pal_static (PAL_FREQ_BASE, 0, 0, 0);
    *base_freq = ret.raw[1];
    return ret.status;
}


/*
 * PAL_FREQ_RATIOS
 */

INLINE pal_status_e pal_freq_ratios (word_t * proc_ratio, word_t * bus_ratio,
				     word_t * itc_ratio)
{
    pal_return_t ret = call_pal_static (PAL_FREQ_RATIOS, 0, 0, 0);
    *proc_ratio = ret.raw[1];
    *bus_ratio  = ret.raw[2];
    *itc_ratio  = ret.raw[3];
    return ret.status;
}


/*
 * PAL_HALT_INFO
 */

class pal_power_info_t
{
public:
    class power_state_t
    {
    public:
	word_t exit_latency			: 16;
	word_t entry_latency			: 16;
	word_t power_consumption		: 28;
	word_t implemented			: 1;
	word_t coherent				: 1;
	word_t __reserved			: 2;
    };

    power_state_t get_power_state(word_t state)
	{
	    return power_state[state];
	}

private:
    power_state_t power_state[8];
};

INLINE pal_status_e pal_halt_info (pal_power_info_t * power_info)
{
    pal_return_t ret = call_pal_stacked (PAL_HALT_INFO, 
					 (word_t) power_info,
					 0, 0);
    return ret.status;
}


/*
 * PAL_MC_CLEAR_LOG
 */

INLINE pal_status_e pal_mc_clear_log (bool * pend_mc = NULL,
				      bool * pend_init = NULL)
{
    pal_return_t ret = call_pal_static (PAL_MC_CLEAR_LOG, 0, 0, 0);
    if (pend_mc)
	*pend_mc = (ret.raw[1] & 1);
    if (pend_init)
	*pend_init = (ret.raw[1] & 2);
    return ret.status;
}


/*
 * PAL_MC_ERROR_INFO
 */

class pal_mc_level_index_t
{
public:
    union {
	struct {
	    word_t cid			: 4;
	    word_t tid			: 4;
	    word_t eic			: 4;
	    word_t edc			: 4;
	    word_t eit			: 4;
	    word_t edt			: 4;
	    word_t ebh			: 4;
	    word_t erf			: 4;
	    word_t ems			: 16;
	    word_t __reserved		: 16;
	};
	word_t raw;
    };
};

class pal_mc_tlb_check_t
{
public:
    union {
	struct {
	    word_t tr_slot		: 8;
	    word_t __reserved1		: 8;
	    word_t dtr			: 1;
	    word_t itr			: 1;
	    word_t dtc			: 1;
	    word_t itc			: 1;
	    word_t mc			: 1;
	    word_t __reserved2		: 43;
	};
	word_t raw;
    };
};

INLINE pal_status_e pal_mc_error_info_errmap (pal_mc_level_index_t * errmap)
{
    pal_return_t ret = call_pal_static (PAL_MC_ERROR_INFO, 0, 0, 0);
    errmap->raw = ret.raw[1];
    return ret.status;
}

INLINE pal_status_e pal_mc_error_info_status (processor_state_t * state)
{
    pal_return_t ret = call_pal_static (PAL_MC_ERROR_INFO, 1, 0, 0);
    state->raw = ret.raw[1];
    return ret.status;
}

INLINE pal_status_e pal_mc_error_info (pal_mc_level_index_t level_idx,
				       bool * more,
				       word_t * info,
				       addr_t * target_addr,
				       word_t * requestor,
				       word_t * responder,
				       addr_t * ip)
{
    pal_return_t ret;

    ret = call_pal_static (PAL_MC_ERROR_INFO, 2, level_idx.raw, 0);
    *info = ret.raw[1];
    ret = call_pal_static (PAL_MC_ERROR_INFO, 2, level_idx.raw, 1);
    *target_addr = (addr_t) ret.raw[1];
    ret = call_pal_static (PAL_MC_ERROR_INFO, 2, level_idx.raw, 2);
    *requestor = ret.raw[1];
    ret = call_pal_static (PAL_MC_ERROR_INFO, 2, level_idx.raw, 3);
    *responder = ret.raw[1];
    ret = call_pal_static (PAL_MC_ERROR_INFO, 2, level_idx.raw, 4);
    *ip = (addr_t) ret.raw[1];

    *more = (ret.raw[2] != 0);
    return ret.status;
}

INLINE pal_status_e pal_mc_error_info_itlb (word_t idx, bool * more,
					    pal_mc_tlb_check_t * info,
					    addr_t * target_addr,
					    word_t * requestor,
					    word_t * responder,
					    addr_t * ip)
{
    pal_mc_level_index_t level_idx;
    level_idx.raw = 0;
    level_idx.eit = idx;
    return pal_mc_error_info (level_idx, more, &info->raw, target_addr,
			      requestor, responder, ip);
}

INLINE pal_status_e pal_mc_error_info_dtlb (word_t idx, bool * more,
					    pal_mc_tlb_check_t * info,
					    addr_t * target_addr,
					    word_t * requestor,
					    word_t * responder,
					    addr_t * ip)
{
    pal_mc_level_index_t level_idx;
    level_idx.raw = 0;
    level_idx.edt = idx;
    return pal_mc_error_info (level_idx, more, &info->raw, target_addr,
			      requestor, responder, ip);
}


/*
 * PAL_MC_REGISTER_MEM
 */

INLINE pal_status_e pal_mc_register_mem (addr_t addr)
{
    pal_return_t ret = call_pal_static_phys (PAL_MC_REGISTER_MEM,
					     (word_t) addr, 0, 0);
    return ret.status;
}


/*
 * PAL_HALT_LIGHT
 */

INLINE pal_status_e pal_halt_light (void)
{
    pal_return_t ret = call_pal_stacked (PAL_HALT_LIGHT, 0, 0, 0);
    return ret.status;
}


/*
 * PAL_PERF_MON_INFO
 */

class pal_perf_mon_info_t
{
public:
    union {
	struct {
	    word_t generic		: 8;
	    word_t width		: 8;
	    word_t cycles		: 8;
	    word_t retired		: 8;
	    word_t __pad		: 32;
	};
	word_t raw;
    };
};

class pal_perf_mon_masks_t
{
    u64_t implemented_pmc_mask[4];
    u64_t implemented_pmd_mask[4];
    u64_t count_cycles_mask[4];
    u64_t count_bundles_mask[4];

public:

    bool is_pmc_implemented (word_t n)
	{ return implemented_pmc_mask[n / 64] & (1UL << (n & 63)); }

    bool is_pmd_implemented (word_t n)
	{ return implemented_pmd_mask[n / 64] & (1UL << (n & 63)); }

    bool can_count_cycles (word_t n)
	{ return count_cycles_mask[n / 64] & (1UL << (n & 63)); }

    bool can_count_retired_bundles (word_t n)
	{ return count_bundles_mask[n / 64] & (1UL << (n & 63)); }
};

INLINE pal_status_e pal_perf_mon_info (pal_perf_mon_masks_t * mask,
				       pal_perf_mon_info_t * info)
{
    pal_return_t ret = call_pal_static (PAL_PERF_MON_INFO,
					(word_t) mask, 0, 0);
    info->raw = ret.raw[1];
    return ret.status;
}


/*
 * PAL_PTCE_INFO
 */

class pal_ptce_info_t
{
public:
    union {
	struct {
	    u64_t	base;
	    u32_t	count1;
	    u32_t	count2;
	    u32_t	stride1;
	    u32_t	stride2;
	};
	u64_t raw[3];
    };
};

INLINE pal_status_e pal_ptce_info (pal_ptce_info_t * info)
{
    pal_return_t ret = call_pal_static (PAL_PTCE_INFO, 0, 0, 0);
    info->raw[0] = ret.raw[1];
    info->raw[1] = ret.raw[2];
    info->raw[2] = ret.raw[3];
    return ret.status;
}


/*
 * PAL_VM_INFO
 */

class pal_vm_info_t
{
public:
    enum type_e {
	none = 0,
	code = 1,
	data = 2
    };

    union {
	struct {
	    word_t num_sets		: 8;
	    word_t num_ways		: 8;
	    word_t num_entries		: 16;
	    word_t preferred_size	: 1;
	    word_t unified		: 1;
	    word_t tr			: 1;
	    word_t __pad		: 29;
	    word_t page_size_mask	: 64;
	};
	word_t raw[2];
    };
};

INLINE pal_status_e pal_vm_info (word_t tc_level, pal_vm_info_t::type_e type,
				 pal_vm_info_t * info)
{
    pal_return_t ret = call_pal_static (PAL_VM_INFO, tc_level,
					(word_t) type, 0);
    info->raw[0] = ret.raw[1];
    info->raw[1] = ret.raw[2];
    return ret.status;
}


/*
 * PAL_VM_PAGE_SIZE
 */
INLINE pal_status_e pal_vm_page_size (word_t * insertable, word_t * purgeable)
{
    pal_return_t ret = call_pal_static (PAL_VM_PAGE_SIZE, 0, 0, 0);
    *insertable = ret.raw[1];
    *purgeable = ret.raw[2];
    return ret.status;
}


/*
 * PAL_VM_SUMMARY
 */

class pal_vm_summary_t
{
public:
    union {
	struct {
	    word_t walker_present			: 1;
	    word_t phys_address_bits_implemented	: 7;
	    word_t protection_key_size			: 8;
	    word_t max_pkr				: 8;
	    word_t hash_tag_id				: 8;
	    word_t max_dtr_entry			: 8;
	    word_t max_itr_entry			: 8;
	    word_t max_unique_tcs			: 8;
	    word_t num_tc_levels			: 8;
	    word_t impl_virtual_addtress_msb		: 8;
	    word_t rid_size				: 8;
	};
	word_t raw[2];
    };
};

INLINE pal_status_e pal_vm_summary (pal_vm_summary_t * summary)
{
    pal_return_t ret = call_pal_static (PAL_VM_SUMMARY, 0, 0, 0);
    summary->raw[0] = ret.raw[1];
    summary->raw[1] = ret.raw[2];
    return ret.status;
}


/*
 * PAL_VM_TR_READ
 */

class pal_vm_tr_read_t
{
public:
    union {
	struct {
	    word_t access_right_valid		: 1;
	    word_t privilege_level_valid	: 1;
	    word_t dirty_bit_valid		: 1;
	    word_t memory_attributes_valid	: 1;
	};
	word_t raw;
    };
};

INLINE pal_status_e pal_vm_tr_read (word_t reg_num, word_t type,
				    addr_t buffer, pal_vm_tr_read_t * tr_valid)
{
    pal_return_t ret = call_pal_stacked_phys (PAL_VM_TR_READ, reg_num, type,
					      virt_to_phys ((word_t) buffer));
    tr_valid->raw = ret.raw[1];
    return ret.status;
}

#endif /* !__ARCH__IA64__PAL_H__ */
