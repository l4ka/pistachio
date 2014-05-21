/*****************************************************************************
 * taskLib.c - defines the wrapper functions and data structures needed
 *             to implement a Wind River VxWorks (R) task control API 
 *             in a POSIX Threads environment.
 *  
 * Copyright (C) 2000, 2001  MontaVista Software Inc.
 *
 * Author : Gary S. Robertson
 *
 * VxWorks is a registered trademark of Wind River Systems, Inc.
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
**  selfRestart is a system function used by a task to restart itself.
**              The function creates a temporary watchdog timer which restarts 
**              the terminated task and then deletes itself. 
*/
extern void
   selfRestart( v2pthread_cb_t *restart_tcb );

extern BOOL
   roundRobinIsEnabled( void );

/*****************************************************************************
**  v2pthread Global Data Structures
*****************************************************************************/
/*
**  task_list is a linked list of pthread task control blocks.
**            It is used to perform en-masse operations on all v2pthread
**            tasks at once.
*/
v2pthread_cb_t *
    task_list = (v2pthread_cb_t *)NULL;

/*
**  task_list_lock is a mutex used to serialize access to the task list
*/
pthread_mutex_t
    task_list_lock = PTHREAD_MUTEX_INITIALIZER;

/*
**  v2pthread_task_lock is a mutex used to make taskLock exclusive to one
**                    thread at a time.
*/
pthread_mutex_t
    v2pthread_task_lock = PTHREAD_MUTEX_INITIALIZER;

/*
**  scheduler_locked contains the pthread ID of the thread which currently
**                   has the scheduler locked (or NULL if it is unlocked).
*/
static pthread_t
    scheduler_locked = (pthread_t)NULL;

/*
**  taskLock_level tracks recursive nesting levels of taskLock/unlock calls
**                 so the scheduler is only unlocked at the outermost
**                 taskUnlock call.
*/
static unsigned long
    taskLock_level = 0;

/*
**  taskLock_change is a condition variable which signals a change from
**                  locked to unlocked or vice-versa.
*/
static pthread_cond_t
    taskLock_change = PTHREAD_COND_INITIALIZER;

/*****************************************************************************
**  thread-safe malloc
*****************************************************************************/
void *ts_malloc( size_t blksize )
{
   void *blkaddr;
    static pthread_mutex_t
        malloc_lock = PTHREAD_MUTEX_INITIALIZER;

    pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                          (void *)&malloc_lock );
    pthread_mutex_lock( &malloc_lock );

    blkaddr = malloc( blksize );

    pthread_cleanup_pop( 1 );

    return( blkaddr );
}
    
/*****************************************************************************
**  thread-safe free
*****************************************************************************/
void ts_free( void *blkaddr )

{
    static pthread_mutex_t
        free_lock = PTHREAD_MUTEX_INITIALIZER;

    pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                          (void *)&free_lock );
    pthread_mutex_lock( &free_lock );

    free( blkaddr );

    pthread_cleanup_pop( 1 );
}
    
/*****************************************************************************
**  my_tcb - returns a pointer to the task control block for the calling task
*****************************************************************************/
v2pthread_cb_t *
   my_tcb( void )
{
    pthread_t my_pthrid;
    v2pthread_cb_t *current_tcb;

    /*
    **  Get caller's pthread ID
    */
    my_pthrid = pthread_self();

    /*
    **  If the task_list contains tasks, scan it for the tcb
    **  whose thread id matches the one to be deleted.  No locking
    **  of the task list is done here since the access is read-only.
    **  NOTE that a tcb being appended to the task_list MUST have its
    **  nxt_task member initialized to NULL before being linked into
    **  the list. 
    */
    if ( task_list != (v2pthread_cb_t *)NULL )
    {
        for ( current_tcb = task_list;
              current_tcb != (v2pthread_cb_t *)NULL;
              current_tcb = current_tcb->nxt_task )
        {
            if ( my_pthrid == current_tcb->pthrid )
            {
                /*
                **  Found the task control_block.
                */
                return( current_tcb );
            }
        }
    }

    /*
    **  No matching task found... return NULL
    */
    return( (v2pthread_cb_t *)NULL );
}

/*****************************************************************************
** tcb_for - returns the address of the task control block for the task
**           idenified by taskid
*****************************************************************************/
v2pthread_cb_t *
   tcb_for( int taskid )
{
    v2pthread_cb_t *current_tcb;
    int found_taskid;

        if ( task_list != (v2pthread_cb_t *)NULL )
        {
            /*
            **  One or more tasks already exist in the task list...
            **  Scan the existing tasks for a matching ID.
            */
            found_taskid = FALSE;
            for ( current_tcb = task_list; 
                  current_tcb != (v2pthread_cb_t *)NULL;
                  current_tcb = current_tcb->nxt_task )
            {
                if ( current_tcb->taskid == taskid )
                {
                    found_taskid = TRUE;
                    break;
                }
            }
            if ( found_taskid == FALSE )
                /*
                **  No matching ID found
                */
                current_tcb = (v2pthread_cb_t *)NULL;
        }
        else
            current_tcb = (v2pthread_cb_t *)NULL;
 
    return( current_tcb );
}

/*****************************************************************************
** taskLock - 'locks the scheduler' to prevent preemption of the current task
**           by other task-level code.  Because we cannot actually lock the
**           scheduler in a pthreads environment, we temporarily set the
**           dynamic priority of the calling thread above that of any other
**           thread, thus guaranteeing that no other tasks preempt it.
*****************************************************************************/
void
   taskLock( void )
{
    pthread_t my_pthrid;
    v2pthread_cb_t *tcb;
    int max_priority, sched_policy, got_lock;

    /*
    **  v2pthread_task_lock ensures that only one v2pthread pthread at a time gets
    **  to run at max_priority (effectively locking out all other v2pthread
    **  pthreads).  Due to the semantics of the pthread_cleanup push/pop
    **  pairs (which protect against deadlocks in the event a thread gets
    **  terminated while holding the mutex lock), we cannot safely leave
    **  the mutex itself locked until taskUnlock() is called.  Therefore,
    **  we instead use the mutex to provide 'atomic access' to a global
    **  flag indicating if the scheduler is currently locked.  We will
    **  'spin' and briefly suspend until the scheduler is unlocked, and
    **  will then lock it ourselves before proceeding.
    */
    got_lock = FALSE;
    my_pthrid = pthread_self();

    /*
    **  'Spin' here until scheduler_locked == NULL or our pthread ID
    **  This effectively prevents more than one pthread at a time from
    **  setting its priority to max_priority.
    */
    do {
        /*
        **  The pthread_cleanup_push/pop pair ensure the mutex will be
        **  unlocked if the calling thread gets killed within this loop.
        */
        pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                              (void *)&v2pthread_task_lock );
        /*
        **  The mutex lock/unlock guarantees 'atomic' access to the
        **  scheduler_locked flag.  Locking via pthread ID allows recursive
        **  locking by the same pthread while excluding all other pthreads.
        */
        pthread_mutex_lock( &v2pthread_task_lock );
        if ( (scheduler_locked == (pthread_t)NULL) ||
             (scheduler_locked == my_pthrid) )
        {
            scheduler_locked = my_pthrid;
            taskLock_level++;
            if ( taskLock_level == 0L )
                taskLock_level--;
            got_lock = TRUE;
            pthread_cond_broadcast( &taskLock_change );
#ifdef DIAG_PRINTFS 
            printf( "\r\ntaskLock taskLock_level %lu locking tid %ld",
                taskLock_level,  scheduler_locked );
#endif
        }
        else
        {
#ifdef DIAG_PRINTFS 
            printf( "\r\ntaskLock locking tid %ld my tid %ld",
                    scheduler_locked, my_pthrid );
#endif
            pthread_cond_wait( &taskLock_change, &v2pthread_task_lock );
        }
        pthread_mutex_unlock( &v2pthread_task_lock );

        /*
        **  Add a cancellation point to this loop, since there are no others.
        */
        pthread_testcancel();
        pthread_cleanup_pop( 0 );
    } while ( got_lock == FALSE );

    /*
    **  task_list_lock prevents other v2pthread pthreads from modifying
    **  the v2pthread pthread task list while we're searching it and modifying
    **  the calling task's priority level.
    */
    pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                          (void *)&task_list_lock );
    pthread_mutex_lock( &task_list_lock );
    tcb = my_tcb();
    if ( tcb != (v2pthread_cb_t *)NULL )
    {
	struct sched_param schedparam;
        pthread_attr_getschedpolicy( &(tcb->attr), &sched_policy );
	pthread_attr_getschedparam( &(tcb->attr), &schedparam );
        max_priority = sched_get_priority_max( sched_policy );
        schedparam.sched_priority = max_priority;
	pthread_attr_setschedparam( &(tcb->attr), &schedparam );
        pthread_setschedparam( tcb->pthrid, sched_policy, &schedparam );
    }
    pthread_cleanup_pop( 1 );
}

