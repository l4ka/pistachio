/*****************************************************************************
 * wdLib.c - defines the wrapper functions and data structures needed
 *           to implement a Wind River VxWorks (R) watchdog timer API 
 *           in a POSIX Threads environment.
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <semaphore.h>
#include <sys/time.h>
#include "v2pthread.h"
#include "vxw_defs.h"

/*****************************************************************************
**  Control block for v2pthread watchdog timer
**
**  These watchdog timers provide a means of executing delayed or cyclic
**  functions.  They are inherently 'one-shot' timers.  For cyclic operation,
**  the timeout handler function must call wdStart to restart the timer.
**  In the v2pthreads environment, these timers execute from the
**  context of the system exception task rather tha the timer interrupt.
*****************************************************************************/
typedef struct v2pt_wdog
{
        /*
        ** Mutex for watchdog access and modification
        */
    pthread_mutex_t
        wdog_lock;

        /*
        ** Ticks remaining until timeout (zero if watchdog already expired).
        */
    int ticks_remaining;

        /*
        ** Function to be executed when ticks remaining decrements to zero.
        */
    void (*timeout_func)( int );

        /*
        ** Parameter to pass to timeout handler function.
        */
    int timeout_parm;

        /*
        ** Pointer to next watchdog control block in watchdog list.
        */
    struct v2pt_wdog *
        nxt_wdog;
} v2pt_wdog_t;

/*****************************************************************************
**  External function and data references
*****************************************************************************/
extern void *
   ts_malloc( size_t blksize );
extern void
   ts_free( void *blkaddr );
extern void
   taskLock( void );
extern void
   taskUnlock( void );
extern v2pthread_cb_t *
   tcb_for( int taskid );
void *
    task_wrapper( void *arg );

/*****************************************************************************
**  v2pthread Global Data Structures
*****************************************************************************/

/*
**  wdog_list is a linked list of watchdog control blocks.  It is used to locate
**             watchdogs by their ID numbers.
*/
static v2pt_wdog_t *
    wdog_list;

/*
**  wdog_list_lock is a mutex used to serialize access to the watchdog list
*/
static pthread_mutex_t
    wdog_list_lock = PTHREAD_MUTEX_INITIALIZER;


/*****************************************************************************
**  wdog_valid - verifies whether the specified watchdog still exists, and if
**                so, locks exclusive access to the watchdog for the caller.
*****************************************************************************/
static int
   wdog_valid( v2pt_wdog_t *wdId )
{
    
    v2pt_wdog_t *current_wdog;
    int found_wdog;

    found_wdog = FALSE;

    /*
    **  Protect the watchdog list while we examine and modify it.
    */
    pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                          (void *)&wdog_list_lock );
    pthread_mutex_lock( &wdog_list_lock );

    if ( wdog_list != (v2pt_wdog_t *)NULL )
    {
        /*
        **  One or more watchdogs already exist in the watchdog list...
        **  Scan the existing watchdogs for a matching ID.
        */
        for ( current_wdog = wdog_list; 
              current_wdog != (v2pt_wdog_t *)NULL;
              current_wdog = current_wdog->nxt_wdog )
        {
#ifdef DIAG_PRINTFS 
            printf( "\r\nlooking for watchdog @ %p - found watchdog @ %p", wdId,
                    current_wdog );
#endif
            if ( current_wdog == wdId )
            {
                /*
                ** Lock mutex for watchdog access (it is assumed that a
                ** 'pthread_cleanup_push()' has already been performed
                **  by the caller in case of unexpected thread termination.)
                */
                pthread_mutex_lock( &(wdId->wdog_lock) );

                found_wdog = TRUE;
                break;
            }
        }
    }
 
    /*
    **  Re-enable access to the watchdog list by other threads.
    */
    pthread_cleanup_pop( 1 );
 
    return( found_wdog );
}

