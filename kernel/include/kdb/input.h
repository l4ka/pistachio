/*********************************************************************
 *                
 * Copyright (C) 2002, 2003, 2005,  Karlsruhe University
 *                
 * File path:     kdb/input.h
 * Description:   Declaration of input functions
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
 * $Id: input.h,v 1.7 2005/05/20 12:23:22 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __KDB__INPUT_H__
#define __KBD__INPUT_H__

class space_t;
class tcb_t;
class comspace_t;
class vrt_t;


/*
 * Number returned from get_hex()/get_dec() if user pressed ESC.
 */
#define ABORT_MAGIC	(0x19022002)


word_t get_hex (const char * prompt = NULL,
		const word_t defnum = 0,
		const char * defstr = NULL);

word_t get_dec (const char * prompt = NULL,
		const word_t defnum = 0,
		const char * defstr = NULL);

char get_choice (const char * prompt,
		 const char * choices,
		 char def);

space_t * get_space (const char * prompt = NULL);
tcb_t * get_thread (const char * prompt = NULL);
comspace_t * get_comspace (const char * prompt = NULL);
vrt_t * get_thrspace (const char * prompt = NULL);

#endif /* !__KDB__INPUT_H__ */