/*****************************************************************************
** taskUnlock - 'unlocks the scheduler' to allow preemption of the current
**             task by other task-level code.  Because we cannot actually lock
**             the scheduler in a pthreads environment, the dynamic priority of
**             the calling thread was temporarily raised above that of any
**             other thread.  Therefore, we now restore the priority of the
**             calling thread to its original value to 'unlock' the task
**             scheduler.
*****************************************************************************/
void
   taskUnlock( void )
{
    v2pthread_cb_t *tcb;
    int sched_policy;

    /*
    **  scheduler_locked ensures that only one v2pthread pthread at a time gets
    **  to run at max_priority (effectively locking out all other v2pthread
    **  pthreads).  Unlock it here to complete 'unlocking' of the scheduler.
    */
    pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                          (void *)&v2pthread_task_lock );
    pthread_mutex_lock( &v2pthread_task_lock );

    if ( scheduler_locked == pthread_self() )
    {
        if ( taskLock_level > 0L )
            taskLock_level--;
        if ( taskLock_level < 1L )
        {
            /*
            **  task_list_lock prevents other v2pthread pthreads from modifying
            **  the v2pthread pthread task list while we're searching it and
            **  modifying the calling task's priority level.
            */
            pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                                  (void *)&task_list_lock );
            pthread_mutex_lock( &task_list_lock );
            tcb = my_tcb();
            if ( tcb != (v2pthread_cb_t *)NULL )
            {
		struct sched_param schedparam;
                pthread_attr_getschedpolicy( &(tcb->attr), &sched_policy );
		pthread_attr_getschedparam( &(tcb->attr), &schedparam );
                schedparam.sched_priority = 
                                           tcb->prv_priority.sched_priority;
		
		pthread_attr_setschedparam( &(tcb->attr), &schedparam );
                pthread_setschedparam( tcb->pthrid, sched_policy,&schedparam );
            }
            pthread_cleanup_pop( 1 );

            scheduler_locked = (pthread_t)NULL;
            pthread_cond_broadcast( &taskLock_change );
        }
#ifdef DIAG_PRINTFS 
        printf( "\r\ntaskUnlock taskLock_level %lu locking tid %ld",
                taskLock_level,  scheduler_locked );
#endif
    }
#ifdef DIAG_PRINTFS 
    else
        printf( "\r\ntaskUnlock locking tid %ld my tid %ld", scheduler_locked,
                pthread_self() );
#endif

    pthread_cleanup_pop( 1 );
}

/*****************************************************************************
** link_susp_tcb - appends a new tcb pointer to a linked list of tcb pointers
**                 for tasks suspended on the object owning the list.
*****************************************************************************/
void
   link_susp_tcb( v2pthread_cb_t **list_head, v2pthread_cb_t *new_entry )
{
    v2pthread_cb_t *nxt_entry;

    if ( list_head != (v2pthread_cb_t **)NULL )
    {
        pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                              (void *)&task_list_lock );
        pthread_mutex_lock( &task_list_lock );
        new_entry->nxt_susp = (v2pthread_cb_t *)NULL;
        if ( *list_head != (v2pthread_cb_t *)NULL )
        {
            for ( nxt_entry = *list_head; 
                  nxt_entry->nxt_susp != (v2pthread_cb_t *)NULL;
                  nxt_entry = nxt_entry->nxt_susp ) ;
            nxt_entry->nxt_susp = new_entry;
#ifdef DIAG_PRINTFS 
            printf( "\r\nadd susp_tcb @ %p to list @ %p", new_entry,
                    nxt_entry );
#endif
        }
        else
        {
            *list_head = new_entry;
#ifdef DIAG_PRINTFS 
            printf( "\r\nadd susp_tcb @ %p to list @ %p", new_entry,
                    list_head );
#endif
        }
        /*
        **  Initialize the suspended task's pointer back to suspend list
        **  This is used for cleanup during task deletion.
        */
        new_entry->suspend_list = list_head;

        /*
        **  Update the task state.
        */
        new_entry->state |= PEND;

        pthread_cleanup_pop( 1 );
    }
}

/*****************************************************************************
** unlink_susp_tcb - removes tcb pointer from a linked list of tcb pointers
**                   for tasks suspended on the object owning the list.
*****************************************************************************/
void
   unlink_susp_tcb( v2pthread_cb_t **list_head, v2pthread_cb_t *entry )
{
    v2pthread_cb_t *current_tcb;

    if ( list_head != (v2pthread_cb_t **)NULL )
    {
        pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                              (void *)&task_list_lock );
        pthread_mutex_lock( &task_list_lock );
        if ( *list_head == entry )
        {
            *list_head = entry->nxt_susp;
#ifdef DIAG_PRINTFS 
            printf( "\r\ndel susp_tcb @ %p from list @ %p - newlist head %p",
                    entry, list_head, *list_head );
#endif
        }
        else
        {
            for ( current_tcb = *list_head;
                  current_tcb != (v2pthread_cb_t *)NULL;
                  current_tcb = current_tcb->nxt_susp )
            {
                if ( current_tcb->nxt_susp == entry )
                {
                    current_tcb->nxt_susp = entry->nxt_susp;
#ifdef DIAG_PRINTFS 
                    printf( "\r\ndel susp_tcb @ %p from list @ %p", entry,
                    current_tcb );
#endif
                }
            }
        }
        entry->nxt_susp = (v2pthread_cb_t *)NULL;

        /*
        **  Update the task state.
        */
        entry->state &= (!PEND);

        pthread_cleanup_pop( 1 );
    }

}

/*****************************************************************************
** signal_for_my_task - searches the specified 'pended task list' for the
**                      task to be selected according to the specified
**                      pend order.  If the selected task is the currently
**                      executing task, the task is deleted from the
**                      specified pended task list and returns a non-zero
**                      result... otherwise the pended task list is not
**                      modified and a zero result is returned.
*****************************************************************************/
int
   signal_for_my_task( v2pthread_cb_t **list_head, int pend_order )
{
    v2pthread_cb_t *signalled_task;
    v2pthread_cb_t *current_tcb;
    int result;

    result = FALSE;
#ifdef DIAG_PRINTFS 
    printf( "\r\nsignal_for_my_task - list head = %p", *list_head );
#endif
    if ( list_head != (v2pthread_cb_t **)NULL )
    {
        signalled_task = *list_head;

        /*
        **  First determine which task is being signalled
        */
        if ( pend_order != 0 )
        {
            /*
            **  Tasks pend in priority order... locate the highest priority
            **  task in the pended list.
            */
            for ( current_tcb = *list_head;
                  current_tcb != (v2pthread_cb_t *)NULL;
                  current_tcb = current_tcb->nxt_susp )
            {
                if ( (current_tcb->prv_priority).sched_priority >
                     (signalled_task->prv_priority).sched_priority )
                    signalled_task = current_tcb;
#ifdef DIAG_PRINTFS 
                printf( "\r\nsignal_for_my_task - tcb @ %p priority %d",
                        current_tcb,
                        (current_tcb->prv_priority).sched_priority );
#endif
            }
        }
            /*
        else
            **
            ** Tasks pend in FIFO order... signal is for task at list head.
            */

        /*
        **  Signalled task located... see if it's the currently executing task.
        */
        if ( signalled_task == my_tcb() )
        {
            /*
            **  The currently executing task is being signalled...
            */
            result = TRUE;
        }
#ifdef DIAG_PRINTFS 
        printf( "\r\nsignal_for_my_task - signalled tcb @ %p my tcb @ %p",
                        signalled_task, my_tcb() );
#endif
    }

    return( result );
}

