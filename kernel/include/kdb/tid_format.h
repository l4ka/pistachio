/*********************************************************************
 *
 * Copyright (C) 2002-2004,  Karlsruhe University
 *
 * File path:     include/kdb/tid_format.h
 * Description:   Common types of %t (thread ID) printf format
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
 * $Id: tid_format.h,v 1.1 2004/06/02 08:37:11 sgoetz Exp $
 *
 ********************************************************************/
#include <types.h>

enum tid_format_value {
    TID_FORMAT_VALUE_GID,
    TID_FORMAT_VALUE_TCB,
    TID_FORMAT_VALUE_BOTH
};

enum tid_format_version {
    TID_FORMAT_VERSION_INLINE,
    TID_FORMAT_VERSION_SEP,
    TID_FORMAT_VERSION_OFF
};

typedef union {
    struct {
	word_t value: 2; // print global IDs or TCB addresses or both
	word_t human: 1; // print special threads as human readable or hex
	word_t version: 2; // format of version field
	word_t sep: 5; // add separator before sep lowermost bits, 0 for none
    } X;
    word_t raw;
} kdb_tid_format_t;

extern kdb_tid_format_t kdb_tid_format;
