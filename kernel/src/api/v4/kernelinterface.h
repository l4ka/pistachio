/*********************************************************************
 *                
 * Copyright (C) 2002-2007,  Karlsruhe University
 *                
 * File path:     api/v4/kernelinterface.h
 * Description:   Version 4 kernel-interface page
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
 * $Id: kernelinterface.h,v 1.28 2006/10/27 16:47:27 reichelt Exp $
 *                
 ********************************************************************/
#ifndef __API__V4__KERNELINTERFACE_H__
#define __API__V4__KERNELINTERFACE_H__

#if !defined(KIP)
#define KIP kip
typedef void (*kdebug_init_t)();
typedef void (*kdebug_entry_t)(void *);
#else
typedef addr_t kdebug_init_t;
typedef addr_t kdebug_entry_t;
#endif

#if !defined(KIP_SECTION)
#define KIP_SECTION "kip"
#endif

#include <generic/memregion.h>
#include INC_API(memdesc.h)
#include INC_API(procdesc.h)

class kernel_descriptor_t;

/**
 * Insert string into kernel feature string list.
 * @param str	feature string
 */
#define FEATURESTRING(str)							\
__asm__ (".section .data." KIP_SECTION ".features, \"aw\", %progbits	\n"	\
	 ".string \"" str "\"						\n"	\
	 ".previous							\n")


/**
 * descriptor for one of the initial servers (sigma0, root server, sigma1)
 */
class root_server_t
{
public:
    /** initial stack pointer, physical address */
    word_t		sp;
    /** initial instruction pointer, physical address */
    word_t		ip;
    /** memory region occupied by this server, physical addresses */
    mem_region_t	mem_region;
};

/**
 * info on location and number of memory descriptors
 */
class memory_info_t
{
public:
    union {
	struct {
	    BITFIELD2(word_t,
		      n		: BITS_WORD/2,
		      memdesc_ptr	: BITS_WORD/2
		);
	};
	word_t raw;
    };

    word_t get_num_descriptors (void) { return n; }

    memdesc_t * get_memdesc (word_t n);
    bool insert (memdesc_t::type_e type, word_t subtype,
		 bool virt, addr_t low, addr_t high);
    bool insert (memdesc_t::type_e type, bool virt, addr_t low, addr_t high)
	{ return insert (type, 0, virt, low, high); }
};

/**
 * Info for utcb size and allocation
 */
class utcb_info_t
{
public:
    BITFIELD4(word_t,
	      multiplier	: 10,
	      alignment		:  6,
	      size		:  6,
				: BITS_WORD - 22
	);
    
    word_t get_minimal_size()	
	{ return 1 << size; }
    word_t get_utcb_alignment()
	{ return 1 << alignment; }
    word_t get_utcb_size()
	{ return get_utcb_alignment() * multiplier; }
    bool is_valid_utcb_location(word_t utcb_location)
	{ return (((1 << alignment) - 1) & utcb_location) == 0; }
};

/**
 * info for kernel interface page size
 */
class kip_area_info_t
{
public:
    BITFIELD2(word_t,
	size	:  6,
		: BITS_WORD - 6
	);

    word_t get_size() { return (1 << size); }
    word_t get_size_log2() { return size; }
};

/**
 * clock_info_t: info for precision of system clock and scheduler
 */
class clock_info_t
{
public:
    BITFIELD3(word_t,
	read_precision		: 16,
	schedule_precision	: 16,
				: BITS_WORD - 32
	);

    word_t get_read_precision (void)	 { return read_precision;	}
    word_t get_schedule_precision (void) { return schedule_precision;	}
};

/**
 * thread_info_t: info for thread number ranges
 */
class thread_info_t
{
public:
    BITFIELD4(word_t,
	t		:  8,
	system_base	: 12,
	user_base	: 12,
			: BITS_WORD - 32
	);

    word_t get_user_base (void)			{ return user_base;	}
    word_t get_system_base (void)		{ return system_base;	}
    word_t get_significant_threadbits (void)	{ return t;		}
    void set_user_base(word_t base)		{ user_base = base;	}
    void set_system_base(word_t base)		{ system_base = base;	}
};

/**
 * info for supported page access rights and page sizes
 */
class page_info_t
{
public:
    BITFIELD3(word_t,
	rwx		:  3,
			:  7,
	size_mask	:  BITS_WORD - 7 - 3
	);

    word_t get_access_rights (void)	{ return rwx;			};
    word_t get_page_size_mask (void)	{ return size_mask << 10;	}
};

/**
 * info for processor descriptor array
 */
class processor_info_t
{
public:
    BITFIELD3(word_t,
	processors	: 16,
			: BITS_WORD - 16 - 4,
	size		: 4
	);
    word_t get_num_processors (void)	{ return processors + 1;	};
    word_t get_procdesc_size (void)	{ return 1 << size;		};
    procdesc_t * get_procdesc (word_t num);
};

/**
 * endianess and word size for current API
 */
class api_flags_t
{
public:
    BITFIELD3(word_t,
	endian		:  2,
	word_size	:  2,
			: BITS_WORD - 4
	);