/*****************************************************************************
** new_tid - assigns the next unused task ID for the caller's task
*****************************************************************************/
static int
   new_tid( void )
{
    v2pthread_cb_t *current_tcb;
    int new_taskid;

    /*
    **  Get the highest previously assigned task id and add one.
    */
    if ( task_list != (v2pthread_cb_t *)NULL )
    {
        /*
        **  One or more tasks already exist in the task list...
        **  Find the highest task ID number in the existing list.
        */
        new_taskid = 0;
        for ( current_tcb = task_list; 
              current_tcb->nxt_task != (v2pthread_cb_t *)NULL;
              current_tcb = current_tcb->nxt_task )
        {
            /*
            **  We use a kluge here to prevent address-based task IDs created
            **  by explicit taskInit calls from polluting the normal sequence
            **  for task ID numbers.  This is based on the PROBABILITY that
            **  Linux will not allocate pthread data space below the 64K
            **  address space.  NOTE that this will BREAK taskSpawn if you
            **  create 64K tasks or more.
            */
            if ( ((current_tcb->nxt_task)->taskid < 65536) &&
                 ((current_tcb->nxt_task)->taskid > new_taskid) )
            {
                new_taskid = (current_tcb->nxt_task)->taskid;
            }
        }

        /*
        **  Add one to the highest existing task ID
        */
        new_taskid++;
    }
    else
    {
        /*
        **  this is the first task being added to the task list.
        */
        new_taskid = 1;
    }

    return( new_taskid );
}

/*****************************************************************************
** translate_priority - translates a v2pthread priority into a pthreads priority
*****************************************************************************/
static int
   translate_priority( int v2pthread_priority, int sched_policy, int *errp )
{
    int max_priority, min_priority, pthread_priority;

    /*
    **  Validate the range of the user's task priority.
    */
    if ( (v2pthread_priority > MIN_V2PT_PRIORITY) | 
         (v2pthread_priority < MAX_V2PT_PRIORITY) )
        *errp = S_taskLib_ILLEGAL_PRIORITY;
 
    /*
    **  Translate the v2pthread priority into a pthreads priority.
    **  'Invert' the caller's priority so highest priority == highest number.
    */
    pthread_priority = MIN_V2PT_PRIORITY - v2pthread_priority;

    /*
    **  Next get the allowable priority range for the scheduling policy.
    */
    min_priority = sched_get_priority_min( sched_policy );
    max_priority = sched_get_priority_max( sched_policy );

    /*
    **  'Telescope' the v2pthread priority (0-255) into the smaller pthreads
    **  priority range (1-97) on a proportional basis.  NOTE that this may
    **  collapse some distinct v2pthread priority levels into the same priority.
    **  Use this technique if the tasks span a large priority range but no
    **  tasks are within fewer than 3 levels of one another.
    */
    pthread_priority *= max_priority;
    pthread_priority /= (MIN_V2PT_PRIORITY + 1);

    /*
    **  Now 'clip' the new priority level to within priority range.
    **  Reserve max_priority level for temporary use during system calls.
    **  Reserve (max_priority level - 1) for the system exception task.
    **  NOTE that relative v2pthread priorities may not translate properly
    **  if the v2pthread priorities used span several multiples of max_priority.
    **  Use this technique if the tasks span a small priority range and their
    **  relative priorities are within 3 or fewer levels of one another.
    */
    pthread_priority %= (max_priority - 1);
    if ( pthread_priority < min_priority )
            pthread_priority = min_priority;

    return( pthread_priority );
}

/*****************************************************************************
** tcb_delete - deletes a pthread task control block from the task_list
**              and frees the memory allocated for the tcb
*****************************************************************************/
static void
   tcb_delete( v2pthread_cb_t *tcb )
{
    v2pthread_cb_t *current_tcb;

    /*
    **  If the task_list contains tasks, scan it for a link to the tcb
    **  being deleted.
    */
    if ( task_list != (v2pthread_cb_t *)NULL )
    {
        /*
        **  Remove the task from the suspend list for any object it
        **  is pending on.
        */
#ifdef DIAG_PRINTFS 
        printf( "\r\ntcb_delete - tcb @ %p suspend_list = %p", tcb,
                tcb->suspend_list );
        fflush( stdout );
#endif
        unlink_susp_tcb( tcb->suspend_list, tcb );
        pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                              (void *)&task_list_lock );
        pthread_mutex_lock( &task_list_lock );
        if ( tcb == task_list )
        {
            task_list = tcb->nxt_task;
        }
        else
        {
            for ( current_tcb = task_list;
                  current_tcb->nxt_task != (v2pthread_cb_t *)NULL;
                  current_tcb = current_tcb->nxt_task )
            {
                if ( tcb == current_tcb->nxt_task )
                {
                    /*
                    **  Found the tcb just prior to the one being deleted.
                    **  Unlink the tcb being deleted from the task_list.
                    */
#ifdef DIAG_PRINTFS 
                    printf( "\r\ntcb_delete - removing tcb @ %p from task list",
                            tcb );
                    fflush( stdout );
#endif
                    current_tcb->nxt_task = tcb->nxt_task;
                    break;
                }
            }
        }
        pthread_cleanup_pop( 1 );
    }

    /* Release the memory occupied by the tcb being deleted. */
    if ( tcb->taskname != (char *)NULL )
    {
        ts_free( (void *)tcb->taskname );
    }

    if ( !(tcb->static_tcb) )
        ts_free( (void *)tcb );
}

/*****************************************************************************
** notify_task_delete - notifies any pended tasks of the specified task's
**                      deletion.
*****************************************************************************/
void
   notify_task_delete( v2pthread_cb_t *tcb )
{
    /*
    **  Task just made deletable... ensure that we awaken any
    **  other tasks pended on deletion of this task
    **
    **  Lock mutex for task delete broadcast completion
    */
#ifdef DIAG_PRINTFS 
    printf( "\r\nnotify_task_delete - lock delete bcast mutex @ tcb %p",
            tcb );
#endif
    pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                          (void *)&(tcb->dbcst_lock) );
    pthread_mutex_lock( &(tcb->dbcst_lock) );

    /*
    **  Lock mutex for deletion condition variable
    */
#ifdef DIAG_PRINTFS 
    printf( "\r\nnotify_task_delete - lock delete cond var mutex @ tcb %p",
            tcb );
#endif
    pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                          (void *)&(tcb->tdelete_lock));
    pthread_mutex_lock( &(tcb->tdelete_lock) );

    /*
    **  Signal the condition variable for task deletion 
    */
#ifdef DIAG_PRINTFS 
    printf( "\r\nnotify_task_delete - bcast delete cond variable @ tcb %p",
            tcb );
#endif
    pthread_cond_broadcast( &(tcb->t_deletable) );

    /*
    **  Unlock the task deleton mutex. 
    */
#ifdef DIAG_PRINTFS 
    printf( "\r\nnotify_task_delete - unlock delete cond var mutex @ tcb %p",
            tcb );
#endif
    pthread_cleanup_pop( 1 );

    /*
    **  Wait for all pended tasks to receive deletion notification.
    **  The last task to receive the notification will signal the
    **  delete broadcast-complete condition variable.
    */
#ifdef DIAG_PRINTFS 
    printf( "\r\nnotify_task_delete - wait till pended tasks respond @ tcb %p",
            tcb );
#endif
    while ( tcb->first_susp != (v2pthread_cb_t *)NULL )
        pthread_cond_wait( &(tcb->delete_bcplt), &(tcb->dbcst_lock) );

#ifdef DIAG_PRINTFS 
    printf( "\r\nnotify_task_delete - all pended tasks responded @ tcb %p",
            tcb );
