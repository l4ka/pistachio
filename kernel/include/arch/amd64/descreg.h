/*********************************************************************
 *                
 * Copyright (C) 2003,  Karlsruhe University
 *                
 * File path:     arch/amd64/descreg.h
 * Description:   Values to load in descriptor registers
 *		  (GDTR, IDTR, LDTR, TR)
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
 * $Id: descreg.h,v 1.2 2003/09/24 19:04:26 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__AMD64__SYSDESC_H__
#define __ARCH__AMD64__SYSDESC_H__

class  amd64_descreg_t{
public:
    enum regtype_e
	{
	    gdtr = 0x1,
	    ldtr = 0x2,
	    idtr = 0x3,
	    tr   = 0x4
	};

    static void setdescreg(const regtype_e type, u64_t address, u16_t size);
    static void getdescreg(const regtype_e type, u64_t *address, u16_t *size);
    static void setdescreg(const regtype_e type, u16_t selector);
    static void getdescreg(const regtype_e type, u16_t *selector);

};

/* GDTR, IDTR */
INLINE void amd64_descreg_t::setdescreg(const regtype_e type, u64_t address, u16_t size)
{

    struct {
	u16_t s;
	u64_t a;
    } __attribute__((packed))  regval = { size, address };
    
    
    switch(type){	
    case gdtr:
	asm("lgdt %0\n" : /* No Output */ : "m"(regval));
	break;
    case idtr:
	asm("lidt %0\n" : /* No Output */ : "m"(regval));
	break;
    default:
	break;
    }	
}

INLINE void amd64_descreg_t::getdescreg(const regtype_e type, u64_t *address, u16_t *size){
    
    struct {
	u16_t s;
	u64_t a;
    } __attribute__((packed)) regval = { 0, 0};
    
    switch(type){	
    case gdtr:
	asm("sgdt %0\n" : "=m"(regval));
	break;
    case idtr:
	asm("lidt %0\n" : "=m"(regval));
	break;
    default:
	break;
    }	
    
    *address = regval.a;
    *size = regval.s;
}

/* LDTR, TR  */
INLINE void amd64_descreg_t::setdescreg(const regtype_e type, u16_t selector){

    switch(type){	
    case ldtr:
	asm("lldt %0\n" : /* No Output */ : "m"(selector));
	break;
    case tr:
	asm("ltr %0\n"  : /* No Output */ : "m"(selector));
	break;
    default:
	break;
    }	
}

INLINE void amd64_descreg_t::getdescreg(const regtype_e type, u16_t *selector){

    
    switch(type){	
    case ldtr:
	asm("sldt %0\n" : "=m"(*selector));
	break;
    case tr:
	asm("str %0\n" : "=m"(*selector));
	break;
    default:
	*selector = 0;
	break;
    }	
    
}
#endif /* !__ARCH__AMD64__SYSDESC_H__ */
