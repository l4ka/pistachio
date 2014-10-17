#include <l4/arm/asm.h>
#include <l4/arm/vregs_asm.h>

#define SYSCALL_thread_switch 0x4
	
BEGIN_PROC(mutex_lock)
	/* r0 contains the lock address */
	stmdb	sp!, {r4, r5, lr} /* r4 and r5 are local variables, 
				    lr needed when we do the syscall */

	/* r4 -- saved lock address */
	mov	r4, r0
	/* Here we load the utcb address */
	mov	r3, #0xff000000
	/* r5 -- saved UTCB address */
	ldr	r5, [r3, #0xff0]

	/* From here r4 and r5 are safe, the thread_switch
		syscall will not trash these registers */
	
	/* r0 is tmp */
	/* First we set the point at which we want to
	   be restarted in case of preemption */
	adr	r0, preempt_handler
	str	r0, [r5, #__L4_TCR_PREEMPT_CALLBACK_IP*4]
	/* And then we enable the preemption callback to
	   occur */
	mov	r0, #32
	strb	r0, [r5, #__L4_TCR_PREEMPT_FLAGS*4]
preempt_handler:                  /* If preempt we restart here */
	/* r0 is lock holder */
	ldr	r0, [r4]
	/* r1 is me */
	ldr	r1, [r5]
	/* test if lock holder is == 0 */
	cmp	r0, #0
	beq	2f
	/* or if lock holder is me */
	cmp	r0, r1
	beq	3f          /* We already have the lock so we jump forward to
                               the part where we turn off preemption and return */
	/* we couldn't get the lock so we fall through to here  */
	/* r0 holds lock holder, will be argument to the system call */
	/* r1 is temp */
	mov	r1, #1
	str	r1, [r4, #4] /* Let current lock holder know there is contention
                                so that it knows to yield at the end of its timeslice */
	/* Determine syscall address */
	mov	ip, sp
	mov	sp, #(0xffffff00 + SYSCALL_thread_switch);
	/* After syscall return to preempt_handler */
	adr	lr, preempt_handler
	/* do the system call */
	swi	0x00
2:
	/* The lock is free -- we try to grab it before being preempted */
	/* r0 is tmp */
	mov	r0, #0
	str	r0, [r4, #4] /* If we get this far, then noone of a higher priority than
		                us wants the lock, so we can unset the yield needed flag */
	/* Now we store ourself as the lock handler, this is transaction complete, although
	   we still might be preempted right here, in which case we validaly have the lock
	   and the preempt handler will go through sucessfully */
	str	r1, [r4]
	strb	r0, [r5, #__L4_TCR_PREEMPT_FLAGS*4]
	ldmia	sp!, {r4, r5, pc} /* RETURN POINT */
3:
	/* Exit path that occurs if we were preempted, before returning --
	   same as above, however we need to zero, r0 first */
	mov	r0, #0
	strb	r0, [r5, #__L4_TCR_PREEMPT_FLAGS*4]
	ldmia	sp!, {r4, r5, pc} /* RETURN POINT */
END_PROC(mutex_lock)