#endif
    /*
    **  Unlock the task delete broadcast completion mutex. 
    */
#ifdef DIAG_PRINTFS 
    printf( "\r\nnotify_task_delete - unlock delete bcast mutex @ tcb %p",
            tcb );
#endif
    pthread_cleanup_pop( 1 );
}


void tcb_delete_unlock(v2pthread_cb_t *tcb)
{
	tcb_delete(tcb);
	taskUnlock();
}


/*****************************************************************************
** taskDeleteForce - removes the specified task(s) from the task list,
**                   frees the memory occupied by the task control block(s),
**                   and kills the pthread(s) associated with the task(s).
*****************************************************************************/
STATUS
   taskDeleteForce( int tid )
{
    v2pthread_cb_t *current_tcb;
    v2pthread_cb_t *self_tcb;
    STATUS error;

    error = OK;

    taskLock();

    /*
    **  Delete the task whose taskid matches tid.
    **  If the task_list contains tasks, scan it for the tcb
    **  whose task id matches the one to be deleted.
    */
    self_tcb = my_tcb();
    if ( tid == 0 )
        /*
        **  NULL tid specifies current task - get TCB for current task
        */
        current_tcb = self_tcb;
    else
        /*
        **  Get TCB for task specified by tid
        */
        current_tcb = tcb_for( tid );

    /*
    **  Make sure valid TCB found in scan of task list
    */
    if ( current_tcb != (v2pthread_cb_t *)NULL )
    {
        /*
        **  Found the task being deleted... first ensure that we
        **  awaken any other tasks pended on deletion of this task
        */
        if ( current_tcb->first_susp != (v2pthread_cb_t *)NULL )
        {
            notify_task_delete( current_tcb );
        }

        /*
        **  Any tasks pended on deletion of this task have been awakened.
        **  Proceed with task deletion.
        */
#ifdef DIAG_PRINTFS 
        printf( "\r\ntaskDeleteForce - delete pthread for task @ tcb %p",
                current_tcb );
        fflush( stdout );
#endif
        if ( current_tcb != self_tcb )
        {
            /*
            **  Task being deleted is not the current task.
            **  Kill the task pthread and wait for it to die.
            **  Then de-allocate its data structures.
            */
#ifdef DIAG_PRINTFS 
            printf( "\r\ntaskDeleteForce - other tcb @ %p", current_tcb );
            fflush( stdout );
#endif
            pthread_cancel( current_tcb->pthrid );
            pthread_join( current_tcb->pthrid, (void **)NULL );
            tcb_delete( current_tcb );
        }
        else
        {
            /*
            **  Kill the currently executing task's pthread
            **  and then de-allocate its data structures.
            */
#ifdef DIAG_PRINTFS 
            printf( "\r\ntaskDeleteForce - self tcb @ %p", current_tcb );
            fflush( stdout );
#endif
            pthread_detach( self_tcb->pthrid );
            pthread_cleanup_push( (void(*)(void *))tcb_delete_unlock,
                                  (void *)self_tcb );
            pthread_exit( (void *)NULL );
            pthread_cleanup_pop( 0 );
        }
    }
    else
        /*
        **  No TCB found in task list for specified tid... must have already
        **  been deleted.
        */
        error = S_objLib_OBJ_DELETED;

    taskUnlock();

    if ( error != OK )
    {
        errno = (int)error;
        error = ERROR;
    }
    return( error );
}

/*****************************************************************************
**  cleanup_scheduler_lock ensures that a killed pthread releases the
**                         scheduler lock if it owned it.
*****************************************************************************/
static void 
    cleanup_scheduler_lock( void *tcb )
{
    v2pthread_cb_t *mytcb;

    mytcb = (v2pthread_cb_t *)tcb;
    pthread_mutex_lock( &v2pthread_task_lock );

    if ( scheduler_locked == pthread_self() )
    {
        taskLock_level = 0;
        scheduler_locked = (pthread_t)NULL;
    }
    pthread_mutex_unlock( &v2pthread_task_lock );
}

/*****************************************************************************
**  task_wrapper is a pthread used to 'contain' a v2pthread task.
*****************************************************************************/
void *
    task_wrapper( void *arg )
{
    v2pthread_cb_t *tcb;

#ifdef DEBUG_PRINTS
	printf("\ntask_wrapper, arg=%p, errno=%d\n", arg, errno);
#endif
    /*
    **  Ensure that errno for this thread is cleared.
    */
    errno = 0;
    
    /*
    **  Make a parameter block pointer from the caller's argument
    **  Then extract the needed info from the parameter block and
    **  free its memory before beginning the v2pthread task
    */
    tcb = (v2pthread_cb_t *)arg;

    /*
    **  Ensure that this pthread will release the scheduler lock if killed.
    */
    pthread_cleanup_push( cleanup_scheduler_lock, (void *)tcb );

    /*
    **  Call the v2pthread task.  Normally this is an endless loop and doesn't
    **  return here.
    */
#ifdef DIAG_PRINTFS
    printf( "\r\ntask_wrapper starting task @ %p tcb @ %p:",
            tcb->entry_point, tcb );
    sleep( 1 );
#endif
	
    /*
	 * TODO: replace this ugly patch with waiting for conditional variable on tcb->thrid
	 * or better yet, fill the tcb->pthrid with pthread_self(). Check if this can be done
	 * with atomic_xchange here and in the taskActivate function simultaneously
	 */
	sched_yield();
    while (0 == (volatile pthread_t)tcb->pthrid)
    {
    	sched_yield();
    	usleep( 5000 );
#ifdef DIAG_PRINTFS
    	printf("task_wrapper() PATCH! task (%s) wait for tcb->pthrid to get its value... \n", tcb->taskname ? tcb->taskname : "no-name");
#endif
    }
    (*(tcb->entry_point))( tcb->parms[0], tcb->parms[1], tcb->parms[2],
                           tcb->parms[3], tcb->parms[4], tcb->parms[5],
                           tcb->parms[6], tcb->parms[7], tcb->parms[8],
                           tcb->parms[9] );

    /*
    **  If for some reason the task above DOES return, clean up the
    **  pthread and task resources and kill the pthread.
    */
    pthread_cleanup_pop( 1 );

    /*
    **  NOTE taskDelete takes no action if the task has already been deleted.
    */
    taskDeleteForce( tcb->taskid );

    return( (void *)NULL );
}

/*****************************************************************************
** taskDelay - suspends the calling task for the specified number of ticks.
**            ( one tick is currently implemented as ten milliseconds )
*****************************************************************************/
STATUS
   taskDelay( int interval )
{
    struct timeval now, timeout;
    unsigned long usec;
    v2pthread_cb_t *tcb;

    /*
    **  Calculate timeout delay in seconds and microseconds
    */
    usec = (unsigned long)(interval * V2PT_TICK * 1000);

    /*
    **  Update the task state.
    */
    tcb = my_tcb();
    tcb->state |= DELAY;

    /*
    **  Delay of zero means yield CPU to other tasks of same priority
    */
    if ( usec > 0L )
    {
        /*
        **  Establish absolute time at expiration of delay interval
        */
        gettimeofday( &now, (struct timezone *)NULL );
        timeout.tv_sec = now.tv_sec;
        timeout.tv_usec = now.tv_usec;
        timeout.tv_usec += usec;
        if ( timeout.tv_usec > 1000000 )
        {
            timeout.tv_sec += timeout.tv_usec / 1000000;
            timeout.tv_usec = timeout.tv_usec % 1000000;
        }

        /*
        **  Wait for the current time of day to reach the time of day calculated
        **  after the timeout expires.  The loop is necessary since the thread
        **  may be awakened by signals before the timeout has elapsed.
        */
        while ( usec > 0 )
        {
            /*
            **  Add a cancellation point to this loop,
            **  since there are no others.
            */
            pthread_testcancel();

            usleep( usec );
            gettimeofday( &now, (struct timezone *)NULL );
            if ( timeout.tv_usec > now.tv_usec )
            {
                usec = timeout.tv_usec - now.tv_usec;
                if ( timeout.tv_sec < now.tv_sec )
                    usec = 0;
                else
                    usec += ((timeout.tv_sec - now.tv_sec) * 1000000);
            }
            else
            {
                usec = (timeout.tv_usec + 1000000) - now.tv_usec;
                if ( (timeout.tv_sec - 1) < now.tv_sec )
                    usec = 0;
                else
                    usec += (((timeout.tv_sec - 1) - now.tv_sec) * 1000000);
            }
        }
    }
    else
        /*
        **  Yield to any other task of same priority without blocking.
        */
        sched_yield();

    /*
    **  Update the task state.
    */
    tcb->state &= (!DELAY);

    return( OK );
}