/*****************************************************************************
** link_wdog - appends a new watchdog control block pointer to the wdog_list
*****************************************************************************/
static void
   link_wdog( v2pt_wdog_t *new_wdog )
{
    v2pt_wdog_t *current_wdog;

    /*
    **  Protect the watchdog list while we examine and modify it.
    */
    pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                          (void *)&wdog_list_lock );
    pthread_mutex_lock( &wdog_list_lock );

    new_wdog->nxt_wdog = (v2pt_wdog_t *)NULL;
    if ( wdog_list != (v2pt_wdog_t *)NULL )
    {
        /*
        **  One or more watchdogs already exist in the watchdog list...
        **  Insert the new entry at the tail of the list.
        */
        for ( current_wdog = wdog_list; 
              current_wdog->nxt_wdog != (v2pt_wdog_t *)NULL;
              current_wdog = current_wdog->nxt_wdog );
        current_wdog->nxt_wdog = new_wdog;
#ifdef DIAG_PRINTFS 
        printf( "\r\nadd watchdog cb @ %p to list @ %p", new_wdog, current_wdog );
#endif
    }
    else
    {
        /*
        **  this is the first watchdog being added to the watchdog list.
        */
        wdog_list = new_wdog;
#ifdef DIAG_PRINTFS 
        printf( "\r\nadd watchdog cb @ %p to list @ %p", new_wdog, &wdog_list );
#endif
    }
 
    /*
    **  Re-enable access to the watchdog list by other threads.
    */
    pthread_mutex_unlock( &wdog_list_lock );
    pthread_cleanup_pop( 0 );
}

/*****************************************************************************
** unlink_wdog - removes a watchdog control block pointer from the wdog_list
*****************************************************************************/
static v2pt_wdog_t *
   unlink_wdog( v2pt_wdog_t *wdId )
{
    v2pt_wdog_t *current_wdog;
    v2pt_wdog_t *selected_wdog;

    selected_wdog =  (v2pt_wdog_t *)NULL;

    if ( wdog_list != (v2pt_wdog_t *)NULL )
    {
        /*
        **  One or more watchdogs exist in the watchdog list...
        **  Protect the watchdog list while we examine and modify it.
        */
        pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                              (void *)&wdog_list_lock );
        pthread_mutex_lock( &wdog_list_lock );

        /*
        **  Scan the watchdog list for a wdog with a matching watchdog ID
        */
        if ( wdog_list == wdId )
        {
            /*
            **  The first watchdog in the list matches the selected watchdog ID
            */
            selected_wdog = wdog_list; 
            wdog_list = selected_wdog->nxt_wdog;
#ifdef DIAG_PRINTFS 
            printf( "\r\ndel watchdog cb @ %p from list @ %p", selected_wdog,
                    &wdog_list );
#endif
        }
        else
        {
            /*
            **  Scan the next wdog for a matching wdId while retaining a
            **  pointer to the current wdog.  If the next wdog matches,
            **  select it and then unlink it from the watchdog list.
            */
            for ( current_wdog = wdog_list; 
                  current_wdog->nxt_wdog != (v2pt_wdog_t *)NULL;
                  current_wdog = current_wdog->nxt_wdog )
            {
                if ( current_wdog->nxt_wdog == wdId )
                {
                    /*
                    **  Queue ID of next wdog matches...
                    **  Select the wdog and then unlink it by linking
                    **  the selected wdog's next wdog into the current wdog.
                    */
                    selected_wdog = current_wdog->nxt_wdog;
                    current_wdog->nxt_wdog = selected_wdog->nxt_wdog;
#ifdef DIAG_PRINTFS 
                    printf( "\r\ndel watchdog cb @ %p from list @ %p",
                            selected_wdog, current_wdog );
#endif
                    break;
                }
            }
        }

        /*
        **  Re-enable access to the watchdog list by other threads.
        */
        pthread_mutex_unlock( &wdog_list_lock );
        pthread_cleanup_pop( 0 );
    }

    return( selected_wdog );
}

