/*********************************************************************
 *                
 * Copyright (C) 2006,  Karlsruhe University
 *                
 * File path:     l4/mips32/syscalls.h
 * Description:   MIPS32 system call ABI
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
 * $Id: syscalls.h,v 1.1 2006/02/23 21:07:57 ud3 Exp $
 *                
 ********************************************************************/
#ifndef __L4__MIPS32__SYSCALLS_H__
#define __L4__MIPS32__SYSCALLS_H__

#include <l4/types.h>
#include <l4/message.h>

/* Syscall KernelInterface */

#define __L4_MAGIC_KIP_REQUEST          (0x141fca11)


L4_INLINE void * L4_KernelInterface (L4_Word_t *ApiVersion,
				     L4_Word_t *ApiFlags,
				     L4_Word_t *KernelId)
{
    register void * base_address	asm ("$8");  /* t0 */
    register L4_Word_t api_version	asm ("$9");  /* t1 */
    register L4_Word_t api_flags	asm ("$10"); /* t2 */
    register L4_Word_t kernel_id	asm ("$11"); /* t3 */

    __asm__ __volatile__ (".set noat;\n\t");
    register L4_Word_t i asm("$1")= __L4_MAGIC_KIP_REQUEST;

    __asm__ __volatile__ (
        ".word 0xF1000000;\r\n" /* illegal instruction */
        ".set at;     \n\r"
        : "=r" (base_address), "=r" (api_version), "=r" (api_flags), "=r" (kernel_id)
        : "r" (i)
        /*:  "$1" XXX*/
        );
    __asm__ __volatile__ ("":::"$1");

    if( ApiVersion ) *ApiVersion = api_version;
    if( ApiFlags ) *ApiFlags = api_flags;
    if( KernelId ) *KernelId = kernel_id;

    return base_address;
}





/* Syscall ThreadControl */

typedef L4_Word_t (*__L4_ThreadControl_t)(L4_ThreadId_t, L4_ThreadId_t, L4_ThreadId_t, L4_ThreadId_t, void *);
extern __L4_ThreadControl_t __L4_ThreadControl;

L4_INLINE L4_Word_t L4_ThreadControl (L4_ThreadId_t dest,
				      L4_ThreadId_t SpaceSpecifier,
				      L4_ThreadId_t Scheduler,
				      L4_ThreadId_t Pager,
				      void * UtcbLocation)
{

    register L4_Word_t	r_dest			asm("$4");   // a0
    register L4_Word_t	r_space			asm("$5");   // a1
    register L4_Word_t	r_schedule		asm("$6");   // a2
    register L4_Word_t	r_pager			asm("$7");   // a3
    register L4_Word_t	r_utcb			asm("$16");  // s0
	
    L4_Word_t  r_result;
	
    r_dest = dest.raw;
    r_space = SpaceSpecifier.raw;
    r_schedule = Scheduler.raw;
    r_pager = Pager.raw;
    r_utcb = (L4_Word_t)UtcbLocation;

    __asm__ __volatile__ (
        "lw $2, __L4_ThreadControl				\n\t"	
        "subu $29, $29, 0x10					\n\t"		
        "jalr $2								\n\t"
        "nop									\n\t"
        "addu $29, $29, 0x10					\n\t"
        "move %0, $2							\n\t"	/* result */
        : "=r"(r_result)
        : "r"(r_dest), "r"(r_space), "r"(r_schedule), "r"(r_pager), "r"(r_utcb)
	);
	
    //__asm__ __volatile__( "" ::: "$1", "$3", "$4", "$5", "$6", "$7", "$8", "$9", "$10", "$11", "$12", "$13", 
    //		"$14", "$15", "$16", "$17", "$18", "$19", "$20", "$21", "$22", "$23", "$24", "$25", "$31" );					  

    __asm__ __volatile__( "" ::: "$1", "$2", "$3", "$4", "$5", "$6", "$7", "$8", "$9", "$10", "$11", "$12", "$13", 
                          "$14", "$15", "$16", "$17", "$18", "$19", "$20", "$21", "$22", "$23", "$24", "$25", "$31" );					  

    return( r_result );
	
}






/* Syscall SpaceControl */

typedef void (*__L4_SpaceControl_t)(L4_ThreadId_t SpaceSpecifier,
                                    L4_Word_t control,
                                    L4_Fpage_t KernelInterfacePageArea,
                                    L4_Fpage_t UtcbArea,
                                    L4_ThreadId_t redirector);
extern __L4_SpaceControl_t __L4_SpaceControl;

