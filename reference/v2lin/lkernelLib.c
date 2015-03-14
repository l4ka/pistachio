/*****************************************************************************
 * kernelLib.c - defines the functions and data structures needed
 *               to initialize a v2pthreads virtual machine in a POSIX Threads
 *               environment.
 *  
 * Copyright (C) 2000, 2001  MontaVista Software Inc.
 *
 * Author : Gary S. Robertson
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 ****************************************************************************/

#include <errno.h>
#include <unistd.h>
#include <sched.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>
#include "v2pthread.h"
#include "vxw_defs.h"

/*
**  process_timer_list is a system function used to service watchdog timers
**                     when a system clock tick expires.  It is called from
**                     the system exception task once per clock tick.
*/
extern void
   process_timer_list( void );

extern void
   taskLock( void );
extern void
   taskUnlock( void );
extern STATUS
   taskDelay( int interval );
extern STATUS
    taskInit( v2pthread_cb_t *tcb, char *name, int pri, int opts,
              char *pstack, int stksize,
              int (*funcptr)( int,int,int,int,int,int,int,int,int,int ),
              int arg1, int arg2, int arg3, int arg4, int arg5,
              int arg6, int arg7, int arg8, int arg9, int arg10 );
extern STATUS
    taskActivate( int tid );

/*****************************************************************************
**  v2pthread Global Data Structures
*****************************************************************************/
/*
**  Task control blocks for the v2pthread system tasks.
*/
static v2pthread_cb_t
    root_tcb;
static v2pthread_cb_t
    excp_tcb;

/*
**  task_list is a linked list of pthread task control blocks.
**            It is used to perform en-masse operations on all v2pthread
**            tasks at once.
*/
extern v2pthread_cb_t *
    task_list;

/*
**  task_list_lock is a mutex used to serialize access to the task list
*/
extern pthread_mutex_t
    task_list_lock;

/*
**  round_robin_enabled is a system-wide mode flag indicating whether the
**                      v2pthread scheduler is to use FIFO or Round Robin
**                      scheduling.
*/
static unsigned char
    round_robin_enabled = 0;

/*****************************************************************************
** round-robin control 
*****************************************************************************/
void disableRoundRobin( void )
{
    round_robin_enabled = 0;
}

void enableRoundRobin( void )
{
    round_robin_enabled = 1;
}

BOOL
   roundRobinIsEnabled( void )
{
    return( (BOOL)round_robin_enabled );
}

/*****************************************************************************
** kernelTimeSlice - turns Round-Robin Timeslicing on or off in the scheduler
*****************************************************************************/
STATUS
    kernelTimeSlice( int ticks_per_quantum )
{
    v2pthread_cb_t *tcb;
    int sched_policy;

    taskLock();

    /*
    **  Linux doesn't allow the round-robin quantum to be changed, so
    **  we only use the ticks_per_quantum as an on/off value for
    **  round-robin scheduling.
    */
    if ( ticks_per_quantum == 0 )
    {
        /*
        **  Ensure Round-Robin Timeslicing is OFF for all tasks, both
        **  existing and yet to be created.
        */
        round_robin_enabled = 0;
        sched_policy = SCHED_FIFO;
    }
    else
    {
        /*
        **  Ensure Round-Robin Timeslicing is ON for all tasks, both
        **  existing and yet to be created.
        */
        round_robin_enabled = 1;
        sched_policy = SCHED_RR;
    }

    if ( task_list != (v2pthread_cb_t *)NULL )
    {
        struct sched_param schedparam;
        /*
        **  Change the scheduling policy for all tasks in the task list.
        */
        for ( tcb = task_list; tcb != (v2pthread_cb_t *)NULL;
              tcb = tcb->nxt_task )
        {
	
            /*
            **  First set the new scheduling policy attribute.  Since the
            **  max priorities are identical under Linux for both real time
            **  scheduling policies, we know we don't have to change priority.
            */
            pthread_attr_setschedpolicy( &(tcb->attr), sched_policy );

	    pthread_attr_getschedparam( &(tcb->attr), &schedparam );
            /*
            **  Activate the new scheduling policy
            */
            if (0 != pthread_setschedparam( tcb->pthrid, sched_policy,
					   &schedparam ))
	          {
                perror( "\r\nkernelTimeSlice pthread_setschedparam returned error:" );
	          }
        }
    }

    taskUnlock();

    return( OK );
}

/*****************************************************************************
**  system exception task
**
**  In the v2pthreads environment, the exception task serves only to
**  handle watchdog timer functions and to allow self-restarting of other
**  v2pthread tasks.
*****************************************************************************/
int exception_task( int dummy0, int dummy1, int dummy2, int dummy3,
                    int dummy4, int dummy5, int dummy6, int dummy7,
                    int dummy8, int dummy9 )
{

    while ( 1 )
    {
        /*
        **  Process system watchdog timers (if any are defined).
        **  NOTE that since ALL timers must be handled during a single
        **  10 msec system clock tick, timers should be used sparingly.
        **  In addition, the timeout functions called by watchdog timers
        **  should be "short and sweet".
        */
        process_timer_list();

        /*
        **  Delay for one timer tick.  Since this is the highest-priority
        **  task in the v2pthreads virtual machine (except for the root task,
        **  which stays blocked almost all the time), any processing done
        **  in this task can impose a heavy load on the remaining tasks.
        **  For this reason, this task and all watchdog timeout functions
        **  should be kept as brief as possible.
        */
        taskDelay( 1 );
    }

    return( 0 );
}

/*****************************************************************************
**  v2pthread main program
**
**  This function serves as the entry point to the v2pthreads emulation
**  environment.  It serves as the parent process to all v2pthread tasks.
**  This process creates an initialization thread and sets the priority of
**  that thread to the highest allowable value.  This allows the initialization
**  thread to complete its work without being preempted by any of the task
**  threads it creates.
*****************************************************************************/
int v2lin_init(void)
{
    int max_priority;

    /*
    **  Set up a v2pthread task and TCB for the system root task.
    */
#ifdef DIAG_PRINTFS 
    printf( "\r\nStarting System Root Task" );
#endif
    taskInit( &root_tcb, "tUsrRoot", 0, 0, 0, 0, NULL,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );

    /*
    **  Get the maximum permissible priority level for the current OS
    **  and make that the pthreads priority for the root task.
    */
    max_priority = sched_get_priority_max( SCHED_FIFO );
    (root_tcb.prv_priority).sched_priority = max_priority;
    pthread_attr_setschedparam( &(root_tcb.attr), &(root_tcb.prv_priority) );
	root_tcb.state = READY;
	root_tcb.pthrid = pthread_self();
	taskActivate( root_tcb.taskid );
	
    /*
    **  Set up a v2pthread task and TCB for the system exception task.
    */
#ifdef DIAG_PRINTFS 
    printf( "\r\nStarting System Exception Task" );
#endif
    taskInit( &excp_tcb, "tExcTask", 0, 0, 0, 0, exception_task,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );

    /*
    **  Get the maximum permissible priority level for a pthread
    **  and make that the pthreads priority for the exception task.
    */
    max_priority = sched_get_priority_max( SCHED_FIFO );
    (excp_tcb.prv_priority).sched_priority = (max_priority - 1);
    pthread_attr_setschedparam( &(excp_tcb.attr), &(excp_tcb.prv_priority) );

    taskActivate( excp_tcb.taskid );

    errno = 0;
    return errno;
}
