/*********************************************************************
 *                
 * Copyright (C) 2002, 2003, 2005,  Karlsruhe University
 *                
 * File path:     arch/ia64/breakpoint.h
 * Description:   IA-64 breakpoints
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
 * $Id: breakpoint.h,v 1.5 2005/10/19 16:19:32 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__IA64__BREAKPOINT_H__
#define __ARCH__IA64__BREAKPOINT_H__


/**
 * Generic IA-64 instruction/data breakpoint
 */
class breakpoint_t
{
protected:
    union {
	struct {
	    word_t address	: 64;
	    word_t mask		: 56;
	    word_t plm		: 4;
	    word_t __ig		: 2;
	    word_t w		: 1;
	    word_t rx		: 1;
	};
	u64_t raw[2];
    };

public:
    enum priv_e {
	kernel	= 1,
	user	= 8,
	both	= 9
    };

    // Constructors

    breakpoint_t (void) {}

    // Modification

    void enable (void)
	{ w = 1; rx = 1; }

    void disable (void)
	{ w = 0; rx = 0; }

    // Retrieval

    word_t get_address (void)
	{ return address; }

    word_t get_mask (void)
	{ return mask | ~((1UL << 55) - 1); }

    priv_e get_priv (void)
	{ return (priv_e) plm; }
};



/**
 * IA-64 data breakpoint
 */
class data_breakpoint_t : public breakpoint_t
{
public:
    // Constructors

    data_breakpoint_t (void) {}

    data_breakpoint_t (addr_t addr, word_t mask, priv_e p, bool w, bool r)
	{
	    this->address = (word_t) addr;
	    this->mask = mask;
	    this->plm = (word_t) p;
	    this->w = w;
	    this->rx = r;
	}

    // Retrieval

    bool is_active (void)
	{ return w || rx; }

    bool is_read_match (void)
	{ return rx; }

    bool is_write_match (void)
	{ return w; }

    // Register access

    void put (word_t num);
    void get (word_t num);
};


/**
 * Put breakpoint value into data breakpoint registers.
 * @param num		register pair number
 */
INLINE void data_breakpoint_t::put (word_t num)
{
    __asm__ __volatile__ (
	"	;;			\n"
	"	.reg.val %0,0		\n"
	"	mov dbr[%0] = %2	\n"
	"	.reg.val %1,1		\n"
	"	mov dbr[%1] = %3	\n"
	"	;;			\n"
	"	srlz.d			\n"
	"	;;			\n"
	:
	:
	"r" (num*2),    "r" ((num*2) + 1),
	"r" (raw[0]), "r" (raw[1]));
}


/**
 * Read breakpoint value from data breakpoint registers.
 * @param num		register pair number
 */
INLINE void data_breakpoint_t::get (word_t num)
{
    __asm__ __volatile__ (
	"	mov %0 = dbr[%2]	\n"
	"	mov %1 = dbr[%3]	\n"
	"	;;			\n"
	:
	"=&r" (raw[0]), "=r" (raw[1])
	:
	"r" (num*2), "r" ((num*2) + 1));
}


/**
 * Read data breakpoint register pair.
 * @param num		register pair number
 * @return the indicated data breakpoint register pair
 */
INLINE data_breakpoint_t get_dbr (word_t num)
{
    data_breakpoint_t dbr;
    dbr.get (num);
    return dbr;
}




/**
 * IA-64 instruction breakpoint
 */
class instr_breakpoint_t : public breakpoint_t
{
public:
    // Constructors

    instr_breakpoint_t (void) {}

    instr_breakpoint_t (addr_t addr, word_t mask, priv_e p, bool x)
	{
	    this->address = (word_t) addr;
	    this->mask = mask;
	    this->plm = (word_t) p;
	    this->rx = x;
	}

    // Retrieval

    bool is_active (void)
	{ return rx; }

    // Register access

    void put (word_t num);
    void get (word_t num);
};


/**
 * Put breakpoint value into instruction breakpoint registers.
 * @param num		register pair number
 */
INLINE void instr_breakpoint_t::put (word_t num)
{
    __asm__ __volatile__ (
	"	;;			\n"
	"	.reg.val %0,0		\n"
	"	mov ibr[%0] = %2	\n"
	"	.reg.val %1,1		\n"
	"	mov ibr[%1] = %3	\n"
	"	;;			\n"
	"	srlz.i			\n"
	"	;;			\n"
	:
	:
	"r" (num*2),    "r" ((num*2) + 1),
	"r" (raw[0]), "r" (raw[1]));
}


/**
 * Read breakpoint value from instruction breakpoint registers.
 * @param num		register pair number
 */
INLINE void instr_breakpoint_t::get (word_t num)
{
    __asm__ __volatile__ (
	"	mov %0 = ibr[%2]	\n"
	"	mov %1 = ibr[%3]	\n"
	"	;;			\n"
	:
	"=&r" (raw[0]), "=r" (raw[1])
	:
	"r" (num*2), "r" ((num*2) + 1));
}


/**
 * Read instruction breakpoint register pair.
 * @param num		register pair number
 * @return the indicated instruction breakpoint register pair
 */
INLINE instr_breakpoint_t get_ibr (word_t num)
{
    instr_breakpoint_t ibr;
    ibr.get (num);
    return ibr;
}


#endif /* !__ARCH__IA64__BREAKPOINT_H__ */