L4_INLINE L4_Word_t L4_SpaceControl (L4_ThreadId_t SpaceSpecifier,
				     L4_Word_t control,
				     L4_Fpage_t KernelInterfacePageArea,
				     L4_Fpage_t UtcbArea,
				     L4_ThreadId_t redirector,
				     L4_Word_t *old_control)
{

    register L4_Word_t r_space		asm("$4");	// a0	
    register L4_Word_t r_control	asm("$5");	// a1	
    register L4_Word_t r_kip		asm("$6");	// a2
    register L4_Word_t r_utcb		asm("$7");	// a3
    register L4_Word_t r_redir		asm("$16");	// s0
	
    L4_Word_t r_res0;
    L4_Word_t r_res1;
	

	
    r_space		= SpaceSpecifier.raw;
    r_control	= control;	
    r_kip		= KernelInterfacePageArea.raw;
    r_utcb		= UtcbArea.raw;
    r_redir		= redirector.raw;

    __asm__ __volatile__ (
        "lw	$2, __L4_SpaceControl				\n\t"	
        "subu $29, $29, 0x10					\n\t"		
        "jalr $2								\n\t"
        "nop									\n\t"
        "addu $29, $29, 0x10					\n\t"
        "move %0, $2							\n\t"	/* result */
        "move %1, $4							\n\t"	/* result */
        : "=r"(r_res0), "=r"(r_res1)
        : "r"(r_space), "r"(r_control), "r"(r_kip), "r"(r_utcb), "r"(r_redir)
	);
	
    //__asm__ __volatile__( "" ::: "$1", "$3", "$5", "$6", "$7", "$8", "$9", "$10", "$11", "$12", "$13", 
    //		"$14", "$15", "$16", "$17", "$18", "$19", "$20", "$21", "$22", "$23", "$24", "$25", "$31" );					  

    __asm__ __volatile__( "" ::: "$1", "$2", "$3", "$4", "$5", "$6", "$7", "$8", "$9", "$10", "$11", "$12", "$13", 
                          "$14", "$15", "$16", "$17", "$18", "$19", "$20", "$21", "$22", "$23", "$24", "$25", "$31" );					  

    if(old_control)
        *old_control = r_res1;

    return( r_res0 );
}





/* Syscall SystemClock */

typedef L4_Word64_t (*__L4_SystemClock_t)(void);
extern __L4_SystemClock_t __L4_SystemClock;

L4_INLINE L4_Clock_t L4_SystemClock (void) {
    return (L4_Clock_t){ raw: __L4_SystemClock() }; 
}





/* Syscall ThreadSwitch */

typedef void (*__L4_ThreadSwitch_t)(L4_ThreadId_t);
extern __L4_ThreadSwitch_t __L4_ThreadSwitch;

L4_INLINE void L4_ThreadSwitch (L4_ThreadId_t dest) {
	
    register L4_Word_t r_control asm("$4");
    r_control = dest.raw;
	
    __asm__ __volatile__ (
        "lw $2, __L4_ThreadSwitch				\n\t"	
        "subu $29, $29, 0x10					\n\t"		
        "jalr $2								\n\t"
        "nop									\n\t"
        "addu $29, $29, 0x10					\n\t"
        :: "r"(r_control)
	);
	
    __asm__ __volatile__( "" ::: "$1", "$2", "$3", "$4", "$5", "$6", "$7", "$8", "$9", "$10", "$11", "$12", "$13", 
                          "$14", "$15", "$16", "$17", "$18", "$19", "$20", "$21", "$22", "$23", "$24", "$25", "$31" );					  
	
}





/* Syscall Schedule */

typedef L4_Word_t (*__L4_Schedule_t)(L4_ThreadId_t dest, L4_Word_t TimeControl, 
                                     L4_Word_t ProcessorControl, L4_Word_t prio, L4_Word_t PreemptionControl); 
extern __L4_Schedule_t __L4_Schedule;