/*****************************************************************************
** process_tick_for - performs service on the specified watchdog timer after
**                    a system timer tick has elapsed.  Returns the address
**                    of the next watchdog timer in the timer list, or NULL
**                    if no further timers remain to  be processed
*****************************************************************************/
static v2pt_wdog_t *
   process_tick_for( v2pt_wdog_t *wdId )
{
    v2pt_wdog_t *nxt_wdog;

    /*
    **  First ensure that the specified watchdog exists and that we have
    **  exclusive access to it.
    */
    pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                          (void *)&(wdId->wdog_lock));
    if ( wdog_valid( wdId ) )
    {
        /*
        **  Mark the next watchdog timer (if any) in the timer list.
        */
        nxt_wdog = wdId->nxt_wdog;
#ifdef DIAG_PRINTFS 
        printf( "\r\ndecrement watchdog @ %p next watchdog @ %p", wdId,
                nxt_wdog );
#endif

        /*
        **  The timer is inactive if no ticks remaining
        */
#ifdef DIAG_PRINTFS 
        printf( "\r\nwatchdog @ %p has %d ticks remaining", wdId,
                wdId->ticks_remaining );
#endif
        if ( wdId->ticks_remaining > 0 )
        {
            /*
            **  Decrement ticks_remaining, and if this was the last tick,
            **  check for a timeout handler function.
            */
            wdId->ticks_remaining--; 
            if ( (wdId->ticks_remaining == 0) &&
                 (wdId->timeout_func != (void (*)( int ))NULL) )
            {
                /*
                **  Unlock the queue mutex so the timeout handler can call
                **  wdStart if desired.  This would create a race condition,
                **  but (1) the task executing this function cannot be
                **  preempted at this point by other v2pthread tasks, and
                **  (2) the timer list itself is locked at this point. 
                */
                pthread_mutex_unlock( &(wdId->wdog_lock) );

#ifdef DIAG_PRINTFS 
                printf( "\r\nwatchdog @ %p calling function @ %p", wdId,
                        wdId->timeout_func );
#endif
                /*
                **  Execute the timeout handler, passing it the
                **  timer handler parameter.
                */
                (*(wdId->timeout_func))( wdId->timeout_parm );
            }
            else
                /*
                **  Unlock the queue mutex. 
                */
                pthread_mutex_unlock( &(wdId->wdog_lock) );
        }
        else
        {
            /*
            **  Unlock the queue mutex. 
            */
            pthread_mutex_unlock( &(wdId->wdog_lock) );
        }
    }
    else
    {
        /*
        **  Invalid watchdog timer specified - return NULL
        */
        nxt_wdog = (v2pt_wdog_t *)NULL;
#ifdef DIAG_PRINTFS 
        printf( "\r\nwatchdog @ %p invalid... next watchdog @ %p", wdId,
                nxt_wdog );
#endif
    }

    /*
    **  Clean up the opening pthread_cleanup_push()
    */
    pthread_cleanup_pop( 0 );

    return( nxt_wdog );
}

/*****************************************************************************
** process_timer_list - performs service on all active watchdog timers after
**                      a system timer tick has elapsed.
*****************************************************************************/
void
   process_timer_list( void )
{
    v2pt_wdog_t *nxt_wdog;

    if ( wdog_list != (v2pt_wdog_t *)NULL )
    {
        /*
        **  One or more watchdogs exist in the watchdog list...
        **  Now traverse the watchdog timer list, servicing all active timers.
        */
        for ( nxt_wdog = wdog_list; nxt_wdog != (v2pt_wdog_t *)NULL; )
        {
#ifdef DIAG_PRINTFS 
            printf( "\r\nprocess next watchdog @ %p", nxt_wdog );
#endif
            nxt_wdog = process_tick_for( nxt_wdog );
        }
    }
}

/*****************************************************************************
** wdCancel - De-activates an existing watchdog timer, but does not remove it
**            from the timer list.  The same watchdog may be re-started or
**            re-used for other timer purposes.
*****************************************************************************/
STATUS
   wdCancel( v2pt_wdog_t *wdId )
{
    STATUS error;

    error = OK;

    /*
    **  First ensure that the specified watchdog exists and that we have
    **  exclusive access to it.
    */
    pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                          (void *)&(wdId->wdog_lock));
    if ( wdog_valid( wdId ) )
    {

        /*
        **  Zero the ticks remaining without executing the timeout function.
        */
        wdId->ticks_remaining = 0; 

        /*
        **  Unlock the queue mutex. 
        */
        pthread_mutex_unlock( &(wdId->wdog_lock) );
    }
    else
    {
        error = S_objLib_OBJ_ID_ERROR;
    }

    /*
    **  Clean up the opening pthread_cleanup_push()
    */
    pthread_cleanup_pop( 0 );

    if ( error != OK )
    {
        errno = (int)error;
        error = ERROR;
    }

    return( error );
}

/*****************************************************************************
** wdCreate - Creates a new (inactive) watchdog timer.
*****************************************************************************/
v2pt_wdog_t *
   wdCreate( void )
{
    v2pt_wdog_t *new_wdog;
        
     /*
     **  Allocate memory for the watchdog timer control block.
     */
    new_wdog = (v2pt_wdog_t *)ts_malloc( sizeof( v2pt_wdog_t ) );
    if ( new_wdog != (v2pt_wdog_t *)NULL )
    {
         /*
         **  Init mutex for watchdog access and modification.
         */
         pthread_mutex_init( &(new_wdog->wdog_lock),
                             (pthread_mutexattr_t *)NULL );

         /*
         **  Zero ticks remaining until timeout so watchdog is inactive.
         */
         new_wdog->ticks_remaining = 0;

         /*
         **  Link the new watchdog control block into the watchdog timer list.
         */
         link_wdog( new_wdog );
    }

    return( new_wdog );
}

