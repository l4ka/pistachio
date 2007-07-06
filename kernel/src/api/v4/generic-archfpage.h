/*********************************************************************
 *                
 * Copyright (C) 2002-2005,  Karlsruhe University
 *                
 * File path:     generic/fpage.h
 * Description:   dummy architecture specific fpage declaration, for use by 
 *		  architectures without such pages
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
 * $Id: generic-archfpage.h,v 1.1 2005/05/19 08:34:59 stoess Exp $
 *                
 ********************************************************************/
#ifndef __GENERIC__FPAGE_H__
#define __GENERIC__FPAGE_H__

#include INC_API(config.h)

class fpage_t;
class tcb_t;


/**
 * Flexpages are size-aligned memory objects and can cover multiple hardware
 * pages. arch_fpage_t implements the architecture-specific flexpage type,
 * having read, write and execute bits.
 */
class arch_fpage_t
{
    /* data members */
public:
    word_t raw;
    /* member functions */
public:

    /**
     * sets the flexpage
     */
    void set(word_t base, word_t log2size, bool read, bool write, bool exec)
	{
	    
	}
    /**
     * @return true if the flexpage is a nil fpage
     */
    bool is_valid_page (void) { return false; }

    /**
     * @return true if flexpage covers the whole arch-specific space
     */
    bool is_complete_page() { return false; }


    /**
     * @return base address of the fpage
     * get_base does not size-align the address
     */
    addr_t get_base() {  return NULL; }

    /**
     * @return size aligned address of the fpage
     */
    addr_t get_address() {  return NULL; }
    
    /**
     * @return size of the flexpage
     */
    word_t get_size() {  return 0; }

    /**
     * @return log2 size of the fpage
     */
    word_t get_size_log2() {  return 0; }
    
    /**
     * @return true if the read bit is set
     */
    bool is_read() {  return false; }

    /** 
     * @return true if the write bit is set
     */
    bool is_write() {  return false; }

    /**
     * @return true if the execute bit is set
     */
    bool is_execute() {  return false; }

    /**
     * @return true if read, write and execute bits are set
     */
    bool is_rwx() {  return true; }

    /**
     * sets all permission bits in the fpage
     */
    void set_rwx() {   }

    /**
     * sets specific permission bits in the fpage
     */
    void set_rwx(word_t rwx) {  }

    /**
     * @return access rights of fpage
     */
    word_t get_rwx() 	{  return false; }

    /**
     * @return delivers an fpage covering the complete arch-specific space
     */
    static arch_fpage_t complete() { return (arch_fpage_t) { raw:0}; }
    

};


#endif /* !__GENERIC__FPAGE_H__ */