/*****************************************************************************
** taskIdListGet - returns a list of active task identifiers
*****************************************************************************/
int
   taskIdListGet( int list[], int maxIds )
{
    v2pthread_cb_t *current_tcb;
    int count;

    count = 0;

    /*
    **  'Lock the v2pthread scheduler'
    */
    taskLock();

    if ( (task_list != (v2pthread_cb_t *)NULL) && (list != (int *)NULL) )
    {
        /*
        **  One or more tasks already exist in the task list...
        **  Return the identifiers for the first (maxIds) tasks in the list.
        **  Quit when (a) the end of the task list is reached or (b) the
        **  specified capacity of the caller's ID list is reached.
        */
        for ( current_tcb = task_list; 
              current_tcb != (v2pthread_cb_t *)NULL;
              current_tcb = current_tcb->nxt_task )
        {
            if ( count < maxIds )
            {
                list[count] = current_tcb->taskid;
                count++;
            }
        }
    }
 
    /*
    **  'Unlock the v2pthread scheduler'
    */
    taskUnlock();

    return( count );
}

/*****************************************************************************
** taskIdSelf - returns the identifier of the calling task
*****************************************************************************/
int
   taskIdSelf( void )
{
    v2pthread_cb_t *self_tcb;
    int my_tid;

    pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                          (void *)&task_list_lock );
    pthread_mutex_lock( &task_list_lock );

    self_tcb = my_tcb();

    pthread_cleanup_pop( 1 );

    if ( self_tcb != (v2pthread_cb_t *)NULL )
        my_tid = self_tcb->taskid;
    else
        my_tid = 0;

    return( my_tid );
}

/*****************************************************************************
** taskIdVerify - indicates whether the specified task exists or not
*****************************************************************************/
STATUS
   taskIdVerify( int taskid )
{
    v2pthread_cb_t *tcb;
    STATUS error;

    pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                          (void *)&task_list_lock );
    pthread_mutex_lock( &task_list_lock );

    if ( taskid == 0 )
        /*
        **  NULL taskid specifies current task - get TCB for current task
        */
        tcb = my_tcb();
    else
        /*
        **  Get TCB for task specified by taskid
        */
        tcb = tcb_for( taskid );

    pthread_cleanup_pop( 1 );

    if ( tcb != (v2pthread_cb_t *)NULL )
        error = OK;
    else /* NULL TCB pointer */
       error = S_objLib_OBJ_ID_ERROR;

    if ( error != OK )
    {
        errno = (int)error;
        error = ERROR;
    }
    return( error );
}

/*****************************************************************************
** taskInit - initializes the requisite data structures to support v2pthread 
**            task behavior not directly supported by Posix threads.
*****************************************************************************/
STATUS
    taskInit( v2pthread_cb_t *tcb, char *name, int pri, int opts,
              char *pstack, int stksize,
              int (*funcptr)( int,int,int,int,int,int,int,int,int,int ),
              int arg1, int arg2, int arg3, int arg4, int arg5,
              int arg6, int arg7, int arg8, int arg9, int arg10 )
{
    v2pthread_cb_t *current_tcb;
    int i, new_priority, sched_policy;
    STATUS error;

    error = OK;

    if (opts != 0 ) 
    {
		/*
		 * no options are currently implemented. If there will be some,
		 * change the condition
		 */
		errno = ENOSYS;
		return (errno);
    }

    if ( tcb != (v2pthread_cb_t *)NULL )
    {
        pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                              (void *)&task_list_lock );
        pthread_mutex_lock( &task_list_lock );

        /*
        **  Got a new task control block.  Initialize it.
        */
        tcb->pthrid = (pthread_t)NULL;

        /*
        **  Use TCB address as task identifier
        */
        tcb->taskid = (int)tcb;

        /*
        **  Copy the task name (if any)
        */
        if ( name != (char *)NULL )
        {
            i = strlen( name ) + 1;
            tcb->taskname = ts_malloc( i );
            if ( tcb->taskname != (char *)NULL )
                strncpy( tcb->taskname, name, i );
        }
        else
            tcb->taskname = (char *)NULL;

        /*
        ** Task v2pthread priority level
        */
        tcb->vxw_priority = pri;

        /*
        **  Initialize the thread attributes to default values.
        **  Then modify the attributes to make a real-time thread.
        */
        pthread_attr_init( &(tcb->attr) );

        /*
        **  Get the default scheduling priority & init prv_priority member
        */
        pthread_attr_getschedparam( &(tcb->attr), &(tcb->prv_priority) );

        /*
        **  Determine whether round-robin time-slicing is to be used or not
        */
        if ( roundRobinIsEnabled() )
            sched_policy = SCHED_RR;
        else
            sched_policy = SCHED_FIFO;
        pthread_attr_setschedpolicy( &(tcb->attr), sched_policy );

        /*
        **  Translate the v2pthread priority into a pthreads priority
        **  and set the new scheduling priority.
        */
        new_priority = translate_priority( pri, sched_policy, &error );

        (tcb->prv_priority).sched_priority = new_priority;
        pthread_attr_setschedparam( &(tcb->attr), &(tcb->prv_priority) );

        /*
        ** Entry point for task
        */
        tcb->entry_point = funcptr;

        /*
        ** Initially assume TCB statically (not dynamically) allocated
        */
        tcb->static_tcb = 1;

        /*
        ** Option flags for task
        */
        tcb->flags = opts;

        /*
        **  The task is initially 'created' with no pthread running it.
        */
        tcb->state = DEAD;

        tcb->suspend_list = (v2pthread_cb_t **)NULL;
        tcb->nxt_susp = (v2pthread_cb_t *)NULL;
        tcb->nxt_task = (v2pthread_cb_t *)NULL;

        /*
        ** Nesting level for number of taskSafe calls
        */
        tcb->delete_safe_count = 0;

        /*
        ** Mutex and Condition variable for task delete 'pend'
        */
        pthread_mutex_init( &(tcb->tdelete_lock),
                            (pthread_mutexattr_t *)NULL );
        pthread_cond_init( &(tcb->t_deletable),
                           (pthread_condattr_t *)NULL );

        /*
        ** Mutex and Condition variable for task delete 'broadcast'
        */
        pthread_mutex_init( &(tcb->dbcst_lock),
                            (pthread_mutexattr_t *)NULL );
        pthread_cond_init( &(tcb->delete_bcplt),
                           (pthread_condattr_t *)NULL );

        /*
        ** First task control block in list of tasks waiting on this task
        ** (for deletion purposes)
        */
        tcb->first_susp = (v2pthread_cb_t *)NULL;

        /*
        **  Save the caller's task arguments in the task control block
        */
        tcb->parms[0] = arg1;
        tcb->parms[1] = arg2;
        tcb->parms[2] = arg3;
        tcb->parms[3] = arg4;
        tcb->parms[4] = arg5;
        tcb->parms[5] = arg6;
        tcb->parms[6] = arg7;
        tcb->parms[7] = arg8;
        tcb->parms[8] = arg9;
        tcb->parms[9] = arg10;

        /*
        **  If everything's okay thus far, we have a valid TCB ready to go.
        */
        if ( error == OK )
        {
            /*
            **  Insert the task control block into the task list.
            **  First see if the task list contains any tasks yet.
            */
            if ( task_list == (v2pthread_cb_t *)NULL )
            {
                task_list = tcb;
            }
            else
            {
                /*
                **  Append the new tcb to the task list.
                */
                for ( current_tcb = task_list;
                      current_tcb->nxt_task != (v2pthread_cb_t *)NULL;
                      current_tcb = current_tcb->nxt_task );
                current_tcb->nxt_task = tcb;
            }
        }
        pthread_mutex_unlock( &task_list_lock );
        pthread_cleanup_pop( 0 );
    }
    else /* NULL TCB pointer */
    {
       error = S_objLib_OBJ_ID_ERROR;
    }

    if ( error != OK )
    {
        errno = (int)error;
        error = ERROR;
    }
    return( error );
}

