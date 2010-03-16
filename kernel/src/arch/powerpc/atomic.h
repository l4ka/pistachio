/*********************************************************************
 *                
 * Copyright (C) 1999-2010,  Karlsruhe University
 * Copyright (C) 2008-2009,  Volkmar Uhlig, IBM Corporation
 *                
 * File path:     src/arch/powerpc/atomic.h
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
 * $Id$
 *                
 ********************************************************************/
#pragma once

class atomic_t {
public:
    int operator ++ (int) 
	{
	    int tmp;
	    sync();
	    __asm__ __volatile__(
		"1:	lwarx	%0,0,%1\n"
		"	addic	%0,%0,1\n"
		"	stwcx.	%0,0,%1 \n"
		"	bne-	1b"
		: "=&r" (tmp)
		: "r" (&val)
		: "cc");
	    isync();
	    return tmp;
	}

    int operator -- (int) 
	{
	    int tmp;
	    sync();
	    __asm__ __volatile__(
		"1:	lwarx	%0,0,%1\n"
		"	addic	%0,%0,-1\n"
		"	stwcx.	%0,0,%1 \n"
		"	bne-	1b"
		: "=&r" (tmp)
		: "r" (&val)
		: "cc");
	    isync();
	    return tmp;
	}

    int operator = (word_t val) 
	{ return this->val = val; }

    int operator = (int val) 
	{ return this->val = val; }

    bool operator == (word_t val) 
	{ return (this->val == val); }
    
    bool operator == (int val) 
	{ return (this->val == (word_t) val); }

    bool operator != (word_t val) 
	{ return (this->val != val); }

    bool operator != (int val) 
	{ return (this->val != (word_t) val); }

    operator word_t (void) 
	{ return val; }

private:
    word_t val;
};
