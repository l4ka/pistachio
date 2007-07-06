/*********************************************************************
 *
 * Copyright (C) 2003, University of New South Wales
 *
 * File path:    openfirmware/types.h
 * Description:  Open Firmware bootloader specific types.
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
 * $Id: types.h,v 1.3 2003/09/24 19:06:14 skoglund Exp $
 *
 ********************************************************************/

#ifndef __OPENFIRMWARE__TYPES_H__
#define __OPENFIRMWARE__TYPES_H__

#include <openfirmware/macros.h>
#include OFW_INC_ARCH(types.h)

typedef ofw_cell_t ofw_phandle_t; /* package handle  */
typedef ofw_cell_t ofw_ihandle_t; /* instance handle */
typedef ofw_cell_t ofw_string_t;  /* string pointer  */
typedef ofw_cell_t ofw_buffer_t;  /* buffer pointer  */
typedef ofw_cell_t ofw_addr_t;    /* Address         */

#define OFW_NULL_PHANDLE ((ofw_phandle_t)0)

/* Panic values */

typedef enum {
  OFW_ERROR_NOSTART = 1,     // Problem in architecture specific startup asm.
  OFW_ERROR_NOCHOSEN,        // Trouble opening "/chosen".
  OFW_ERROR_NOSTDOUT         // Trouple opening stdout.
} ofw_panic_t;

typedef enum {
  L4_OFW_ReservedMem = 1 // Memory used by the firmware.
} L4_ofw_memdesc_t;


#endif /* !__OPENFIRMWARE__TYPES_H__ */