/*****************************************************************************
** taskIsReady - indicates if the specified task is ready to run
*****************************************************************************/
BOOL
   taskIsReady( int taskid )
{
    v2pthread_cb_t *tcb;
    BOOL result;

    result = (BOOL)FALSE;

    pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                          (void *)&task_list_lock );
    pthread_mutex_lock( &task_list_lock );

    if ( taskid == 0 )
        /*
        **  NULL taskid specifies current task - get TCB for current task
        */
        tcb = my_tcb();
    else
        /*
        **  Get TCB for task specified by taskid
        */
        tcb = tcb_for( taskid );

    if ( tcb != (v2pthread_cb_t *)NULL )
        if ( (tcb->state & RDY_MSK) == READY )
            result = (BOOL)TRUE;

    pthread_cleanup_pop( 1 );

    return( result );
}

/*****************************************************************************
** taskIsSuspended - indicates if the specified task is explicitly suspended 
*****************************************************************************/
BOOL
   taskIsSuspended( int taskid )
{
    errno = ENOSYS;
    return( 0 );
}

/*****************************************************************************
** taskActivate -  creates a pthread containing the specified v2pthread task
*****************************************************************************/
STATUS
    taskActivate( int tid )
{
    v2pthread_cb_t *tcb;
    STATUS error;

    error = OK;

    /*
    **  'Lock the v2pthread scheduler' to defer any context switch to a higher
    **  priority task until after this call has completed its work.
    */
    taskLock();

    tcb = tcb_for( tid );
    if ( tcb != (v2pthread_cb_t *)NULL )
    {
        /*
        **  Found our task control block.
        **  Start a new real-time pthread for the task. 
        */
        if ( tcb->state == DEAD )
        {
            tcb->state = READY;

#ifdef DIAG_PRINTFS 
            printf( "\r\ntaskActivate task @ %p tcb @ %p:", tcb->entry_point,
                    tcb );
#endif

			/*
			 * TODO There is un ugly patch here to ensure the called thread has a valid tcb->pthrid
			 * check if that can be done better with atomic_set
			 */
			tcb->pthrid = 0;
            if ( pthread_create( &(tcb->pthrid), &(tcb->attr),
                                 task_wrapper, (void *)tcb ) != 0 )
            {
#ifdef DIAG_PRINTFS 
                perror( "\r\ntaskActivate pthread_create returned error:" );
#endif
                error = S_memLib_NOT_ENOUGH_MEMORY;
                tcb_delete( tcb );
            }
        }
        else
        {
            /*
            ** task already made runnable
            */
#ifdef DIAG_PRINTFS 
            printf( "\r\ntaskActivate task @ tcb %p already active", tcb );
#endif
        }
    }
    else
        error = S_objLib_OBJ_ID_ERROR;

    /*
    **  'Unlock the v2pthread scheduler' to enable a possible context switch
    **  to a task made runnable by this call.
    */
    taskUnlock();

#ifdef DIAG_PRINTFS
	printf("taskActivate: ADDED task=%d (%s) tcb=0x%X, vxw_Prio=%d, Linux-Prio=%d\n", 
		(int)tcb->pthrid, tcb->taskname ? tcb->taskname : "no-name", 
		(int)tcb, tcb->vxw_priority, (tcb->prv_priority).sched_priority);
#endif

    if ( error != OK )
    {
        errno = (int)error;
        error = ERROR;
    }
    return( error );
}

/*****************************************************************************
** taskSpawn -   initializes the requisite data structures to support v2pthread 
**               task behavior not directly supported by Posix threads and
**               creates a pthread to contain the specified v2pthread task.
*****************************************************************************/
int
    taskSpawn( char *name, int pri, int opts, int stksize,
               int (*funcptr)( int,int,int,int,int,int,int,int,int,int ),
               int arg1, int arg2, int arg3, int arg4, int arg5,
               int arg6, int arg7, int arg8, int arg9, int arg10 )
{
    v2pthread_cb_t *tcb;
    int my_tid;
    STATUS error;
    char myname[16];


    /* First allocate memory for a new pthread task control block */
    tcb = ts_malloc( sizeof( v2pthread_cb_t ) );
    if ( tcb != (v2pthread_cb_t *)NULL )
    {
        /*
        ** Synthesize a default task name if none specified
        */
        my_tid = new_tid();
        if ( name == (char *)NULL )
        {
            sprintf( myname, "t%d", my_tid );
            name = &(myname[0]);
        }

        /*
        **  Initialize the task control block and pthread attribute structure.
        */
        error = taskInit( tcb, name, pri, opts, (char *)NULL, stksize, funcptr,
                          arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9,
                          arg10 );
        if ( error == OK )
        {

            /*
            **  Establish 'normal' task identifier, overwriting taskid set by
            **  taskInit call.
            */
            pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                                  (void *)&task_list_lock );
            pthread_mutex_lock( &task_list_lock );
            tcb->taskid = my_tid;

            /*
            ** Indicate TCB dynamically allocated
            */
            tcb->static_tcb = 0;

            pthread_mutex_unlock( &task_list_lock );
            pthread_cleanup_pop( 0 );

            /*
            **  Activate (begin execution of) the task just initialized.
            */
            error = taskActivate( my_tid );
        }
        else
        {
            /*
            **  OOPS! Something went wrong... clean up & exit.
            */
            ts_free( (void *)tcb );
        }
    }
    else /* malloc failed */
    {
        error = S_smObjLib_NOT_INITIALIZED;
        my_tid = (int)error;
    }

    if ( error != OK )
    {
        my_tid = errno = (int)error;
    }
    return( my_tid );
}


/*****************************************************************************
** taskSuspend - suspends the specified v2pthread task
*****************************************************************************/
STATUS
    taskSuspend( int tid )
{
    errno = ENOSYS;
    return( errno );
}

/*****************************************************************************
** taskResume - creates a pthread to contain the specified v2pthread task and
*****************************************************************************/
STATUS
    taskResume( int tid )
{
    errno = ENOSYS;
    return( errno );
}

/*****************************************************************************
** taskPriorityGet - examines the current priority for the specified task
*****************************************************************************/
STATUS
    taskPriorityGet( int tid, int *priority )
{
    v2pthread_cb_t *tcb;
    STATUS error;

    error = OK;

    taskLock();

    tcb = tcb_for( tid );
    if ( tcb != (v2pthread_cb_t *)NULL )
    {
        if ( priority != (int *)NULL )
            *priority = tcb->vxw_priority;
    } 
    else
        error = S_objLib_OBJ_ID_ERROR;

    taskUnlock();

    if ( error != OK )
    {
        errno = (int)error;
        error = ERROR;
    }
    return( error );
}