L4_INLINE L4_Word_t  L4_Schedule (L4_ThreadId_t dest,
				  L4_Word_t TimeControl,
				  L4_Word_t ProcessorControl,
				  L4_Word_t prio,
				  L4_Word_t PreemptionControl,
				  L4_Word_t * old_TimeControl)
{

    register L4_Word_t r_dest  asm("$4");	// a0
    register L4_Word_t r_time  asm("$5");	// a1
    register L4_Word_t r_proc  asm("$6");	// a2
    register L4_Word_t r_prio  asm("$7");	// a3
    register L4_Word_t r_pree  asm("$16");	// s0
	
    L4_Word_t res0;
    L4_Word_t res1;	

    r_dest = dest.raw;
    r_time = TimeControl;
    r_proc = ProcessorControl;
    r_prio = prio;
    r_pree = PreemptionControl;

	
    __asm__ __volatile__ (
        "lw $2, __L4_Schedule					\n\t"
        "subu $29, $29, 0x10					\n\t"		
        "jalr $2								\n\t"
        "nop									\n\t"
        "addu $29, $29, 0x10					\n\t"
        "move %0, $2							\n\t"  /* result */
        "move %1, $4							\n\t"  /* result */
        : "=r"(res0), "=r"(res1)
        : "r"(r_dest), "r"(r_time), "r"(r_proc), "r"(r_prio), "r"(r_pree) 
	);
	
    __asm__ __volatile__( "" ::: "$1", "$2", "$3", "$4", "$5", "$6", "$7", "$8", "$9", "$10", "$11", "$12", "$13", 
                          "$14", "$15", "$16", "$17", "$18", "$19", "$20", "$21", "$22", "$23", "$24", "$25", "$31" );					  

    //__asm__ __volatile__( "" ::: "$1", "$3", "$5", "$6", "$7", "$8", "$9", "$10", "$11", "$12", "$13", 
    //		"$14", "$15", "$16", "$17", "$18", "$19", "$20", "$21", "$22", "$23", "$24", "$25", "$31" );					  

    if(old_TimeControl) *old_TimeControl = res1;

    return( res0 );
}





/* Unmap */

typedef void (*__L4_Unmap_t)(L4_Word_t);
extern __L4_Unmap_t __L4_Unmap;

L4_INLINE void L4_Unmap (L4_Word_t control)
{
    register L4_Word_t r_control asm("$4");
    r_control = control;
	
    __asm__ __volatile__ (
        "lw $2, __L4_Unmap						\n\t"	
        "subu $29, $29, 0x10					\n\t"		
        "jalr $2								\n\t"
        "nop									\n\t"
        "addu $29, $29, 0x10					\n\t"
        :: "r"(r_control)
	);
	
    __asm__ __volatile__( "" ::: "$1", "$2", "$3", "$4", "$5", "$6", "$7", "$8", "$9", "$10", "$11", "$12", "$13", 
                          "$14", "$15", "$16", "$17", "$18", "$19", "$20", "$21", "$22", "$23", "$24", "$25", "$31" );					  

    //__asm__ __volatile__( "" ::: "$1", "$2", "$3", "$4", "$5", "$6", "$7", "$8", "$9", "$10", "$11", "$12", "$13", 
    //		"$14", "$15", "$16", "$17", "$18", "$19", "$20", "$21", "$22", "$23", "$24", "$25", "$31" );					  

}





/* Syscall ProcessorControl */

typedef L4_Word_t (*__L4_ProcessorControl_t)(L4_Word_t ProcessorNo,
                                             L4_Word_t InternalFrequency,
                                             L4_Word_t ExternalFrequency,
                                             L4_Word_t voltage);
extern __L4_ProcessorControl_t __L4_ProcessorControl;
    
L4_INLINE L4_Word_t L4_ProcessorControl (L4_Word_t ProcessorNo,
					 L4_Word_t InternalFrequency,
					 L4_Word_t ExternalFrequency,
					 L4_Word_t voltage)
{
    return( 0 ); // XXX
    //return __L4_ProcessorControl(ProcessorNo, InternalFrequency, ExternalFrequency, voltage);
}





/* Syscall MemoryControl */

typedef L4_Word_t (*__L4_MemoryControl_t)(L4_Word_t control,
                                          L4_Word_t attr0, L4_Word_t attr1,
                                          L4_Word_t attr2, L4_Word_t attr3);
extern __L4_MemoryControl_t __L4_MemoryControl;

L4_INLINE L4_Word_t L4_MemoryControl (L4_Word_t control,
                                      const L4_Word_t * attributes)
{
    return( 0 ); //XXX
//    return __L4_MemoryControl(control, attributes[0], attributes[1], attributes[2], attributes[3]);
}





/* Syscall ExchangeRegisters */

typedef L4_Word_t (*__L4_ExchangeRegisters_t)(L4_ThreadId_t dest,
                                              L4_Word_t control,
                                              L4_Word_t sp,
                                              L4_Word_t ip,
                                              L4_Word_t flags,
                                              L4_Word_t UserDefHandle,
                                              L4_ThreadId_t pager);
