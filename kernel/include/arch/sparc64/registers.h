/*********************************************************************
 *                
 * Copyright (C) 2003-2004, University of New South Wales
 *                
 * File path:    arch/sparc64/registers.h
 * Description:  Describes the register specifics of the SPARC v9 
 *               architecture.
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
 * $Id: registers.h,v 1.4 2004/02/06 05:48:54 philipd Exp $
 *                
 ********************************************************************/

#ifndef __ARCH__SPARC64__REGISTERS_H__
#define __ARCH__SPARC64__REGISTERS_H__

#include INC_CPU(registers.h)

/********************
 * Integer Registers *
 ********************/

#define REGISTER_WINDOW_SIZE 16 /* 16 registers per window */

#ifndef ASSEMBLY

/******************
 * State Registers *
 ******************/

/**
 *  Y Register (Y_REG)
 */
class y_reg_t {
public:
    u32_t y;

public:

    /* Y register management. */

    void set(void)
    {
	__asm__ __volatile__("wr\t%0, %%y\n"
			     : /* no output */
			     : "r" (y)); // %0
    }

    void get(void)
    {
	__asm__ __volatile__("rd\t%%y, %0\n"
			     : "=r" (y) // %0
			     : /* no input */);
    }

    /* Printing. */

    void print(void) { printf("y: 0x%x", y);}

}; // y_reg_t

/**
 *  Condition Codes Register (CCR)
 */
class ccr_t {
public:
    union {
	u8_t raw;
	struct {
	    BITFIELD8(u8_t,
		      /* icc: 32-bit condition codes. */
		      icc_c : 1, // Carry.
		      icc_v : 1, // Overflow.
		      icc_z : 1, // Zero.
		      icc_n : 1, // Negative.
		      /* xcc: 64-bit condition codes. */
		      xcc_c : 1,
		      xcc_v : 1,
		      xcc_z : 1,
		      xcc_n : 1

	    ) // BITFIELD8()
	} ccr;

    }; // union

public:

    /* CCR management. */

    void set(void)
    {
	__asm__ __volatile__("wr\t%0, %%ccr\n"
			     : /* no output */
			     : "r" (raw)); // %0
    }

    void get(void)
    {
	__asm__ __volatile__("rd\t%%ccr, %0\n"
			     : "=r" (raw) // %0
			     : /* no input */);
    }

    /* Printing. */

    void print(void)
    {
	printf("xcc: %c%c%c%c icc: %c%c%c%c",
	       ccr.xcc_c ? 'C' : 'c',
	       ccr.xcc_v ? 'V' : 'v',
	       ccr.xcc_z ? 'Z' : 'z',
	       ccr.xcc_n ? 'N' : 'n',
	       ccr.icc_c ? 'C' : 'c',
	       ccr.icc_v ? 'V' : 'v',
	       ccr.icc_z ? 'Z' : 'z',
	       ccr.icc_n ? 'N' : 'n');
    }

} __attribute__((packed)); // ccr_t

/**
 *  Address Space Identifier (ASI)
 */
class asi_t {
public:
    u8_t asi;

public:

    /* ASI management. */

    void set(void)
    {
	__asm__ __volatile__("wr\t%0, %%asi\n"
			     : /* no output */
			     : "r" (asi)); // %0
    }

    void get(void)
    {
	__asm__ __volatile__("rd\t%%asi, %0\n"
			     : "=r" (asi)
			     : /* no input */); // %0
    }

    /* Printing. */

    void print(void){printf("asi: 0x%x", asi);}

}; // asi_t

/**
 *  Floating-Point Register Status (FPRS)
 */

/***********************
 * Privileged Registers *
 ***********************/

/**
 *  Tick (TICK)
 */

/**
 *  Floating-Point Deferred-Trap Queue (FQ)
 */

/**
 *  Version Register (VER)
 *  Notes: Read only register.
 */
class ver_t {
public:
    union {
	u64_t raw;
	struct {
	    BITFIELD7(u64_t,
		maxwin : 5,  /* Maximum window index (NWINDOWS - 1).          */
		__rv1  : 3,  /* Reserved by architecture.                     */
		maxtl  : 8,  /* Maximum number of trap levels supported.      */
		__rv2  : 8,  /* Reserved by architecture.                     */
		mask   : 8,  /* Mask set revision, manufacturer.              */
		impl   : 16, /* Implementation of architecture, manufacturer. */
		manuf  : 16  /* JEDEC semiconductor Manufacturer Code.        */

	    ) // BITFIELD7()
	} ver;

    }; // union

public:

