/****************************************************************************
 *                
 * Copyright (C) 2002-2003, Karlsruhe University
 *                
 * File path:	include/piggybacker/ieee1275.h
 * Description:
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
 * $Id: ieee1275.h,v 1.4 2005/01/19 14:04:39 cvansch Exp $
 *
 ***************************************************************************/

#ifndef __PIGGYBACKER__INCLUDE__IEEE1275_H__
#define __PIGGYBACKER__INCLUDE__IEEE1275_H__

#include <l4/types.h>

#define __noreturn      __attribute__ ((noreturn))
#define __unused	__attribute__ ((unused))

#define PROM_DECL	extern

typedef L4_Word_t word_t;

typedef L4_Word32_t (*prom_entry_t)( void * );
typedef L4_Word32_t (*prom_callback_t)( void * );
typedef void * prom_handle_t;

typedef struct {
	L4_Word32_t service;
	union {
		struct {
			L4_Word32_t phys;
			L4_Word32_t virt;
			L4_Word32_t size;
			L4_Word32_t mode;
			L4_Word32_t error;
		} map;
		struct {
			L4_Word32_t virt;
			L4_Word32_t size;
		} unmap;
		struct {
			L4_Word32_t virt;
			L4_Word32_t error;
			L4_Word32_t real;
			L4_Word32_t mode;
		} translate;
	};
} prom_callback_args_t;


#define INVALID_PROM_HANDLE     ((prom_handle_t)-1)
#define ROOT_PROM_HANDLE	((prom_handle_t)0)

extern L4_Word32_t prom_entry( void * );
extern prom_handle_t prom_stdout;
extern prom_handle_t prom_stdin;
extern prom_handle_t prom_chosen;
extern prom_handle_t prom_options;
extern prom_handle_t prom_memory;

PROM_DECL prom_handle_t prom_instance_to_package( prom_handle_t ihandle );
PROM_DECL int prom_instance_to_path( prom_handle_t phandle, char *path, int pathlen );
PROM_DECL int prom_package_to_path( prom_handle_t phandle, char *path, int pathlen );
PROM_DECL int prom_next_prop( prom_handle_t node, const char *prev_name, char *name );
PROM_DECL int prom_write( prom_handle_t phandle, const void *buf, int len );
PROM_DECL int prom_read( prom_handle_t phandle, void *buf, int len );
PROM_DECL void prom_puts( const char *msg );
PROM_DECL prom_handle_t prom_find_device( const char *name );
PROM_DECL int prom_get_prop( prom_handle_t phandle, const char *name, void *buf, int buflen );
PROM_DECL int prom_get_prop_len( prom_handle_t phandle, const char *name );
PROM_DECL prom_handle_t prom_nav_tree( prom_handle_t node, const char *which );
PROM_DECL __noreturn void prom_exit( void );
PROM_DECL __noreturn void prom_fatal( const char *msg );
PROM_DECL void prom_enter( void );
PROM_DECL int prom_interpret( const char *forth );
PROM_DECL word_t prom_instantiate_rtas( word_t rtas_base_address );
PROM_DECL void prom_get_rom_range( int ranges[], unsigned len, int *cnt );
PROM_DECL void prom_quiesce( void );
PROM_DECL prom_callback_t prom_set_callback( prom_callback_t new_cb );

PROM_DECL void prom_init( word_t r5 );
PROM_DECL void get_prom_range( word_t *start, word_t *size );
PROM_DECL int prom_claim( unsigned long virt, unsigned long size, unsigned long align );
PROM_DECL int prom_map( void *phys, void *virt, L4_Word32_t size );
PROM_DECL int prom_unmap( void *phys, void *virt, L4_Word32_t size );

#endif	/* __PIGGYBACKER__INCLUDE__IEEE1275_H__ */

