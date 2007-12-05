/*********************************************************************
 *                
 * Copyright (C) 2007,  Karlsruhe University
 *                
 * File path:     arch/x86/trapgate.h
 * Description:   
 *                
 * @LICENSE@
 *                
 * $Id:$
 *                
 ********************************************************************/
#ifndef __ARCH__X86__TRAPGATE_H__
#define __ARCH__X86__TRAPGATE_H__

#include INC_ARCH_SA(trapgate.h)

#if defined(CONFIG_DEBUG)
extern "C" int printf(const char* format, ...);
#endif

class x86_exceptionframe_t : public x86_exceptionregs_t
{
public:
   
#if defined(CONFIG_DEBUG)
    static const char		*name[num_regs];
    static const word_t		dbgreg[num_dbgregs];

    void dump_flags()
	{
	    printf("%c%c%c%c%c%c%c%c%c%c%c",
		   regs[freg] & (1 <<  0) ? 'C' : 'c',
		   regs[freg] & (1 <<  2) ? 'P' : 'p',
		   regs[freg] & (1 <<  4) ? 'A' : 'a',
		   regs[freg] & (1 <<  6) ? 'Z' : 'z',
		   regs[freg] & (1 <<  7) ? 'S' : 's',
		   regs[freg] & (1 << 11) ? 'O' : 'o',
		   regs[freg] & (1 << 10) ? 'D' : 'd',
		   regs[freg] & (1 <<  9) ? 'I' : 'i',
		   regs[freg] & (1 <<  8) ? 'T' : 't',
		   regs[freg] & (1 << 16) ? 'R' : 'r',
		   ((regs[freg] >> 12) & 3) + '0'
		);
	}

    void dump ()
	{
	    printf("fault addr: %8x\tstack: %8x\terror code: %x frame: %p\n",
		   regs[ipreg], regs[spreg], error,  this);
	    
	    for (word_t r=0; r < num_dbgregs; r++)
	    {
		printf("\t%s: %wx", name[dbgreg[r]], regs[dbgreg[r]]);
		
		if (dbgreg[r] == freg)
		{ 
		    printf(" ["); dump_flags(); printf("]"); 
		} 
		if ((r+1) % 2 == 0) printf("\n");

	    }
	}
#endif /* defined(CONFIG_DEBUG */

   
};

#endif /* !__ARCH__X86__TRAPGATE_H__ */