    /* VER management. */

    void get(void)
    {
	__asm__ __volatile__("rdpr\t%%ver, %0\n"
			     : "=r" (raw)
			     : /* no input */); // %0
    }

    /* Printing. */

    void print(void)
    {
	printf("ver: trap levels %d windows %d manuf 0x%x impl 0x%x mask 0x%x\n",
	       ver.maxtl, ver.maxwin + 1, ver.manuf, ver.impl, ver.mask);
    }

} __attribute__((packed)); // ver_t

/**
 *  Current Window Pointer (CWP)
 */
class cwp_t {
public:
    u8_t cwp;

public:

    /* CWP management. */

    void set(void)
    {
	__asm__ __volatile__("wrpr\t%0, %%cwp\n"
			     : /* no output */
			     : "r" (cwp)); // %0
    }

    void get(void)
    {
	__asm__ __volatile__("rdpr\t%%cwp, %0\n"
			     : "=r" (cwp) // %0
			     : /* no input */);
    }

    /* Printing. */

    void print(void) {printf("cwp: %d", cwp);}

}; // cwp_t

/**
 *  Window State Register (WSTATE)
 */
class wstate_t {
public:
    union {
	u8_t raw;
	struct {
	    BITFIELD3(u8_t,
		      normal : 3, /* Which normal spill/fill handler to use. */
		      other  : 3, /* Which other spill/fill handler to use.  */
		      __rv1  : 2  /* Unused.                                 */

	    ) // BITFIELD3()
	} wstate;

    }; // union

public:

    /* WSTATE management. */

    void set(void)
    {
	__asm__ __volatile__("wrpr\t%0, %%wstate\n"
			     : /* no output */
			     : "r" (raw)); // %0
    }

    void get(void)
    {
	__asm__ __volatile__("rdpr\t%%wstate, %0\n"
			     : "=r" (raw) // %0
			     : /* no input */);
    }

    /* Printing. */

    void print(void)
    {
	printf("wstate: normal %d other %d",
	       wstate.normal, wstate.other);

    }

}; // wstate_t

/**
 *  Register Window State (CWP, CANSAVE, CANRESTORE, OTHERWIN, WSTATE, CLEANWIN)
 */
class reg_win_t {
public:
    cwp_t    cwp;
    u8_t     cansave;
    u8_t     canrestore;
    u8_t     otherwin;
    wstate_t wstate;
    u8_t     cleanwin;

public:

    /* State management. */

    void set(void)
    {
	cwp.get();
	wstate.get();

	__asm__ __volatile__("wrpr\t%0, %%cansave\n"
			     "wrpr\t%1, %%canrestore\n"
			     "wrpr\t%2, %%otherwin\n"
			     "wrpr\t%3, %%cleanwin\n"
			     : /* no output */
			     : "r" (cansave),    // %0
			     "r" (canrestore), // %1
			     "r" (otherwin),   // %2
			     "r" (cleanwin));  // %3
    }

    void get(void)
    {
	cwp.get();
	wstate.get();

	__asm__ __volatile__("rdpr\t%%cansave, %0\n"
			     "rdpr\t%%canrestore, %1\n"
			     "rdpr\t%%otherwin, %2\n"
			     "rdpr\t%%cleanwin, %3\n"
			     : "=r" (cansave),    // %0
			     "=r" (canrestore), // %1
			     "=r" (otherwin),   // %2
			     "=r" (cleanwin)    // %3 
			     : /* no input */);
    }

    /* Printing. */

    void print(void)
    {
	cwp.print(), printf(" cansave: %d canrestor %d ", cansave, canrestore),
	wstate.print(), printf(" cleanwin: %d\n", cleanwin);
    }

}; // reg_win_t

/**
 *  Processor State Register (PSTATE)
 */

#endif /* !ASSEMBLY */

/*      PSTATE_PID1 (1 << 11)*/ /* CPU implementation dependent.     */
/*      PSTATE_PID0 (1 << 10)*/ /* CPU implementation dependent.     */
#define PSTATE_CLE  (1 << 9)    /* Current little endian.            */
#define PSTATE_TLE  (1 << 8)    /* Traps little endian.              */