extern __L4_ExchangeRegisters_t __L4_ExchangeRegisters;

L4_INLINE L4_ThreadId_t L4_ExchangeRegisters (L4_ThreadId_t dest,
					      L4_Word_t control,
					      L4_Word_t sp,
					      L4_Word_t ip,
					      L4_Word_t flags,
					      L4_Word_t UserDefHandle,
					      L4_ThreadId_t pager,
					      L4_Word_t *old_control,
					      L4_Word_t *old_sp,
					      L4_Word_t *old_ip,
					      L4_Word_t *old_flags,
					      L4_Word_t *old_UserDefHandle,
                                              L4_ThreadId_t *old_pager)
{

    register L4_Word_t r_dest		asm ("$4");	  // a0     
    register L4_Word_t r_control	asm ("$5");	  // a1
    register L4_Word_t r_sp			asm ("$6");	  // a2
    register L4_Word_t r_ip			asm ("$7");	  // a3
    register L4_Word_t r_flags		asm ("$16");  // s0	 
    register L4_Word_t r_userhandle	asm ("$17");  // s1	
    register L4_Word_t r_pager		asm ("$18");  // s2
    register L4_Word_t r_local		asm ("$19");  // s3

    L4_Word_t res0;   
    L4_Word_t res1;
    L4_Word_t res2;
    L4_Word_t res3;
    L4_Word_t res4;	
    L4_Word_t res5;
    L4_Word_t res6;

    L4_ThreadId_t ret;
	
    r_dest = dest.raw;
    r_control = control;
    r_sp = sp;
    r_ip = ip;
    r_flags = flags;
    r_userhandle = UserDefHandle;
    r_pager = pager.raw;
    r_local = 0; // XXX

    __asm__ __volatile__ (
        "lw $2, __L4_ExchangeRegisters			\n\t"	
        "subu $29, $29, 0x10					\n\t"		
        "jalr $2								\n\t"
        "nop									\n\t"
        "addu $29, $29, 0x10					\n\t"
        "move %0, $2							\n\t"  /* result */
        "move %1, $4							\n\t"  /* result */
        "move %2, $5							\n\t"  /* result */
        "move %3, $6							\n\t"  /* result */
        "move %4, $7							\n\t"  /* result */
        "move %5, $8							\n\t"  /* result */
        "move %6, $9							\n\t"  /* result */
        : "=r"(res0), "=r"(res1), "=r"(res2), "=r"(res3), "=r"(res4), "=r"(res5), "=r"(res6)
        : "r"(r_dest), "r"(r_control), "r"(r_sp), "r"(r_ip), "r"(r_flags), "r"(r_userhandle), "r"(r_pager), "r"(r_local) 
	);

    //__asm__ __volatile__( "" ::: "$1", "$3", "$10", "$11", "$12", "$13", "$14", "$15", "$16", "$17", "$18", "$19", 
    //		"$20", "$21", "$22", "$23", "$24", "$25", "$31" );					  


    __asm__ __volatile__( "" ::: "$1", "$2", "$3", "$4", "$5", "$6", "$7", "$8", "$9", "$10", "$11", "$12", "$13", 
                          "$14", "$15", "$16", "$17", "$18", "$19", "$20", "$21", "$22", "$23", "$24", "$25", "$31" );					  


    if( old_control ) *old_control = res1;
    if( old_sp ) *old_sp = res2;
    if( old_ip ) *old_ip = res3;
    if( old_flags ) *old_flags = res4;
    if( old_UserDefHandle ) *old_UserDefHandle = res5;
    if( old_pager )	*((L4_Word_t*)old_pager) = res6;

    ret.raw = res0;
    return( ret );
}




/* Syscall IPC */

typedef L4_ThreadId_t (*__L4_Ipc_t)(L4_ThreadId_t to, L4_ThreadId_t FromSpecifier, L4_Word_t Timeouts);
extern __L4_Ipc_t __L4_Ipc;