    word_t get_endian (void)	{ return endian;	}
    word_t get_word_size (void)	{ return word_size;	}
    /** conversion to word_t */
    operator word_t() { return ((word_size << 2) | endian); }
};

/**
 * version/subversion of current API
 */
class api_version_t
{
public:
    BITFIELD4(word_t,
			: 16,
	subversion	: 8,
	version		: 8,
			: BITS_WORD - 32
	);

    word_t get_version (void)		{ return version;	}
    word_t get_subversion (void)	{ return subversion;	}
    /** conversion to word_t */
    operator word_t() { return ((version << 24) | (subversion << 16)); }
};

/**
 * The KIP magic - the "L4µK" byte string
 */
class magic_word_t
{
public:
    union {
	char string[4];
	word_t raw;
    };
};

/**
 * system call gate for users
 */
typedef word_t syscall_t;



/**
 * The kernel interface page (KIP)
 */
class kernel_interface_page_t 
{
public:
    /* --- functions --- */
    void init();
    kernel_descriptor_t * get_kernel_descriptor()
	{ return (kernel_descriptor_t*)((addr_word_t)this + kernel_desc_ptr); }

public:
    /* --- member variables --- */
    magic_word_t	magic;
    api_version_t	api_version;
    api_flags_t		api_flags;
    word_t		kernel_desc_ptr;

    /* kdebug */
    kdebug_init_t	kdebug_init;
    kdebug_entry_t	kdebug_entry;
    mem_region_t	kdebug_mem;

    /* root server */
    root_server_t	sigma0;
    root_server_t	sigma1;
    root_server_t	root_server;

    word_t		reserved0;
    memory_info_t	memory_info;
    word_t		kdebug_config[2];

    /* memory regions */
    mem_region_t	main_mem;
    mem_region_t	reserved_mem0;

    mem_region_t	reserved_mem1;
    mem_region_t	dedicated_mem0;

    mem_region_t	dedicated_mem1;
    mem_region_t	dedicated_mem2;

    mem_region_t	dedicated_mem3;
    mem_region_t	dedicated_mem4;

    /* info fields */
    word_t		reserved1[2];
    utcb_info_t		utcb_info;
    kip_area_info_t	kip_area_info;
    
    word_t		reserved2[2];
    word_t		boot_info;
    word_t		proc_desc_ptr;

    clock_info_t	clock_info;
    thread_info_t	thread_info;

    page_info_t		page_info;
    processor_info_t	processor_info;

    /* system calls */
    syscall_t		space_control_syscall;
    syscall_t		thread_control_syscall;
    syscall_t		processor_control_syscall;
    syscall_t		memory_control_syscall;

    syscall_t		ipc_syscall;
    syscall_t		lipc_syscall;
    syscall_t		unmap_syscall;
    syscall_t		exchange_registers_syscall;
    
    syscall_t		system_clock_syscall;
    syscall_t		thread_switch_syscall;
    syscall_t		schedule_syscall;
    word_t		reserved3[5];

    syscall_t		arch_syscall0;
    syscall_t		arch_syscall1;
    syscall_t		arch_syscall2;
    syscall_t		arch_syscall3;
};

extern "C" {
    extern kernel_interface_page_t KIP;
}

INLINE kernel_interface_page_t * get_kip()
{
    return &KIP;
}



/**
 * id/subid of kernel
 */
class kernel_id_t
{
public:
    BITFIELD4(word_t,
		: 16,
	subid	: 8,
	id	: 8,
		: BITS_WORD - 32
	);

    word_t get_subid (void)	{ return subid;	}
    word_t get_id (void)	{ return id;	}
    word_t get_raw (void)	{ return (id << 24) | (subid << 16); }
};

/**
 * kernel generation date
 */
class kernel_gen_date_t
{
public:
    BITFIELD4(word_t,
	day	:  5,
	month	:  4,
	year	:  7,
		: BITS_WORD - 16
	);

    word_t get_day (void)	{ return day;		}
    word_t get_month (void)	{ return month;		}
    word_t get_year (void)	{ return year + 2000;	}
};

/**
 * kernel version
 */
class kernel_version_t
{
public:
    BITFIELD4(word_t,
	subsubver	: 16,
	subver		: 8,
	ver		: 8,
			: BITS_WORD - 32
	);

    word_t get_subsubver (void)	{ return subsubver;	}
    word_t get_subver (void)	{ return subver;	}
    word_t get_ver (void)	{ return ver;		}
};

class kernel_supplier_t
{
public:
    union {
	/** 4 character string identifying kernel supplier (manufacturer).
	 *  It's whitespace-padded and NOT zero-terminated. */
	char string[4];
	/** placeholder to bloat structure to full word_t size */
	word_t raw;
    };
};

/**
 * kernel_descriptor_t: describes 
 */
class kernel_descriptor_t
{
public:
    kernel_id_t		kernel_id;
    kernel_gen_date_t	kernel_gen_date;
    kernel_version_t	kernel_version;
    kernel_supplier_t	kernel_supplier;
    char		version_parts[0];

    char * get_version_string (void) { return version_parts; }
};


/* From api/v4/init.cc */
void init_hello (void);


#endif /* !__API__V4__KERNELINTERFACE_H__ */
