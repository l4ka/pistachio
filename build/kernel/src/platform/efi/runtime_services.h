/*********************************************************************
 *                
 * Copyright (C) 2002,  Karlsruhe University
 *                
 * File path:     platform/efi/runtime_services.h
 * Description:   EFI runtime services
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
 * $Id: runtime_services.h,v 1.2 2003/09/24 19:04:55 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __PLATFORM__EFI__RUNTIME_SERVICES_H__
#define __PLATFORM__EFI__RUNTIME_SERVICES_H__

#include INC_PLAT(types.h)
#include INC_PLAT(memory_map.h)

class efi_time_t
{
public:
    u16_t	year;
    u8_t	month;
    u8_t	day;
    u8_t	hour;
    u8_t	minute;
    u8_t	second;
    u8_t	__pad1;
    u32_t	nanosecond;
    s16_t	timezone;
    u8_t	daylight;
    u8_t	__pad2;
};

/* Daylight definitions. */
#define EFI_TIME_ADJUST_DAYLIGHT	0x01
#define EFI_TIME_IN_DAYLIGHT		0x02

/* Timezone definitions. */
#define EFI_UNSPECIFIED_TIMEZONE	0x07ff

class efi_timecap_t
{
public:
    u32_t	resoultion;
    u32_t	accuracy;
    efi_bool_t	sets_to_zero;
};


/* Variable attributes. */
#define EFI_VARIABLE_NON_VOLATILE	0x01
#define EFI_VARIABLE_BOOTSERVICE_ACCESS	0x02
#define EFI_VARIABLE_RUNTIME_ACCESS	0x04


/**
 * efi_func_t: pointer to EFI runtime service function
 */
typedef word_t (*efi_func_t)(word_t, word_t, word_t, word_t, word_t);


#define __EFIERR(x)	((1UL << (sizeof (word_t) * 8 - 1)) | x)


/**
 * efi_runtime_services_t: table for EFI runtime services
 */
class efi_runtime_services_t
{
public:
    efi_table_header_t	hdr;

    /*
     * Function locations.
     */

    efi_func_t get_time_f;
    efi_func_t set_time_f;
    efi_func_t get_wakeup_time_f;
    efi_func_t set_wakeup_time_f;
    efi_func_t set_virtual_address_map_f;
    efi_func_t convert_pointer_f;
    efi_func_t get_variable_f;
    efi_func_t get_next_variable_f;
    efi_func_t set_variable_f;
    efi_func_t get_next_high_monotinic_count_f;
    efi_func_t reset_system_f;

    enum reset_type_e {
	cold,
	warm
    };

    enum status_e {
	success =		0,
	invalid_parameter =	__EFIERR(2),
	unsupported =		__EFIERR(3),
	device_error =		__EFIERR(7),
	not_found =		__EFIERR(14)
    };

    /*
     * Function wrappers.
     */

    status_e set_virtual_address_map (word_t memory_map_size,
				      word_t descriptor_size,
				      word_t descriptor_version,
				      efi_memory_desc_t * virt_map);

    void reset_system (efi_runtime_services_t::reset_type_e reset_type,
		       efi_runtime_services_t::status_e reset_status,
		       word_t data_size,
		       char16 * reset_data);
};


/*
 * Architecure dependent stub for performing physical EFI call.
 */

extern "C" efi_runtime_services_t::status_e
call_efi_physical (word_t function, word_t a0, word_t a1,
		   word_t a2, word_t a3, word_t a4);


/*
 * If IA-64 kernel is compiled with -mno-pic it will not be able to
 * handle EFI function pointers properly (i.e., it does not load GP).
 * We therefore invoke a wrapper function which implements the normal
 * calling conventions.
 */

#if __ARCH__ == ia64

extern "C" efi_runtime_services_t::status_e
call_efi (word_t function, word_t a0, word_t a1,
	  word_t a2, word_t a3, word_t a4);

#define CALL_EFI(func, a0, a1, a2, a3, a4) \
    call_efi ((word_t) func, a0, a1, a2, a3, a4)

#else /* __ARCH__ != ia64 */
#define CALL_EFI(func, a0, a1, a2, a3, a4) func (a0, a1, a2, a3, a4)
#endif



/*
 * Wrappers for EFI runtime service calls.
 */

INLINE efi_runtime_services_t::status_e
efi_runtime_services_t::set_virtual_address_map (word_t memory_map_size,
						 word_t descriptor_size,
						 word_t descriptor_version,
						 efi_memory_desc_t * virt_map)
{
    return call_efi_physical ((word_t) set_virtual_address_map_f,
			      memory_map_size, descriptor_size,
			      descriptor_version, (word_t) virt_map, 0);
}


INLINE void
efi_runtime_services_t::reset_system (efi_runtime_services_t::reset_type_e rt,
				      efi_runtime_services_t::status_e rs,
				      word_t data_size,
				      char16 * reset_data)
{
    (void) CALL_EFI (reset_system_f, (word_t) rt, (word_t) rs, data_size,
		     (word_t) reset_data, 0);
}

		    

#endif /* !__PLATFORM__EFI__RUNTIME_SERVICES_H__ */
