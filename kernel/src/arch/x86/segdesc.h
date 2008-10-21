/*********************************************************************
 *                
 * Copyright (C) 2007-2008,  Karlsruhe University
 *                
 * File path:     arch/x86/segdesc.h
 * Description:   
 *                
 * @LICENSE@
 *                
 * $Id:$
 *                
 ********************************************************************/
/* 
 * GDTR, IDTR, LDTR, TR  
 * 
 */
#ifndef __ARCH__X86__SEGDESC_H__
#define __ARCH__X86__SEGDESC_H__

#include INC_ARCH_SA(segdesc.h)

class  x86_descreg_t
{
public:
    
    union 
    {
	struct {
	    u16_t	   size;
	    word_t	   addr __attribute__((packed)); 
	} descriptor;
	u16_t  selector;
    };
	
    enum regtype_e
    {
	gdtr = 0x1,
	ldtr = 0x2,
	idtr = 0x3,
	tr   = 0x4
    };
    
    x86_descreg_t(word_t addr, u16_t size)
	{ 
	    descriptor.addr = addr; 
	    descriptor.size = size;  
	}

    x86_descreg_t(u16_t sel)
	{ selector = sel; }

    void setdescreg(const regtype_e type)
	{
	    switch(type)
	    {	
	    case gdtr:
		asm("lgdt %0\n" : /* No Output */ : "m"(descriptor)); 
		break;
	    case idtr:
		asm("lidt %0\n" : /* No Output */ : "m"(descriptor));
		break;
	    default:
		break;
	    }	
	}
    
    void getdescreg(const regtype_e type)
	{
	    
	    switch(type){	
	    case gdtr:
		asm("sgdt %0\n" : "=m"(descriptor));
		break;
	    case idtr:
		asm("sidt %0\n" : "=m"(descriptor));
		break;
	    default:
		break;
	    }	
    
	}

    void setselreg(const regtype_e type)
	{
	   
	    switch(type)
	    {	
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
    void getselreg(const regtype_e type)
	{
	    
	    switch(type){	
	    case ldtr:
		asm("sldt %0\n" : "=m"(selector));
		break;
	    case tr:
		asm("str %0\n" : "=m"(selector));
		break;
	    default:
		selector = 0;
		break;
	    }	
	}
};

#endif /* !__ARCH__X86__SEGDESC_H__ */
