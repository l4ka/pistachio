/*********************************************************************
 *                
 * Copyright (C) 2007,  Karlsruhe University
 *                
 * File path:     arch/x86/atomic.h
 * Description:   
 *                
 * @LICENSE@
 *                
 * $Id:$
 *                
 ********************************************************************/
#ifndef __ARCH__X86__ATOMIC_H__
#define __ARCH__X86__ATOMIC_H__

#ifdef CONFIG_SMP
# define X86_LOCK "lock;"
#else
# define X86_LOCK
#endif


class atomic_t {
public:
    int operator ++ (int) 
	{
	    __asm__ __volatile__(X86_LOCK "add $1, %0" : "=m"(val));
	    return val;
	}

    int operator-- (int) 
	{
	    __asm__ __volatile__(X86_LOCK "sub $1, %0" : "=m"(val));
	    return val;
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

    
    bool cmpxchg( word_t old_val, word_t new_val )
	{
	    bool result;
	    __asm__ __volatile__ (
		X86_LOCK "cmpxchg %1, %2	\n\t"
		"setz %0		\n\t"

		: "=a" (result)
		: "r" (new_val), "m"(val), "0" (old_val)
		: "memory"
		);
	    return (result);
	}

private:
    word_t val;
};


#endif /* !__ARCH__X86__ATOMIC_H__ */