/* PSTATE_MM, Memory model */
#define PSTATE_TSO  (0 << 6)    /* Total store order.                */
/*      PSTATE_PSO  (1 << 6)*/  /* Partial store order, CPU dep.     */
/*      PSTATE_RMO  (2 << 6)*/  /* Relaxed memory order, CPU dep.    */

#define PSTATE_RED  (1 << 5)    /* Reset, Error, and Debug state.    */
#define PSTATE_PEF  (1 << 4)    /* Priv, Floating point unit enable. */
#define PSTATE_AM   (1 << 3)    /* Address-mask, 32-bit mode.        */
#define PSTATE_PRIV (1 << 2)    /* Priviledged mode.                 */
#define PSTATE_IE   (1 << 1)    /* Interrupts enabled.               */
#define PSTATE_AG   (1 << 0)    /* Alternate globals enabled.        */

#ifndef ASSEMBLY

class pstate_t {
public:
    union {
	u16_t raw;
	struct {
	    BITFIELD12(u16_t,
		       ag          : 1, /* Use aternative global registers. */
		       ie          : 1, /* Interrupts enable.               */
		       priv        : 1, /* Priviledged mode.                */
		       am          : 1, /* Address mask, 32 vs64 addresses. */
		       pef         : 1, /* Enabled Floating-point unit.     */
		       red         : 1, /* Reset Error and Debug state.     */
		       mm          : 2, /* Memory model, see mm_e below .   */
		       tle         : 1, /* Traps will use little endian.    */
		       cle         : 1, /* Currently little endian.         */
		       PSTATE_PID0 : 1, /* see INC_CPU(registers.h)         */
		       PSTATE_PID1 : 1, /* see INC_CPU(registers.h)         */
		       __rv        : 4  /* Unused.                          */

	    ) // BITFIELD12()
	} pstate;

    }; // union

    /* Memory model */
    enum mm_e {
	TSO = 0, /* Total Store Order.    */
	PSO = 1, /* Partial Store Order.  */
	RMO = 2  /* Relaxed Memory Order. */
    };

public:

    /* PSTATE management. */

    void set(void)
    {
	__asm__ __volatile__("wrpr\t%0, %%pstate\n"
			     : /* no output */
			     : "r" (raw)); // %0      
    }

    void get(void)
    {
	__asm__ __volatile__("rdpr\t%%pstate, %0\n"
			     : "=r" (raw) // %0
			     : /* no input */);
    }

    /* Printing. */

    void print(void)
    {
	printf("pstate: %c%c%c%c %s %c%c%c%c%c%c",
	       PSTATE_PID1_CHAR(),
	       PSTATE_PID0_CHAR(),
	       pstate.cle  ? 'C' : 'c',
	       pstate.tle  ? 'T' : 't',
	       (pstate.mm == TSO) ? "TSO" :
	       ((pstate.mm == PSO) ? "PSO" :
		((pstate.mm == RMO) ? "RMO" : "XXX" )),
	       pstate.red  ? 'R' : 'r',
	       pstate.pef  ? 'F' : 'f',
	       pstate.am   ? 'A' : 'a',
	       pstate.priv ? 'P' : 'p',
	       pstate.ie   ? 'E' : 'e',
	       pstate.ag   ? 'G' : 'g');
    }

} __attribute__((packed)); // pstate_t

/**
 *  Processor Interrupt Level (PIL)
 */
class pil_t {
public:
    u8_t pil;

public:

    /* PIL management */

    void set(void)
    {
	__asm__ __volatile__("wrpr\t%0, %%pil\n"
			     : /* no output */
			     : "r" (pil)); // %0
    }

    void get(void)
    {
	__asm__ __volatile__("rdpr\t%%pil, %0\n"
			     : "=r" (pil) // %0
			     : /* no input */);
    }

    /* Printing. */

    void print(void) {printf("pil: %d", pil);}

}; // pil_t

/**
 *  Trap Level (TL)
 */
class tl_t {
public:
    u8_t tl;

public:

    /* TL management. */

    void set(void)
    {
	__asm__ __volatile__("wrpr\t%0, %%tl\n"
			     : /* no output */
			     :  "r" (tl)); // %0
    }

    void get(void)
    {
	__asm__ __volatile__("rdpr\t%%tl, %0\n"
			     : "=r" (tl) // %0
			     : /* no input */);
    }

    /* Printing. */

