/*********************************************************************
 *
 * Copyright (C) 2003, University of New South Wales
 *
 * File path:    openfirmware/openfirmware.h
 * Description:  Open Firmware (IEEE std 1275) bootloader specifics.
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
 * $Id: openfirmware.h,v 1.3 2003/09/24 19:06:14 skoglund Exp $
 *
 ********************************************************************/

#ifndef __OPENFIRMWARE__OPENFIRMWARE_H__
#define __OPENFIRMWARE__OPENFIRMWARE_H__

#include <openfirmware/types.h>

/* Buffer for loading parameters into */
#define BUFFER_SIZE 1024
extern char buffer[BUFFER_SIZE];

/**********************
* Function Prototypes *
**********************/

/* Archictecture specific functions */
extern "C" void ofw_entry(void * args);       /* Entry point openfirmware */
extern "C" void ofw_panic(ofw_panic_t error); /* Unrecoverable error      */

/* Device Tree */

ofw_phandle_t ofw_peer(ofw_phandle_t peer);
ofw_phandle_t ofw_child(ofw_phandle_t parent);
ofw_phandle_t ofw_parent(ofw_phandle_t child);

ofw_cell_t ofw_nextprop(ofw_phandle_t package, ofw_string_t previous,
			ofw_buffer_t buffer);
ofw_cell_t ofw_getproplen(ofw_phandle_t package, ofw_string_t property);
ofw_cell_t ofw_getprop(ofw_phandle_t package, ofw_string_t property,
		       ofw_buffer_t buffer, ofw_cell_t buffer_size);
ofw_cell_t ofw_setprop(ofw_phandle_t package, ofw_string_t property,
		       ofw_buffer_t buffer, ofw_cell_t buffer_size);

ofw_cell_t    ofw_canon(ofw_string_t device, ofw_buffer_t buffer,
			ofw_cell_t buffer_size);
ofw_phandle_t ofw_finddevice(ofw_string_t device);

ofw_phandle_t ofw_instance2package(ofw_ihandle_t instance);
ofw_cell_t    ofw_instance2path(ofw_ihandle_t instance, ofw_buffer_t buffer,
				ofw_cell_t buffer_size);
ofw_cell_t    ofw_package2path(ofw_phandle_t package, ofw_buffer_t buffer,
			       ofw_cell_t buffer_size);

ofw_cell_t ofw_call_method(ofw_string_t method, ofw_ihandle_t instance,
			   ofw_cell_t num_args, ofw_cell_t * args, 
			   ofw_cell_t num_regs, ofw_cell_t * results);

/* Device I/O */

ofw_ihandle_t ofw_open(ofw_string_t device);
void          ofw_close(ofw_ihandle_t device);

ofw_cell_t ofw_read(ofw_ihandle_t device, ofw_buffer_t buffer,
		    ofw_cell_t lenght);
ofw_cell_t ofw_write(ofw_ihandle_t device, ofw_buffer_t buffer,
		     ofw_cell_t length);
ofw_cell_t ofw_seek(ofw_ihandle_t device, ofw_cell_t pos_hi, ofw_cell_t pos_lo);

/* Memory */

ofw_addr_t ofw_claim(ofw_addr_t vaddr, ofw_cell_t size, ofw_cell_t align);
void       ofw_release(ofw_addr_t vaddr, ofw_cell_t size);

/* Control Transfer */

void ofw_boot(ofw_string_t bootspec);
void ofw_enter(void);
void ofw_exit(void);
void ofw_chain(void * vaddr, ofw_cell_t size,
	       void * entry, void * args, ofw_cell_t len);

/* User Interface */

ofw_cell_t ofw_interpret(ofw_string_t command,
			 ofw_cell_t num_args, ofw_cell_t * args,
			 ofw_cell_t num_rets, ofw_cell_t * returns);
ofw_addr_t ofw_setcallback(ofw_addr_t newfunc);


#endif /* !__OPENFIRMWARE__OPENFIRMWARE_H__ */
