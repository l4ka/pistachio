/*********************************************************************
 *                
 * Copyright (C) 2002-2004,  Karlsruhe University
 *                
 * File path:     kdb/linker_set.h
 * Description:   Generic sets defined at link time
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
 * $Id: linker_set.h,v 1.7 2004/06/04 19:32:08 ud3 Exp $
 *                
 ********************************************************************/
#ifndef __KDB__LINKER_SET_H__
#define __KDB__LINKER_SET_H__

class linker_set_entry_t;

/**
 * linker_set_t: A linker set contains a number of set entries defined
 * at link time.  These entries can be iterated over using the reset()
 * and next() methods.
 */
class linker_set_t
{
public:
    linker_set_entry_t	*list;
    word_t		entries;
    word_t		curidx;

    void print (void);
    void reset (void);
    addr_t next (void);
    word_t size (void);
    addr_t get (word_t n);
};


/**
 * linker_set_entry_t: Linker set entry types are opaque.  The entries
 * themselves are retrieved using the get_entry() method.
 */
class linker_set_entry_t
{
public:
    linker_set_t	*set;
    addr_t		entry;

    inline linker_set_t * get_set (void) { return set; }
    inline addr_t get_entry (void) { return entry; }
};



/**
 * DECLARE_SET: Declares a new set and ensures that it is initialized
 * upon startup.
 */
#define DECLARE_SET(name)						\
  linker_set_t name							\
  __attribute__ ((__section__ (".setlist"), __unused__)) = { NULL, 0 }


/**
 * PUT_SET: Puts an object into the given set.  The object can be of
 * any type.  It is up to the user to ensure correct use of types.
 */
#define PUT_SET(set, sym)						 \
  extern linker_set_t set;						 \
  linker_set_entry_t __setentry_##set##_##sym			 \
  __attribute__ ((__section__ ("set_" #set #sym), __unused__)) = 	 \
	{ &set, (addr_t)&sym }



#endif /* !__KDB__LINKER_SET_H__ */
