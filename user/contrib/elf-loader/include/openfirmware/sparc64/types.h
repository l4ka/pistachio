/*********************************************************************
 *
 * Copyright (C) 2003,  University of New South Wales
 *
 * File path:      openfirmware/sparc64/types.h
 * Description:    Open Firmware bootloader SPARC v9 specific types.
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
 * $Id: types.h,v 1.2 2003/09/24 19:06:15 skoglund Exp $
 *
 ********************************************************************/
#ifndef __OPENFIRMWARE__SPARC64__TYPES_H__
#define __OPENFIRMWARE__SPARC64__TYPES_H__

#include <l4/types.h>

/********
* Types *
********/

typedef L4_Word_t ofw_cell_t; /* Sparc v9 firmware is 64-bit */

/*******************
* Type conversions *
*******************/

#define ADDR2OFW_STRING(x) ((ofw_string_t)(x))
#define ADDR2OFW_BUFFER(x) ((ofw_buffer_t)(x))
#define ADDR2OFW_ADDR(x)   ((ofw_addr_t)(x))

#define INT2OFW_CELL(x) ((ofw_cell_t)(x))


#endif /* !__OPENFIRMWARE__SPARC64__TYPES_H__ */