L4_INLINE L4_MsgTag_t L4_Ipc (L4_ThreadId_t to,
			      L4_ThreadId_t FromSpecifier,
			      L4_Word_t Timeouts,
			      L4_ThreadId_t * from)
{
		
    register L4_Word_t r_to			asm("$4");	// a0
    register L4_Word_t r_from		asm("$5");	// a1
    register L4_Word_t r_time		asm("$6");	// a2

    L4_Word_t res0;
    L4_MsgTag_t ret;
	
    register L4_Word_t mr0 asm ("$16"); // s0
    register L4_Word_t mr1 asm ("$17"); // s1
    register L4_Word_t mr2 asm ("$18"); // s2
    register L4_Word_t mr3 asm ("$19"); // s3
    register L4_Word_t mr4 asm ("$20"); // s4
    register L4_Word_t mr5 asm ("$21"); // s5
    register L4_Word_t mr6 asm ("$22"); // s6
    register L4_Word_t mr7 asm ("$23"); // s7

    r_to = to.raw;
    r_from = FromSpecifier.raw;
    r_time = Timeouts;

    if( !L4_IsNilThread( to ) ) {

        mr0 = (__L4_Mips32_Utcb())[__L4_TCR_MR_OFFSET + 0];
        mr1 = (__L4_Mips32_Utcb())[__L4_TCR_MR_OFFSET + 1];
        mr2 = (__L4_Mips32_Utcb())[__L4_TCR_MR_OFFSET + 2];
        mr3 = (__L4_Mips32_Utcb())[__L4_TCR_MR_OFFSET + 3];
        mr4 = (__L4_Mips32_Utcb())[__L4_TCR_MR_OFFSET + 4];
        mr5 = (__L4_Mips32_Utcb())[__L4_TCR_MR_OFFSET + 5];
        mr6 = (__L4_Mips32_Utcb())[__L4_TCR_MR_OFFSET + 6];
        mr7 = (__L4_Mips32_Utcb())[__L4_TCR_MR_OFFSET + 7];

        __asm__ __volatile__(""::"r" (mr0), "r" (mr1), "r" (mr2), "r" (mr3), "r" (mr4), "r" (mr5), "r" (mr6), "r" (mr7));

    }

    // result = __L4_Ipc(to, FromSpecifier, Timeouts);
    __asm__ __volatile__(
        "lw $2, __L4_Ipc			\n\t"	
        "subu $29, $29, 0x10		\n\t"		
        "jalr $2					\n\t"
        "nop									\n\t"
        "addu $29, $29, 0x10		\n\t"
        "move %0, $2				\n\t"  /* result */
        : "=r"(res0), "=r" (mr0), "=r" (mr1), "=r" (mr2), "=r" (mr3), "=r" (mr4), "=r" (mr5), "=r" (mr6), "=r" (mr7)
        : "r"(r_to), "r"(r_from), "r"(r_time)
	);

    __asm__ __volatile__( "" ::: "$1", "$2", "$3", "$4", "$5", "$6", "$7", "$8", "$9", "$10", "$11", "$12", "$13", 
                          "$14", "$15", "$24", "$25", "$31" );					  


    //__asm__ __volatile__( "" ::: "$1", "$3", "$4", "$5", "$6", "$7", "$8", "$9", "$10", "$11", "$12", "$13", "$14", "$15", "$24", "$25", "$31" );					  
	
    if( !L4_IsNilThread( FromSpecifier ) ) {
        (__L4_Mips32_Utcb())[__L4_TCR_MR_OFFSET + 1] = mr1;
        (__L4_Mips32_Utcb())[__L4_TCR_MR_OFFSET + 2] = mr2;
        (__L4_Mips32_Utcb())[__L4_TCR_MR_OFFSET + 3] = mr3;
        (__L4_Mips32_Utcb())[__L4_TCR_MR_OFFSET + 4] = mr4;
        (__L4_Mips32_Utcb())[__L4_TCR_MR_OFFSET + 5] = mr5;
        (__L4_Mips32_Utcb())[__L4_TCR_MR_OFFSET + 6] = mr6;
        (__L4_Mips32_Utcb())[__L4_TCR_MR_OFFSET + 7] = mr7;
        if( from ) *((L4_Word_t*)from) = res0;
    }

    ret.raw = mr0;
    return( ret );

}



/* Syscall LIPC */

typedef L4_ThreadId_t (*__L4_Lipc_t)(L4_ThreadId_t to, L4_ThreadId_t FromSpecifier, L4_Word_t Timeouts);
extern __L4_Lipc_t __L4_Lipc;

L4_INLINE L4_MsgTag_t L4_Lipc (L4_ThreadId_t to,
			       L4_ThreadId_t FromSpecifier,
			       L4_Word_t Timeouts,
			       L4_ThreadId_t * from)
{
    //XXX does not work with local tid... adjust syscall exchange regs first
    return( L4_Ipc( to, FromSpecifier, Timeouts, from ) );
}

#endif /* !__L4__MIPS32__SYSCALLS_H__ */