/*****************************************************************************
** taskPrioritySet - sets a new priority for the specified task
*****************************************************************************/
STATUS
    taskPrioritySet( int tid, int pri )
{
    v2pthread_cb_t *tcb;
    int new_priority, sched_policy;
    STATUS error;

    error = OK;

    taskLock();

    if ( tid == 0 )
        /*
        **  NULL tid specifies current task - get TCB for current task
        */
        tcb = my_tcb();
    else
        /*
        **  Get TCB for task specified by tid
        */
        tcb = tcb_for( tid );

    if ( tcb != (v2pthread_cb_t *)NULL )
    {
        /*
        **  Translate the v2pthread priority into a pthreads priority
        */
        pthread_attr_getschedpolicy( &(tcb->attr), &sched_policy );
        new_priority = translate_priority( pri, sched_policy, &error );

        /*
        **  Update the TCB with the new priority
        */
        tcb->vxw_priority = pri;
        (tcb->prv_priority).sched_priority = new_priority;

        /*
        **  If the selected task is not the currently-executing task,
        **  modify the pthread's priority now.  If the selected task
        **  IS the currently-executing task, the taskUnlock operation
        **  will restore this task to the new priority level.
        */
        if ( (tid != 0) && (tcb != my_tcb()) )
        {
	    struct sched_param schedparam;
            pthread_attr_setschedparam( &(tcb->attr), &(tcb->prv_priority) );
	    pthread_attr_getschedparam( &(tcb->attr), &schedparam );
            schedparam.sched_priority = new_priority;
	    pthread_attr_setschedparam( &(tcb->attr), &schedparam );
            pthread_setschedparam( tcb->pthrid, sched_policy, &schedparam );
        }
    }
    else
        error = S_objLib_OBJ_ID_ERROR;

    taskUnlock();

    if ( error != OK )
    {
        errno = (int)error;
        error = ERROR;
    }
    return( error );
}

/*****************************************************************************
** taskName - returns the name of the specified v2pthread task
*****************************************************************************/
char *
    taskName( int tid )
{
    v2pthread_cb_t *current_tcb;
    char *taskname;
	static char NullTaskName[] = "NULLTASK";
    pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                          (void *)&task_list_lock );
    pthread_mutex_lock( &task_list_lock );

    if ( tid == 0 )
        /*
        **  NULL tid specifies current task - get TCB for current task
        */
        current_tcb = my_tcb();
    else
        /*
        **  Get TCB for task specified by tid
        */
        current_tcb = tcb_for( tid );

    if ( current_tcb != (v2pthread_cb_t *)NULL )
        taskname = current_tcb->taskname;
    else
        taskname = NullTaskName;

    pthread_cleanup_pop( 1 );

    return( taskname );
}

/*****************************************************************************
** taskNametoId - identifies the named v2pthread task
*****************************************************************************/
int
    taskNameToId( char *name )
{
    v2pthread_cb_t *current_tcb;
    int tid;

    tid = (int)ERROR;

    if ( name != (char *)NULL )
    {
        /*
        **  Scan the task list for a name matching the caller's name.
        */
        pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                              (void *)&task_list_lock );
        pthread_mutex_lock( &task_list_lock );

        for ( current_tcb = task_list;
              current_tcb != (v2pthread_cb_t *)NULL;
              current_tcb = current_tcb->nxt_task )
        {
            if ( (strcmp( name, current_tcb->taskname )) == 0 )
            {
                /*
                **  A matching name was found... return its TID
                */
                tid = current_tcb->taskid;
                break;
            }
        }

        pthread_cleanup_pop( 1 );
    }

    return( tid );
}

/*****************************************************************************
** taskDelete - removes the specified task(s) from the task list,
**              frees the memory occupied by the task control block(s),
**              and kills the pthread(s) associated with the task(s).
*****************************************************************************/
STATUS
   taskDelete( int tid )
{
    v2pthread_cb_t *current_tcb;
    v2pthread_cb_t *self_tcb;
    int task_deletable;
    STATUS error;

    error = OK;

    taskLock();

    /*
    **  Get pointer to TCB for specified task
    */
    self_tcb = my_tcb();
    if ( tid == 0 )
        current_tcb = self_tcb;
    else
        current_tcb = tcb_for( tid );

    /*
    **  Verify that specified task still exists in task list.
    */
    if ( current_tcb != (v2pthread_cb_t *)NULL )
    {
#ifdef DIAG_PRINTFS 
        printf( "\r\ntaskDelete - lock delete cond var mutex @ tcb %p",
                current_tcb );
#endif
        /*
        **  Lock mutex for task delete_safe_count & condition variable
        */
        pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                              (void *)&( current_tcb->tdelete_lock));
        pthread_mutex_lock( &( current_tcb->tdelete_lock) );

        if ( current_tcb->delete_safe_count > 0 )
            task_deletable = FALSE;
        else
            task_deletable = TRUE;

        /*
        **  Unlock the mutex for the condition variable and clean up.
        */
#ifdef DIAG_PRINTFS 
        printf( "\r\ntaskDelete - unlock delete cond var mutex @ tcb %p",
                current_tcb );
#endif
        pthread_cleanup_pop( 1 );

        if ( task_deletable == FALSE )
        {
            if ( current_tcb == self_tcb )
            {
                /*
                **  Task being deleted is currently executing task, and is
                **  delete-protected at this time...
                **  Task cannot block or delete itself while delete-protected.
                */
#ifdef DIAG_PRINTFS 
                printf( "\r\ntaskDelete - can't self-delete prot task @ tcb %p",
                        current_tcb );
#endif
                error = S_objLib_OBJ_UNAVAILABLE;
            }
            else
            {
                /*
                **  Specified task is a different task, but not deletable.
                **  Our task must pend until the specified task becomes
                **  deletable.  Add our task to the list of tasks waiting to
                **  delete the specified task.
                */
#ifdef DIAG_PRINTFS 
                printf( "\r\ntask @ %p wait on task-delete list @ %p", self_tcb,
                        &(current_tcb->first_susp) );
#endif
                link_susp_tcb( &(current_tcb->first_susp), self_tcb );

                /*
                **  Lock mutex for task delete_safe_count & condition variable
                */
#ifdef DIAG_PRINTFS 
                printf( "\r\ntaskDelete - lock delete cond var mutex @ tcb %p",
                        current_tcb );
#endif
                pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                                      (void *)&( current_tcb->tdelete_lock));
                pthread_mutex_lock( &( current_tcb->tdelete_lock) );

                /*
                **  Unlock scheduler to allow other tasks to make specified
                **  task deletable.
                */
                taskUnlock();

                /*
                **  Wait without timeout for task to become deletable.
                */
#ifdef DIAG_PRINTFS 
                printf( "\r\ntaskDelete - wait till task @ tcb %p deletable",
                        current_tcb );
#endif
                while ( current_tcb->delete_safe_count > 0 )
                {
                    pthread_cond_wait( &(current_tcb->t_deletable),
                                       &(current_tcb->tdelete_lock) );
                }

#ifdef DIAG_PRINTFS 
                printf( "\r\ntaskDelete - task @ tcb %p now deletable",
                        current_tcb );
#endif
                taskLock();

                /*
                **  Remove the calling task's tcb from the pended task list
                **  for the task being deleted.  Clear the calling task 's
                **  suspend list pointer since the TCB it was suspended on is
                **  being deleted and deallocated.
                */
                unlink_susp_tcb( &(current_tcb->first_susp), self_tcb );
                self_tcb->suspend_list = (v2pthread_cb_t **)NULL;

                /*
                **  If our task was the last one pended, signal the task
                **  which enabled the deletion and indicate that all pended
                **  tasks have been awakened.
                */
                if ( current_tcb->first_susp == (v2pthread_cb_t *)NULL )
                {
                    /*
                    ** Lock mutex for task delete broadcast completion
                    */
#ifdef DIAG_PRINTFS 
                    printf( "\r\ntaskDelete - lock del bcast mutex @ tcb %p",
                        current_tcb );
#endif
                    pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                                          (void *)&(current_tcb->dbcst_lock) );
                    pthread_mutex_lock( &(current_tcb->dbcst_lock) );

                    /*
                    **  Signal task delete broadcast completion. 
                    */
#ifdef DIAG_PRINTFS 
                    printf( "\r\ntaskDelete - bcast delete complt @ tcb %p",
                        current_tcb );
#endif
                    pthread_cond_broadcast( &(current_tcb->delete_bcplt) );

                    /*
                    **  Unlock the task delete broadcast completion mutex. 
                    */
#ifdef DIAG_PRINTFS 
                    printf( "\r\ntaskDelete - unlock del bcast mutex @ tcb %p",
                        current_tcb );
#endif
                    pthread_cleanup_pop( 1 );
                    task_deletable = TRUE;
                }

                /*
                **  Unlock the mutex for the condition variable and clean up.
                */
#ifdef DIAG_PRINTFS 
                printf( "\r\ntaskDelete - unlock delete cond var mutex @ tcb %p",
                        current_tcb );
#endif
                pthread_cleanup_pop( 1 );
            }
        }

        if ( task_deletable == TRUE )
            /*
            **  At this point the specified task has been marked as deletable.
            **  If the current task is one of several attempting to delete the
            **  specified task, then only the first of the deleting tasks will
            **  actually succeed in deleting the target task.
            **  Kill the task pthread and deallocate its data structures.
            */
            error = taskDeleteForce( tid );
    }
    else
        error = S_objLib_OBJ_DELETED;

    taskUnlock();

    if ( error != OK )
    {
        errno = (int)error;
        error = ERROR;
    }

    return( error );
}