/*****************************************************************************
** wdDelete - Removes te specified watchdog timer from the watchdog list
**            and deallocates the memory used by the watchdog.
*****************************************************************************/
STATUS
   wdDelete( v2pt_wdog_t *wdId )
{
    STATUS error;

    error = OK;

    /*
    **  First ensure that the specified watchdog exists and that we have
    **  exclusive access to it.
    */
    pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                          (void *)&(wdId->wdog_lock));
    if ( wdog_valid( wdId ) )
    {
        /*
        **  First remove the watchdog from the watchdog list
        */
        unlink_wdog( wdId );

        /*
        **  Finally delete the watchdog control block itself;
        */
        ts_free( (void *)wdId );

        /*
        **  Unlock the queue mutex. 
        */
        pthread_mutex_unlock( &(wdId->wdog_lock) );
    }
    else
    {
        error = S_objLib_OBJ_ID_ERROR;
    }

    /*
    **  Clean up the opening pthread_cleanup_push()
    */
    pthread_cleanup_pop( 0 );

    if ( error != OK )
    {
        errno = (int)error;
        error = ERROR;
    }

    return( error );
}

/*****************************************************************************
** wdStart - Activates or reactivates an existing watchdog timer.
*****************************************************************************/
STATUS
   wdStart( v2pt_wdog_t *wdId, int delay, void (*funcptr)( int ), int parm )
{
    STATUS error;

    error = OK;

    /*
    **  First ensure that the specified watchdog exists and that we have
    **  exclusive access to it.
    */
    pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                          (void *)&(wdId->wdog_lock));
    if ( wdog_valid( wdId ) )
    {
        /*
        ** Ticks remaining until timeout (zero if watchdog already expired).
        ** (On an x86 Linux system, the minimum taskDelay appears to be
        ** 20 msec - not 10 msec.
        */
        wdId->ticks_remaining = delay / 2;
        if ( wdId->ticks_remaining < 1 )
            wdId->ticks_remaining = 1;

        /*
        ** Function to be executed when ticks remaining decrements to zero.
        */
        wdId->timeout_func = funcptr;

        /*
        ** Parameter to pass to timeout handler function.
        */
        wdId->timeout_parm = parm;

        /*
        **  Unlock the queue mutex. 
        */
        pthread_mutex_unlock( &(wdId->wdog_lock) );
    }
    else
    {
        error = S_objLib_OBJ_ID_ERROR;
    }

    /*
    **  Clean up the opening pthread_cleanup_push()
    */
    pthread_cleanup_pop( 0 );

    if ( error != OK )
    {
        errno = (int)error;
        error = ERROR;
    }

    return( error );
}

/*****************************************************************************
** self_starter - called from a task-restart watchdog timer.  Starts a new
**                pthread for the specified task and deletes the watchdog.
*****************************************************************************/
static v2pt_wdog_t *restart_wdog;

static void
   self_starter( int taskid )
{
    v2pthread_cb_t *tcb;

    /*
    **  Delete the task restart watchdog regardless of the success or failure
    **  of the restart operation.
    */
    wdDelete( restart_wdog );

    /*
    **  Get the address of the task control block for the specified task ID.
    */
    tcb = tcb_for( taskid );
    if ( tcb != (v2pthread_cb_t *)NULL )
    {
        /*
        **  Mark the task as ready to run and create a new pthread using the
        **  attributes and entry point defined in the task control block.
        */
        tcb->state = READY;
        tcb->pthrid = (pthread_t)NULL;
        pthread_create( &(tcb->pthrid), &(tcb->attr), task_wrapper,
                        (void *)tcb );
    }
}

/*****************************************************************************
** selfRestart - restarts the specified (existing) v2pthread task after said
**               task has terminated itself.
*****************************************************************************/
void
   selfRestart( v2pthread_cb_t *restart_tcb )
{
    /*
    **  First create a watchdog timer to handle the restart operation.
    */
    restart_wdog = wdCreate();

    if ( restart_wdog != (v2pt_wdog_t *)NULL )
    {
        /*
        **  Start the watchdog timer to expire after one tick and to call
        **  a function which will first restart the specified task and
        **  then delete the restart watchdog timer.
        */
        wdStart( restart_wdog, 1, self_starter, restart_tcb->taskid );
    }
}

