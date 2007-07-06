/*********************************************************************
 *                
 * Copyright (C) 2002,  Karlsruhe University
 *                
 * File path:     arch/ia64/sal.h
 * Description:   IA-64 System Abstraction Layer
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
 * $Id: sal.h,v 1.7 2003/09/24 19:04:28 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__IA64__SAL_H__
#define __ARCH__IA64__SAL_H__


/**
 * sal_system_table_t: header for SAL system table
 */
class sal_system_table_t
{
public:
    union {
	u32_t		raw;
	char		string[4];
    } signature;

    u32_t		total_table_length;

    union {
	u16_t		raw;
	struct {
	    u8_t 	minor;
	    u8_t	major;
	} x;
    } sal_revision;

    u16_t		entry_count;

    u8_t		checksum;
    u8_t		__reserved1[7];

    union {
	u16_t		raw;
	struct {
	    u8_t 	minor;
	    u8_t	major;
	} x;
    } sal_a_version;

    union {
	u16_t		raw;
	struct {
	    u8_t 	minor;
	    u8_t	major;
	} x;
    } sal_b_version;

    char		oem_id[32];
    char		product_id[32];
    u8_t		__reserved2[8];

    inline bool is_valid (void)
	{ return signature.string[0] == 'S' && signature.string[1] == 'S' &&
	      signature.string[2] == 'T' && signature.string[3] == '_'; }

};


/**
 * sal_entrypoint_desc_t: SAL table entry containing entrypoints
 */
class sal_entrypoint_desc_t
{
public:
    u8_t		type;
    u8_t		__reserved1[7];
    addr_t		pal_proc;
    addr_t		sal_proc;
    addr_t		sal_global_data;
    u8_t		__reserved2[16];
};

/**
 * Translation Register Descriptor
 */
class sal_trans_reg_desc_t
{
public:
    u8_t		type;
    u8_t		reg_type;
    u8_t		reg_num;
    u8_t		__reserved1[5];
    u64_t		covered_area;
    u64_t		page_size;
    u8_t		__reserved2[8];
};


/**
 * Platform feature descriptor entry
 */
class sal_feature_desc_t
{
public:
    u8_t	type;
    union {
	u8_t raw;
	BITFIELD4 (	u8_t,
			bus_lock		: 1,
			irq_redirection_hint	: 1,
			ipi_redirection_hint	: 1,
			__reserved1		: 5);
    };
    u8_t	__reserved2[2];

    bool has_buslock() 
	{ return bus_lock == 1; }

    bool has_irq_redirection_hint()
	{ return irq_redirection_hint == 1; }

    bool has_ipi_redirection_hint()
	{ return ipi_redirection_hint == 1; }
};

/**
 * Purge translation cache coherency domain entry
 */
class sal_coherency_domain_info_t
{
public:
    u64_t		num_processors;
    word_t *		processor_ids;
};

class sal_purge_tc_domain_desc_t
{
public:
    u8_t				type;
    u8_t				__reserved[3];
    u32_t				num_domains;
    sal_coherency_domain_info_t *	domain_info;
};

/** 
 * Application processor wakeup entry
 */
class sal_ap_wakeup_desc_t 
{
public:
    u8_t		type;
    u8_t		wakeup_mechanism;
    u8_t		__reserved[6];
    u64_t		irq_vector;
};


extern sal_system_table_t * ia64_sal_table;


/**
 * Function identifiers for SAL procedures.
 */
enum sal_function_id_e {
    SAL_SET_VECTORS		= 0x01000000,
    SAL_GET_STATE_INFO		= 0x01000001,
    SAL_GET_STATE_INFO_SIZE	= 0x01000002,
    SAL_CLEAR_STATE_INFO	= 0x01000003,
    SAL_MC_RENDEZ		= 0x01000004,
    SAL_MC_SET_PARAMS		= 0x01000005,
    SAL_REGISTER_PHYSICAL_ADDR	= 0x01000006,
    SAL_CACHE_FLUSH		= 0x01000008,
    SAL_CACHE_INIT		= 0x01000009,
    SAL_PCI_CONFIG_READ		= 0x01000010,
    SAL_PCI_CONFIG_WRITE	= 0x01000011,
    SAL_FREQ_BASE		= 0x01000012,
    SAL_UPDATE_PAL		= 0x01000020,
};


/**
 * Return status of SAL calls
 */
enum sal_status_e {
    SAL_OK		=  0,
    SAL_UNIMPLEMENTED	= -1,
    SAL_INVALID_ARG	= -2,
    SAL_ERROR		= -3,
    SAL_VADDR_NOTREG	= -4,
};


/**
 * Four word structure containing return values of SAL calls.
 */
class sal_return_t
{
public:
    union {
	sal_status_e status;
	word_t raw[4];
    };
};

typedef sal_return_t (*ia64_sal_func_t)
    (sal_function_id_e id, word_t a1, word_t a2, word_t a3,
     word_t a4, word_t a5, word_t a6, word_t a7);

extern ia64_sal_func_t ia64_sal_entry;


/*
 * SAL_FREQ_BASE
 */

class sal_freq_base_t
{
public:
    enum type_e {
	platform	= 0,
	itc		= 1,
	rtc		= 2
    };
};

INLINE sal_status_e sal_freq_base (sal_freq_base_t::type_e type,
				   word_t * clock_freq, word_t * drift_info)
{
    sal_return_t ret = ia64_sal_entry (SAL_FREQ_BASE, (word_t) type,
				       0, 0, 0, 0, 0, 0);

    *clock_freq = ret.raw[1];
    *drift_info = ret.raw[2];
    return ret.status;
}

/* 
 * SAL_SET_VECTORS
 */
class sal_vector_t
{
public:
    enum type_e {
	machine_check	= 0,
	init		= 1,
	boot_rendez	= 2
    };
};

INLINE sal_status_e sal_set_vectors (sal_vector_t::type_e type,
				     addr_t phys_addr_1, addr_t gp_1,
				     u32_t length_1,
				     addr_t phys_addr_2, addr_t gp_2,
				     u32_t length_2)
{
    sal_return_t ret = ia64_sal_entry (SAL_SET_VECTORS, type,
				       (word_t) phys_addr_1, (word_t) gp_1,
				       (word_t) length_1,
				       (word_t) phys_addr_2, (word_t) gp_2,
				       (word_t) length_2);
    return ret.status;
}

/**
 * Set the rendezvous point for application processors.
 * @param rendez_func		rendevous function
 * @param length		length of rendevous procedure
 */
INLINE sal_status_e sal_set_boot_rendez (void (*rendez_func)(),
					 u32_t length)
{
    addr_t ip = (addr_t) ((word_t *) rendez_func)[0];
    addr_t gp = (addr_t) ((word_t *) rendez_func)[1];

    return sal_set_vectors (sal_vector_t::boot_rendez,
			    ip, gp, length, 0, 0, 0);
}
				    


#endif /* !__ARCH__IA64__SAL_H__ */
