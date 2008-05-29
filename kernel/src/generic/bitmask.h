/*********************************************************************
 *                
 * Copyright (C) 2003, 2007-2008,  Karlsruhe University
 *                
 * File path:     generic/bitmask.h
 * Description:   Generic bitmask class
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
 * $Id: bitmask.h,v 1.2 2003/09/24 19:04:24 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __BITMASK_H__
#define __BITMASK_H__


/**
 * Generic bitmask manipulation.
 */
template <typename T> class bitmask_t
{
    T	maskvalue;
    static const word_t masksize = sizeof(T) * 8;
    
public:

    // Constructors
    bitmask_t (void) { maskvalue = 0; }
    bitmask_t (T initvalue) 
	{ maskvalue = initvalue; }    
    
    // Modification 
    
    bitmask_t clear()
	{
           maskvalue = 0;
           return (bitmask_t) maskvalue;
	}
  
    inline bitmask_t operator = (const int &n) 
	{
	    maskvalue = (1UL << n);
	    return (bitmask_t) maskvalue;
	}
    
    inline bitmask_t operator + (const int &n) const
	{
	    bitmask_t m (maskvalue | (1UL << n));
	    return m;
	}

    inline bitmask_t operator - (const int &n) const
	{
	    bitmask_t m (maskvalue & ~(1UL << n));
	    return m;
	}

    inline bitmask_t operator += (const int &n)
	{
	    maskvalue = maskvalue | (1UL << n);
	    return (bitmask_t) maskvalue;
	}
   
    inline bitmask_t operator -= (const int &n)
	{
	    maskvalue = maskvalue & ~(1UL << n);
	    return (bitmask_t) maskvalue;
	}

    // Predicates
    inline bitmask_t operator == (bitmask_t &m2) const
	{
	    return maskvalue == m2.maskvalue;
	}


    inline bool is_set (const int &n) const
	{ 
	    return (maskvalue & (1UL << n)) != 0;
	}

    // Conversion

    inline operator T (void) const
	{ 
	    return maskvalue;
	}

#if defined(CONFIG_DEBUG)
    char *string()
	{
	    static const char *d = "0123456789";
	    static char s[3+masksize];
	    s[0] = '['; s[1+masksize]=']'; s[2+masksize]=0;
	    for (word_t i=0; i< masksize; i++)
		s[masksize-i] = is_set(i) ? d[i%10] : '~';
	    return s;
	}
#endif    
};



#endif /* !__BITMASK_H__ */