    void print(void) {printf("tl: %d", tl);}

}; // tl_t

/**
 *  Trap Program Counter (TPC)
 */
class tpc_t {
public:
    word_t tpc;

public:

    /* TPC management. */

    void set(void)
    {
	__asm__ __volatile__("wrpr\t%0, %%tpc\n"
			     : /* no output */
			     : "r" (tpc)); // %0      
    }

    void get(void)
    {
	__asm__ __volatile__("rdpr\t%%tpc, %0\n"
			     : "=r" (tpc) // %0
			     : /* no input */);
    }

    /* Printing. */

    void print(void) {printf("tpc: 0x%x", tpc);}

}; // tpc_t

/**
 *  Trap Next Program Counter (TNPC)
 */
class tnpc_t {
public:
    word_t tnpc;

public:

    /* TNPC management. */

    void set(void)
    {
	__asm__ __volatile__("wrpr\t%0, %%tnpc\n"
			     : /* no output */
			     : "r" (tnpc)); // %0      
    }

    void get(void)
    {
	__asm__ __volatile__("rdpr\t%%tnpc, %0\n"
			     : "=r" (tnpc)
			     : /* no input */); // %0
    }

    /* Printing. */

    void print(void) {printf("tnpc: 0x%x", tnpc);}

}; // tnpc_t

/**
 *  Trap state (TSTATE)
 */
class tstate_t {
private:
    union {
	u64_t raw;
	struct {
	    BITFIELD7(u64_t,
		      cwp    : 5,  /* Trap CWP.    */
		      __rv1  : 3,  /* Reserved.    */
		      pstate : 12, /* Trap PSTATE. */
		      __rv2  : 4,  /* Reserved.    */
		      asi    : 8,  /* Trap ASI.    */
		      ccr    : 8,  /* Trap CCR.    */
		      __rv3  : 24  /* Reserved.    */

	    ) // BITFIELD7()
	} tstate;

    }; // union

public:

    /* TSTATE management. */

    void set(void)
    {
	__asm__ __volatile__("wrpr\t%0, %%tstate\n"
			     : /* no output */
			     : "r" (raw)); // %0      
    }

    void get(void)
    {
	__asm__ __volatile__("rdpr\t%%tstate, %0\n"
			     : "=r" (raw)
			     : /* no input */); // %0
    }

    /* TSTATE field access. */

    void set_cwp(cwp_t cwp){tstate.cwp = (u64_t)cwp.cwp;}
    void set_asi(asi_t asi){tstate.asi = (u64_t)asi.asi;}
    void set_ccr(ccr_t ccr){tstate.ccr = (u64_t)ccr.raw;}
    void set_pstate(pstate_t pstate){tstate.pstate = (u64_t)pstate.raw;} 

    cwp_t    get_cwp(void){cwp_t cwp; cwp.cwp = tstate.cwp; return cwp;}
    asi_t    get_asi(void){asi_t asi; asi.asi = tstate.asi; return asi;}
    ccr_t    get_ccr(void){ccr_t ccr; ccr.raw = tstate.ccr; return ccr;}
    pstate_t get_pstate(void)
    {pstate_t pstate; pstate.raw = tstate.pstate; return pstate;}

    /* Printing. */

    void print(void)
    {
	extern void putc(char);

	cwp_t cwp = get_cwp();
	asi_t asi = get_asi();
	ccr_t ccr = get_ccr();
	pstate_t pstate = get_pstate();

	printf("tstate: ");
	ccr.print(), putc(' '), asi.print(), putc(' '),
	pstate.print(), putc(' '), cwp.print(), putc('\n');

    } // print()

} __attribute__((packed)); // tstate_t

/**
 *  Trap Type (TT)
 */
class tt_t {
public:
    u16_t tt;

public:

    /* TT management. */

    void set(void)
    {
	__asm__ __volatile__("wrpr\t%0, %%tt\n"
			     : /* no output */
			     : "r" (tt)); // %0      
    }

    void get(void)
    {
	__asm__ __volatile__("rdpr\t%%tt, %0\n"
			     : "=r" (tt) // %0
			     : /* no input */);
    }

    /* Printing. */

    void print(void) {printf("tt: %0x", tt);}

}; // tt_t

/**
 *  Trap Base Adddress (TBA)
 */

#endif /* !ASSEMBLY */

#endif /* !__ARCH__SPARC64__REGISTERS_H__ */