/*****************************************************************************
** taskRestart - terminates the specified task and starts it over from its
**               entry point.
*****************************************************************************/
STATUS
    taskRestart( int tid )
{
    v2pthread_cb_t *current_tcb;
    v2pthread_cb_t *self_tcb;
    STATUS error;

    error = OK;

    taskLock();

    /*
    **  Restart the task whose taskid matches tid.
    **  If the task_list contains tasks, scan it for the tcb
    **  whose task id matches the one to be restarted.
    */
    self_tcb = my_tcb();
    if ( tid == 0 )
        /*
        **  NULL tid specifies current task - get TCB for current task
        */
        current_tcb = self_tcb;
    else
        /*
        **  Get TCB for task specified by tid
        */
        current_tcb = tcb_for( tid );

    /*
    **  Make sure valid TCB found in scan of task list
    */
    if ( current_tcb != (v2pthread_cb_t *)NULL )
    {
        /*
        **  Found the task being restarted...
        */
#ifdef DIAG_PRINTFS 
        printf( "\r\ntaskRestart - restart pthread for task @ tcb %p",
                current_tcb );
        fflush( stdout );
#endif
        /*
        **  Remove the task from the suspend list for any object it
        **  is pending on.
        */
#ifdef DIAG_PRINTFS 
        printf( "\r\ntaskRestart - tcb @ %p suspend_list = %p", current_tcb,
                current_tcb->suspend_list );
        fflush( stdout );
#endif
        unlink_susp_tcb( current_tcb->suspend_list, current_tcb );

        if ( current_tcb != self_tcb )
        {
            /*
            **  Task being restarted is not the current task.
            **  Kill the task pthread and wait for it to die.
            */
#ifdef DIAG_PRINTFS 
            printf( "\r\ntaskRestart - other tcb @ %p", current_tcb );
            fflush( stdout );
#endif
            pthread_cancel( current_tcb->pthrid );
            pthread_join( current_tcb->pthrid, (void **)NULL );

            /*
            **  Start a new pthread using the existing task control block.
            */
            current_tcb->pthrid = (pthread_t)NULL;
            current_tcb->state = READY;
            if ( pthread_create( &(current_tcb->pthrid), &(current_tcb->attr),
                                 task_wrapper, (void *)current_tcb ) != 0 )
            {
#ifdef DIAG_PRINTFS 
                perror( "\r\ntaskRestart pthread_create returned error:" );
#endif
                error = S_memLib_NOT_ENOUGH_MEMORY;
            }
        }
        else
        {
            /*
            **  Restart the currently executing task.
            */
#ifdef DIAG_PRINTFS 
            printf( "\r\ntaskRestart - self tcb @ %p", current_tcb );
            fflush( stdout );
#endif

            /*
            **  Detach our task's pthread to clean up memory, etc. without
            **  a 'join' operation.
            */
            pthread_detach( self_tcb->pthrid );

            /*
            **  Start a watchdog timer to restart our task after we terminate.
            */
            selfRestart( self_tcb );

            /*
            **  Kill the currently executing task's pthread.
            */
            pthread_exit( (void *)NULL );
        }
    }
    else
        /*
        **  No TCB found in task list for specified tid... must have already
        **  been deleted.
        */
        error = S_objLib_OBJ_ID_ERROR;

    taskUnlock();

    if ( error != OK )
    {
        errno = (int)error;
        error = ERROR;
    }
    return( error );
}

/*****************************************************************************
** taskSafe - marks the calling task as safe from explicit deletion.
*****************************************************************************/
STATUS
   taskSafe( void )
{
    v2pthread_cb_t *current_tcb;

    taskLock();

    /*
    **  Get pointer to TCB for current task
    */
    current_tcb = my_tcb();

    /*
    **  Lock mutex for task delete_safe_count & condition variable
    */
    pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                          (void *)&(current_tcb->tdelete_lock));
    pthread_mutex_lock( &(current_tcb->tdelete_lock) );
#ifdef DIAG_PRINTFS 
    printf( "\r\ntaskSafe - lock delete cond var mutex @ tcb %p", current_tcb );
#endif

    /*
    **  Increment task delete_safe_count and adjust for any overflow.
    */
    current_tcb->delete_safe_count++;
    if ( current_tcb->delete_safe_count <= 0 )
        current_tcb->delete_safe_count--;
#ifdef DIAG_PRINTFS 
    printf( "\r\ntaskSafe - new delete_safe_count %d @ tcb %p",
            current_tcb->delete_safe_count, current_tcb );
#endif

    /*
    **  Unlock the mutex for the condition variable and clean up.
    */
#ifdef DIAG_PRINTFS 
    printf( "\r\ntaskSafe - unlock delete cond var mutex @ tcb %p",
            current_tcb );
#endif
    pthread_cleanup_pop( 1 );

    taskUnlock();

    return( (STATUS)OK );
}

/*****************************************************************************
** taskUnsafe - marks the calling task as subject to explicit deletion.
*****************************************************************************/
STATUS
   taskUnsafe( void )
{
    v2pthread_cb_t *current_tcb;
    int task_made_deletable;

    taskLock();

    /*
    **  Get pointer to TCB for current task
    */
    current_tcb = my_tcb();

    task_made_deletable = FALSE;

    /*
    **  Lock mutex for task delete_safe_count & condition variable
    */
#ifdef DIAG_PRINTFS 
    printf( "\r\ntaskUnsafe - lock delete cond var mutex @ tcb %p",
            current_tcb );
#endif
    pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                          (void *)&(current_tcb->tdelete_lock));
    pthread_mutex_lock( &(current_tcb->tdelete_lock) );

    /*
    **  Decrement task delete_safe_count
    */
    if ( current_tcb->delete_safe_count > 0 )
    {
        current_tcb->delete_safe_count--;
        if ( current_tcb->delete_safe_count == 0 )
            task_made_deletable = TRUE;
    }
#ifdef DIAG_PRINTFS 
    printf( "\r\ntaskUnsafe - new delete_safe_count %d @ tcb %p",
            current_tcb->delete_safe_count, current_tcb );
#endif

    /*
    **  Unlock the mutex for the condition variable and clean up.
    */
#ifdef DIAG_PRINTFS 
    printf( "\r\ntaskUnsafe - unlock delete cond var mutex @ tcb %p",
            current_tcb );
#endif
    pthread_cleanup_pop( 1 );

    taskUnlock();

    if ( task_made_deletable )
    {
        /*
        **  Task just made deletable... ensure that we awaken any
        **  other tasks pended on deletion of this task
        */
        if ( current_tcb->first_susp != (v2pthread_cb_t *)NULL )
        {
            notify_task_delete( current_tcb );
        }
    }

    return( (STATUS)OK );
}

/*****************************************************************************
** taskTcb - returns the address of the task control block for the task
**           idenified by taskid
*****************************************************************************/
void *
   taskTcb( int taskid )
{
    v2pthread_cb_t *tcb;

    pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                          (void *)&task_list_lock );
    pthread_mutex_lock( &task_list_lock );

    if ( taskid == 0 )
        /*
        **  NULL taskid specifies current task - get TCB for current task
        */
        tcb = my_tcb();
    else
        /*
        **  Get TCB for task specified by taskid
        */
        tcb = tcb_for( taskid );

    pthread_cleanup_pop( 1 );

    return( (void *)tcb );
}

