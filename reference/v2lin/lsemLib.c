/*****************************************************************************
 * semLib.c - defines the wrapper functions and data structures needed
 *            to implement a Wind River VxWorks (R) semaphore API 
 *            in a POSIX Threads environment.
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

#define SEM_OPT_MASK       0x0f
#define SEM_TYPE_MASK      0xf0

#define BINARY_SEMA4       0x00
#define MUTEX_SEMA4        0x10
#define COUNTING_SEMA4     0x20

#define SEND  0
#define FLUSH 1
#define KILLD 2

/*****************************************************************************
**  Control block for v2pthread semaphore
**
**  The basic POSIX semaphore does not provide for time-bounded waits nor
**  for selection of a thread to ready based either on FIFO or PRIORITY-based
**  waiting.  This 'wrapper' extends the POSIX pthreads semaphore to include
**  the attributes of a v2pthread semaphore.
**
*****************************************************************************/
typedef struct v2pt_sema4
{
        /*
        ** Option and Type Flags for semaphore
        */
    int 
        flags;

        /*
        ** Mutex and Condition variable for semaphore post/pend
        */
    pthread_mutex_t
        sema4_lock;
    pthread_cond_t
        sema4_send;

        /*
        ** Mutex and Condition variable for semaphore delete
        */
    pthread_mutex_t
        smdel_lock;
    pthread_cond_t
        smdel_cplt;

        /*
        **  Count of available 'tokens' for semaphore.
        */
    int
        token_count;

        /*
        ** Type of send operation last performed on semaphore
        */
    int
        send_type;

        /*
        **  Ownership nesting level for mutual exclusion semaphore.
        */
    int
        recursion_level;

        /*
        ** Task control block ptr for task which currently owns semaphore
        */
    v2pthread_cb_t *
        current_owner;

        /*
        **  Pointer to next semaphore control block in semaphore list.
        */
    struct v2pt_sema4 *
        nxt_sema4;

        /*
        ** First task control block in list of tasks waiting on semaphore
        */
    v2pthread_cb_t *
        first_susp;
} v2pt_sema4_t;

/*****************************************************************************
**  External function and data references
*****************************************************************************/
extern void *
    ts_malloc( size_t blksize );
extern void
    ts_free( void *blkaddr );
extern v2pthread_cb_t *
   my_tcb( void );
extern void
   taskLock( void );
extern void
   taskUnlock( void );
extern STATUS
   taskDelay( int interval );
extern STATUS
   taskSafe( void );
extern STATUS
   taskUnsafe( void );
extern void
   link_susp_tcb( v2pthread_cb_t **list_head, v2pthread_cb_t *new_entry );
extern void
   unlink_susp_tcb( v2pthread_cb_t **list_head, v2pthread_cb_t *entry );
extern int
   signal_for_my_task( v2pthread_cb_t **list_head, int pend_order );

/*****************************************************************************
**  v2pthread Global Data Structures
*****************************************************************************/

/*
**  sema4_list is a linked list of semaphore control blocks.  It is used to
**             validate semaphores by their ID numbers.
*/
static v2pt_sema4_t *
    sema4_list;

/*
**  sema4_list_lock is a mutex used to serialize access to the semaphore list
*/
static pthread_mutex_t
    sema4_list_lock = PTHREAD_MUTEX_INITIALIZER;


/*****************************************************************************
**  sema4_valid - verifies whether the specified semaphore still exists, and if
**                so, locks exclusive access to the semaphore for the caller.
*****************************************************************************/
static int
   sema4_valid( v2pt_sema4_t *sema4 )
{
    
    v2pt_sema4_t *current_smcb;
    int found_sema4;

    found_sema4 = FALSE;

    /*
    **  Protect the semaphore list while we examine and modify it.
    */
    pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                          (void *)&sema4_list_lock );
    pthread_mutex_lock( &sema4_list_lock );

    if ( sema4_list != (v2pt_sema4_t *)NULL )
    {
        /*
        **  One or more semaphores already exist in the semaphore list...
        **  Scan the existing semaphores for a matching ID.
        */
        for ( current_smcb = sema4_list; 
              current_smcb != (v2pt_sema4_t *)NULL;
              current_smcb = current_smcb->nxt_sema4 )
        {
            if ( current_smcb == sema4 )
            {
                /*
                ** Lock mutex for semaphore access (it is assumed that a
                ** 'pthread_cleanup_push()' has already been performed
                **  by the caller in case of unexpected thread termination.)
                */
                pthread_mutex_lock( &(sema4->sema4_lock) );

                found_sema4 = TRUE;
                break;
            }
        }
    }
 
    /*
    **  Re-enable access to the semaphore list by other threads.
    */
    pthread_mutex_unlock( &sema4_list_lock );
    pthread_cleanup_pop( 0 );
 
    return( found_sema4 );
}


/*****************************************************************************
** link_smcb - appends a new semaphore control block pointer to the sema4_list
*****************************************************************************/
static void
   link_smcb( v2pt_sema4_t *new_sema4 )
{
    v2pt_sema4_t *current_smcb;

    /*
    **  Protect the semaphore list while we examine and modify it.
    */
    pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                          (void *)&sema4_list_lock );
    pthread_mutex_lock( &sema4_list_lock );

    new_sema4->nxt_sema4 = (v2pt_sema4_t *)NULL;
    if ( sema4_list != (v2pt_sema4_t *)NULL )
    {
        /*
        **  One or more semaphores already exist in the semaphore list...
        **  Insert the new entry at the tail of the list.
        */
        for ( current_smcb = sema4_list; 
              current_smcb->nxt_sema4 != (v2pt_sema4_t *)NULL;
              current_smcb = current_smcb->nxt_sema4 );
        current_smcb->nxt_sema4 = new_sema4;
#ifdef DIAG_PRINTFS 
        printf( "\r\nadd semaphore cb @ %p to list @ %p", new_sema4,
                current_smcb );
#endif
    }
    else
    {
        /*
        **  this is the first semaphore being added to the semaphore list.
        */
        sema4_list = new_sema4;
#ifdef DIAG_PRINTFS 
        printf( "\r\nadd semaphore cb @ %p to list @ %p", new_sema4,
                &sema4_list );
#endif
    }
 
    /*
    **  Re-enable access to the semaphore list by other threads.
    */
    pthread_mutex_unlock( &sema4_list_lock );
    pthread_cleanup_pop( 0 );
}

/*****************************************************************************
** unlink_smcb - removes a semaphore control block pointer from the sema4_list
*****************************************************************************/
static v2pt_sema4_t *
   unlink_smcb( v2pt_sema4_t *smid )
{
    v2pt_sema4_t *current_smcb;
    v2pt_sema4_t *selected_smcb;

    selected_smcb =  (v2pt_sema4_t *)NULL;

    if ( sema4_list != (v2pt_sema4_t *)NULL )
    {
        /*
        **  One or more semaphores exist in the semaphore list...
        **  Protect the semaphore list while we examine and modify it.
        */
        pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                              (void *)&sema4_list_lock );
        pthread_mutex_lock( &sema4_list_lock );

        /*
        **  Scan the semaphore list for an smcb with a matching semaphore ID
        */
        if ( sema4_list == smid )
        {
            /*
            **  The first semaphore in the list matches the selected ID
            */
            selected_smcb = sema4_list; 
            sema4_list = selected_smcb->nxt_sema4;
#ifdef DIAG_PRINTFS 
            printf( "\r\ndel semaphore cb @ %p from list @ %p", selected_smcb,
                    &sema4_list );
#endif
        }
        else
        {
            /*
            **  Scan the next smcb for a matching smid while retaining a
            **  pointer to the current smcb.  If the next smcb matches,
            **  select it and then unlink it from the semaphore list.
            */
            for ( current_smcb = sema4_list; 
                  current_smcb->nxt_sema4 != (v2pt_sema4_t *)NULL;
                  current_smcb = current_smcb->nxt_sema4 )
            {
                if ( current_smcb->nxt_sema4 == smid )
                {
                    /*
                    **  Semaphore ID of next smcb matches...
                    **  Select the smcb and then unlink it by linking
                    **  the selected smcb's next smcb into the current smcb.
                    */
                    selected_smcb = current_smcb->nxt_sema4;
                    current_smcb->nxt_sema4 = selected_smcb->nxt_sema4;
#ifdef DIAG_PRINTFS 
                    printf( "\r\ndel semaphore cb @ %p from list @ %p",
                            selected_smcb, current_smcb );
#endif
                    break;
                }
            }
        }

        /*
        **  Re-enable access to the semaphore list by other threads.
        */
        pthread_mutex_unlock( &sema4_list_lock );
        pthread_cleanup_pop( 0 );
    }

    return( selected_smcb );
}

/*****************************************************************************
** new_sema4 - creates a new v2pthread semaphore using pthreads resources
*****************************************************************************/
v2pt_sema4_t *
    new_sema4( int count )
{
    v2pt_sema4_t *semaphore;

    /*
    **  First allocate memory for the semaphore control block
    */
    semaphore = (v2pt_sema4_t *)ts_malloc( sizeof( v2pt_sema4_t ) );
    if ( semaphore != (v2pt_sema4_t *)NULL )
    {
        /*
        **  Ok... got a control block.
        **  Initialize the token count.
        */
        semaphore->token_count = count;

        /*
        ** Mutex and Condition variable for semaphore send/pend
        */
        pthread_mutex_init( &(semaphore->sema4_lock),
                            (pthread_mutexattr_t *)NULL );
        pthread_cond_init( &(semaphore->sema4_send),
                           (pthread_condattr_t *)NULL );

        /*
        ** Mutex and Condition variable for semaphore delete/delete
        */
        pthread_mutex_init( &(semaphore->smdel_lock),
                            (pthread_mutexattr_t *)NULL );
        pthread_cond_init( &(semaphore->smdel_cplt),
                           (pthread_condattr_t *)NULL );

        /*
        ** Type of send operation last performed on semaphore
        */
        semaphore->send_type = SEND;

        /*
        **  Ownership nesting level for mutual exclusion semaphore.
        */
        semaphore->recursion_level = 0;

        /*
        ** Task control block ptr for task which currently owns semaphore
        */
        semaphore->current_owner = (v2pthread_cb_t *)NULL;

        /*
        ** First task control block in list of tasks waiting on semaphore
        */
        semaphore->first_susp = (v2pthread_cb_t *)NULL;
    }

    return( semaphore );
}

/*****************************************************************************
** semBCreate - creates a v2pthread binary semaphore
*****************************************************************************/
v2pt_sema4_t *
    semBCreate( int opt, int initial_state )
{
    v2pt_sema4_t *semaphore;

    if ( (opt & SEM_Q_PRIORITY)
    	||(opt & SEM_DELETE_SAFE)
	||(opt & SEM_INVERSION_SAFE) )
    {
    	errno = ENOSYS;
		return( NULL );
    }

    /*
    **  First allocate memory for the semaphore control block
    */
    if ( initial_state == 0 )
        semaphore = new_sema4( 0 );
    else
        semaphore = new_sema4( 1 );

    if ( semaphore != (v2pt_sema4_t *)NULL )
    {
        /*
        **  Ok... got a control block.  Initialize it.
        */
#ifdef DIAG_PRINTFS 
        printf( "\r\nCreating binary semaphore - id %p", semaphore );
#endif

        /*
        ** Option and Type Flags for semaphore
        */
        semaphore->flags = (opt & SEM_Q_PRIORITY) | BINARY_SEMA4;

        /*
        **  Link the new semaphore into the semaphore list.
        */
        link_smcb( semaphore );
    }

    return( semaphore );
}

/*****************************************************************************
** semCCreate - creates a v2pthread counting semaphore
*****************************************************************************/
v2pt_sema4_t *
    semCCreate( int opt, int initial_count )
{
    v2pt_sema4_t *semaphore;

    if ( (opt & SEM_Q_PRIORITY)
    	||(opt & SEM_DELETE_SAFE)
	||(opt & SEM_INVERSION_SAFE) )
    {
    	errno = ENOSYS;
		return( NULL );
    }

    /*
    **  First allocate memory for the semaphore control block
    */
    semaphore = new_sema4( initial_count );

    if ( semaphore != (v2pt_sema4_t *)NULL )
    {
        /*
        **  Ok... got a control block.  Initialize it.
        */
#ifdef DIAG_PRINTFS 
        printf( "\r\nCreating counting semaphore - id %p", semaphore );
#endif

        /*
        ** Option and Type Flags for semaphore
        */
        semaphore->flags = (opt & SEM_Q_PRIORITY) | COUNTING_SEMA4;

        /*
        **  Link the new semaphore into the semaphore list.
        */
        link_smcb( semaphore );
    }

    return( semaphore );
}

/*****************************************************************************
** semMCreate - creates a v2pthread mutual exclusion semaphore
*****************************************************************************/
v2pt_sema4_t *
    semMCreate( int opt )
{
    v2pt_sema4_t *semaphore;

    if ( (opt & SEM_Q_PRIORITY)
    	||(opt & SEM_DELETE_SAFE)
	||(opt & SEM_INVERSION_SAFE) )
    {
    	errno = ENOSYS;
		return( NULL );
    }
    /*
    **  First allocate memory for the semaphore control block
    */
    semaphore = new_sema4( 1 );

    if ( semaphore != (v2pt_sema4_t *)NULL )
    {
        /*
        **  Ok... got a control block.  Initialize it.
        */
#ifdef DIAG_PRINTFS 
        printf( "\r\nCreating mutex semaphore - id %p", semaphore );
#endif

        /*
        ** Option and Type Flags for semaphore
        */
        semaphore->flags = (opt & SEM_OPT_MASK) | MUTEX_SEMA4;

        /*
        **  Link the new semaphore into the semaphore list.
        */
        link_smcb( semaphore );
    }

    return( semaphore );
}

/*****************************************************************************
** semDelete - removes the specified semaphore from the semaphore list and
**             frees the memory allocated for the semaphore control block.
*****************************************************************************/
STATUS
   semDelete( v2pt_sema4_t *semaphore )
{
#ifdef DIAG_PRINTFS 
    v2pthread_cb_t *our_tcb;
#endif
    STATUS error;

    error = OK;

    /*
    **  First ensure that the specified semaphore exists and that we have
    **  exclusive access to it.
    */
    pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                          (void *)&(semaphore->sema4_lock));
    if ( sema4_valid( semaphore ) )
    {
#ifdef DIAG_PRINTFS 
        our_tcb = my_tcb();
        printf( "\r\ntask @ %p delete semaphore @ %p", our_tcb, semaphore );
#endif
        /*
        **  Send signal and block while any tasks are still waiting
        **  on the semaphore
        */
        taskLock();
        if ( semaphore->first_susp != (v2pthread_cb_t *)NULL )
        {
#ifdef DIAG_PRINTFS 
            printf( "\r\nisemDelete - tasks pending on semaphore list @ %p",
                    semaphore->first_susp );
#endif
            /*
            ** Lock mutex for semaphore delete completion
            */
            pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                                  (void *)&(semaphore->smdel_lock) );
            pthread_mutex_lock( &(semaphore->smdel_lock) );

            /*
            **  Declare the send type
            */
            semaphore->send_type = KILLD;

            /*
            **  Signal the condition variable for the semaphore
            */
            pthread_cond_broadcast( &(semaphore->sema4_send) );

            /*
            **  Unlock the semaphore mutex. 
            */
            pthread_mutex_unlock( &(semaphore->sema4_lock) );

            /*
            **  Wait for all pended tasks to receive delete message.
            **  The last task to receive the message will signal the
            **  delete-complete condition variable.
            */
            while ( semaphore->first_susp != (v2pthread_cb_t *)NULL )
                pthread_cond_wait( &(semaphore->smdel_cplt),
                                   &(semaphore->smdel_lock) );

            /*
            **  (No need to unlock the semaphore delete completion mutex.) 
            */
            pthread_cleanup_pop( 0 );
        }

        /*
        **  First remove the semaphore from the semaphore list
        */
        unlink_smcb( semaphore );

        /*
        **  Finally delete the semaphore control block itself;
        */
        ts_free( (void *)semaphore );

        taskUnlock();
    }
    else
    {
        error = S_objLib_OBJ_ID_ERROR;       /* Invalid semaphore specified */
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
** semFlush - unblocks all tasks waiting on the specified semaphore
*****************************************************************************/
STATUS
   semFlush( v2pt_sema4_t *semaphore )
{
#ifdef DIAG_PRINTFS 
    v2pthread_cb_t *our_tcb;
#endif
    STATUS error;

    error = OK;

    /*
    **  First ensure that the specified semaphore exists and that we have
    **  exclusive access to it.
    */
    pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                          (void *)&(semaphore->sema4_lock));
    if ( sema4_valid( semaphore ) )
    {
        if ( (semaphore->flags & SEM_TYPE_MASK) != MUTEX_SEMA4 )
        {
#ifdef DIAG_PRINTFS 
            our_tcb = my_tcb();
            printf( "\r\ntask @ %p flush semaphore list @ %p", our_tcb,
                    &(semaphore->first_susp) );
#endif
            /*
            **  Send signal and block while any tasks are still waiting
            **  on the semaphore
            */
            taskLock();
            if ( semaphore->first_susp != (v2pthread_cb_t *)NULL )
            {
                /*
                ** Lock mutex for semaphore delete completion
                */
                pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                                      (void *)&(semaphore->smdel_lock) );
                pthread_mutex_lock( &(semaphore->smdel_lock) );

                /*
                **  Declare the send type
                */
                semaphore->send_type = FLUSH;

                /*
                **  Signal the condition variable for the semaphore
                */
                pthread_cond_broadcast( &(semaphore->sema4_send) );

                /*
                **  Unlock the semaphore mutex. 
                */
                pthread_mutex_unlock( &(semaphore->sema4_lock) );

                /*
                **  Wait for all pended tasks to receive delete message.
                **  The last task to receive the message will signal the
                **  delete-complete condition variable.
                */
                while ( semaphore->first_susp != (v2pthread_cb_t *)NULL )
                    pthread_cond_wait( &(semaphore->smdel_cplt),
                                       &(semaphore->smdel_lock) );

                /*
                **  Unlock the semaphore delete completion mutex. 
                */
                pthread_mutex_unlock( &(semaphore->smdel_lock) );
                pthread_cleanup_pop( 0 );
            }
            taskUnlock();
        }
        else
        {
            error = S_semLib_INVALID_OPERATION;  /* Flush invalid for mutex */

            /*
            **  Unlock the semaphore mutex. 
            */
            pthread_mutex_unlock( &(semaphore->sema4_lock) );
        }
    }
    else
    {
        error = S_objLib_OBJ_ID_ERROR;       /* Invalid semaphore specified */
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
** semGive - releases a v2pthread semaphore token and awakens the first
**           selected task waiting on the semaphore.
*****************************************************************************/
STATUS
   semGive( v2pt_sema4_t *semaphore )
{
    v2pthread_cb_t *our_tcb;
    STATUS error;

    error = OK;

    /*
    **  First ensure that the specified semaphore exists and that we have
    **  exclusive access to it.
    */
    pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                          (void *)&(semaphore->sema4_lock));
    if ( sema4_valid( semaphore ) )
    {
        /*
        **  If semaphore is a mutex, make sure we own it before giving up
        **  the token.
        */
        our_tcb = my_tcb();
        if ( ((semaphore->flags & SEM_TYPE_MASK) == MUTEX_SEMA4) &&
             (semaphore->current_owner != our_tcb) )
        {
            error = S_semLib_INVALID_OPERATION;  /* Not owner of mutex */
        }
        else
        {
            /*
            **  Either semaphore isn't a mutex or we currently own the mutex.
            **  If semaphore is a mutex, recursion level should be > zero.
            **  In this case, decrement recursion level, and if level == zero
            **  after decrement, relinquish mutex ownership.
            */
#ifdef DIAG_PRINTFS 
            printf( "\r\nSemaphore list @ %p recursion level = %d",
                    &(semaphore->first_susp), semaphore->recursion_level );
#endif
            if ( semaphore->recursion_level > 0 )
            {
                if ( (--(semaphore->recursion_level)) == 0 )
                {
                    semaphore->token_count++;
                    semaphore->current_owner = (v2pthread_cb_t *)NULL;
                    if ( semaphore->flags & SEM_DELETE_SAFE )
                        /*
                        **  Task was made deletion-safe when mutex acquired...
                        **  Remove deletion safety now.
                        */
                        taskUnsafe();
                    if ( semaphore->flags & SEM_INVERSION_SAFE )
                    {
                        /*
                        **  Task priority may have been boosted during
                        **  ownership of semaphore...
                        **  Restore to original priority now.  Call taskLock
                        **  so taskUnlock can be called to restore priority.
                        */
#ifdef DIAG_PRINTFS 
                        printf( "\r\nRestoring task priority for owner of semaphore list @ %p", 
                                &(semaphore->first_susp) );
#endif
                        taskLock();
                        taskUnlock();
                    }
                }
            }
            else
                semaphore->token_count++;

#ifdef DIAG_PRINTFS 
            printf( "\r\ntask @ %p post to semaphore list @ %p", our_tcb,
                    &(semaphore->first_susp) );
#endif

            if ( semaphore->first_susp != (v2pthread_cb_t *)NULL )
                /*
                **  Signal the condition variable for the semaphore
                */
                pthread_cond_broadcast( &(semaphore->sema4_send) );
        }

        /*
        **  Unlock the semaphore mutex. 
        */
        pthread_mutex_unlock( &(semaphore->sema4_lock) );
    }
    else
    {
        error = S_objLib_OBJ_ID_ERROR;       /* Invalid semaphore specified */
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
** waiting_on_sema4 - returns a nonzero result unless a qualifying event
**                    occurs on the specified semaphore which should cause the
**                    pended task to be awakened.  The qualifying events
**                    are:
**                        (1) a token is returned to the semaphore and the
**                            current task is selected to receive it
**                        (2) the semaphore is deleted or flushed
*****************************************************************************/
static int
    waiting_on_sema4( v2pt_sema4_t *semaphore, struct timespec *timeout,
                      int *retcode )
{
    int result;
    struct timeval now;
    ulong usec;

    if ( (semaphore->send_type & KILLD) || (semaphore->send_type & FLUSH) )
    {
        /*
        **  Semaphore has been killed... waiting is over.
        */
        result = 0;
        *retcode = 0;
    }
    else
    {
        /*
        **  Semaphore still in service... check for token availability.
        **  Initially assume no token available for our task
        */
        result = 1;

        /*
        **  Multiple posts to the semaphore may be represented by only
        **  a single signal to the condition variable, so continue
        **  checking for a token for our task as long as more tokens
        **  are available.
        */
        while ( semaphore->token_count > 0 )
        {
            /*
            **  Available token arrived... see if it's for our task.
            */
            if ( (signal_for_my_task( &(semaphore->first_susp),
                                      (semaphore->flags & SEM_Q_PRIORITY) )) )
            {
                /*
                **  Token was destined for our task specifically...
                **  waiting is over.
                */
                semaphore->token_count--;
                result = 0;
                *retcode = 0;
                break;
            }
            else
            {
                /*
                **  Token isn't for our task...  Sleep awhile to
                **  allow other tasks ahead of ours in the queue of tasks
                **  waiting on the semaphore to get their tokens, bringing
                **  our task to the head of the list.
                */
                pthread_mutex_unlock( &(semaphore->sema4_lock) );
                taskDelay( 1 );
                pthread_mutex_lock( &(semaphore->sema4_lock) );
            }

            /*
            **  If a timeout was specified, make sure we respect it and
            **  exit this loop if it expires.
            */
            if ( timeout != (struct timespec *)NULL )
            {
                gettimeofday( &now, (struct timezone *)NULL );
                if ( timeout->tv_nsec > (now.tv_usec * 1000) )
                {
                    usec = (timeout->tv_nsec - (now.tv_usec * 1000)) / 1000;
                    if ( timeout->tv_sec < now.tv_sec )
                        usec = 0;
                    else
                        usec += ((timeout->tv_sec - now.tv_sec) * 1000000);
                }
                else
                {
                    usec = ((timeout->tv_nsec + 1000000000) -
                            (now.tv_usec * 1000)) / 1000;
                    if ( (timeout->tv_sec - 1) < now.tv_sec )
                        usec = 0;
                    else
                        usec += (((timeout->tv_sec - 1) - now.tv_sec)
                                 * 1000000);
                }
                if ( usec == 0 )
                    break;
            }
        }
    }

    return( result );
}

/*****************************************************************************
** wait_for_token - blocks the calling task until a token is available on the
**                  specified v2pthread semaphore.  If a token is acquired and
**                  the semaphore is a mutex type, this function also handles
**                  priority inversion and deletion safety issues as needed.
*****************************************************************************/
STATUS
   wait_for_token( v2pt_sema4_t *semaphore, int max_wait,
                   v2pthread_cb_t *our_tcb )
{
    struct timeval now;
    struct timespec timeout;
    int retcode;
    long sec, usec;
    STATUS error;
    v2pthread_cb_t *tcb;
    int my_priority, owners_priority, sched_policy;

    error = OK;

    /*
    **  Add tcb for task to list of tasks waiting on semaphore
    */
#ifdef DIAG_PRINTFS 
    printf( "\r\ntask @ %p wait on semaphore list @ %p", our_tcb,
            &(semaphore->first_susp) );
#endif

    link_susp_tcb( &(semaphore->first_susp), our_tcb );

    retcode = 0;

    if ( max_wait == NO_WAIT )
    {
        /*
        **  Caller specified no wait on semaphore token...
        **  Check the condition variable with an immediate timeout.
        */
        gettimeofday( &now, (struct timezone *)NULL );
        timeout.tv_sec = now.tv_sec;
        timeout.tv_nsec = now.tv_usec * 1000;
        while ( (waiting_on_sema4( semaphore, &timeout, &retcode )) &&
                (retcode != ETIMEDOUT) )
        {
            retcode = pthread_cond_timedwait( &(semaphore->sema4_send),
                                              &(semaphore->sema4_lock),
                                              &timeout );
        }
    }
    else
    {
        /*
        **  Caller expects to wait on semaphore, with or without a timeout.
        **  If the semaphore is a mutex type, check for and handle any
        **  priority inversion issues.
        */
        if ( (semaphore->flags & SEM_INVERSION_SAFE) &&
             (semaphore->current_owner != (v2pthread_cb_t *)NULL) )
        {
            /*
            **  Ensure against preemption by other tasks
            */
            taskLock();

            /*
            **  Get pthreads priority of current task
            */
            my_priority = our_tcb->prv_priority.sched_priority;

            /*
            **  Get pthreads priority of task which owns mutex
            */
            tcb = semaphore->current_owner;
            owners_priority = tcb->prv_priority.sched_priority;

#ifdef DIAG_PRINTFS 
            printf( "\r\nTask @ %p priority %d owns mutex", tcb,
                    owners_priority );
            printf( "\r\nCalling task @ %p priority %d wants mutex", our_tcb,
                    my_priority );
#endif
            /*
            **  If mutex owner's priority is lower than ours, boost it
            **  to our priority level tempororily until owner releases mutex.
            **  This avoids 'priority inversion'.
            */
            if ( owners_priority < my_priority )
            {
		struct sched_param schedparam;
                pthread_attr_getschedpolicy( &(tcb->attr), &sched_policy );
                pthread_attr_getschedparam( &(tcb->attr), &schedparam );
                schedparam.sched_priority = my_priority;
                pthread_attr_setschedparam( &(tcb->attr), &schedparam );
                pthread_setschedparam( tcb->pthrid, sched_policy, &schedparam );
            }

            /*
            **  Re-enable preemption by other tasks
            */
            taskUnlock();
        }

        if ( max_wait == WAIT_FOREVER )
        {
            /*
            **  Infinite wait was specified... wait without timeout.
            */
            while ( waiting_on_sema4( semaphore, 0, &retcode ) )
            {
                pthread_cond_wait( &(semaphore->sema4_send),
                                   &(semaphore->sema4_lock) );
            }
        }
        else
        {
            /*
            **  Wait on semaphore message arrival with timeout...
            **  Calculate timeout delay in seconds and microseconds.
            */
            sec = 0;
            usec = max_wait * V2PT_TICK * 1000;
            gettimeofday( &now, (struct timezone *)NULL );
            usec += now.tv_usec;
            if ( usec > 1000000 )
            {
                sec = usec / 1000000;
                usec = usec % 1000000;
            }
            timeout.tv_sec = now.tv_sec + sec;
            timeout.tv_nsec = usec * 1000;

            /*
            **  Wait for a semaphore message for the current task or
            **  for the timeout to expire.  The loop is required since
            **  the task may be awakened by signals for semaphore
            **  tokens which are not ours, or for signals other than
            **  from a semaphore token being returned.
            */
            while ( (waiting_on_sema4( semaphore, &timeout, &retcode )) &&
                    (retcode != ETIMEDOUT) )
            {
                retcode = pthread_cond_timedwait( &(semaphore->sema4_send),
                                                  &(semaphore->sema4_lock),
                                                  &timeout );
            }
        }
    }

    /*
    **  Remove the calling task's tcb from the waiting task list
    **  for the semaphore.  Clear our TCB's suspend list pointer in
    **  case the semaphore was killed & its ctrl blk deallocated.
    */
    unlink_susp_tcb( &(semaphore->first_susp), our_tcb );
    our_tcb->suspend_list = (v2pthread_cb_t **)NULL;

    /*
    **  See if we were awakened due to a semDelete or a semFlush.
    */
    if ( (semaphore->send_type & KILLD) || (semaphore->send_type & FLUSH) )
    {
        if ( semaphore->send_type & KILLD )
        {
            error = S_objLib_OBJ_ID_ERROR;       /* Semaphore deleted */

#ifdef DIAG_PRINTFS 
            printf( "...semaphore deleted" );
#endif
        }
        else
        {
#ifdef DIAG_PRINTFS 
            printf( "...semaphore flushed" );
#endif
        }

        if ( semaphore->first_susp == (v2pthread_cb_t *)NULL )
        {
            /*
            ** Lock mutex for semaphore delete completion
            */
            pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                                  (void *)&(semaphore->smdel_lock) );
            pthread_mutex_lock( &(semaphore->smdel_lock) );

            /*
            **  Signal the delete-complete condition variable
            **  for the semaphore
            */
            pthread_cond_broadcast( &(semaphore->smdel_cplt) );

            semaphore->send_type = SEND;

            /*
            **  Unlock the semaphore delete completion mutex. 
            */
            pthread_cleanup_pop( 1 );
        }
    }
    else
    {
        /*
        **  See if we timed out or if we got a token
        */
        if ( retcode == ETIMEDOUT )
        {
            /*
            **  Timed out without a token
            */
            if ( max_wait == NO_WAIT )
            {
                error = S_objLib_OBJ_UNAVAILABLE;
#ifdef DIAG_PRINTFS 
                printf( "...no token available" );
#endif
            }
            else
            {
                error = S_objLib_OBJ_TIMEOUT;
#ifdef DIAG_PRINTFS 
                printf( "...timed out" );
#endif
            }
        }
        else
        {
            /*
            **  Just received a token from the semaphore...
            **  If the semaphore is a mutex, indicate the mutex is now owned
            **  by the current task, and then see if the task owning the
            **  token is to be made deletion-safe.  
            */
            if ( (semaphore->flags & SEM_TYPE_MASK) == MUTEX_SEMA4 )
            {
                semaphore->current_owner = our_tcb;
                semaphore->recursion_level++;
                if ( semaphore->flags & SEM_DELETE_SAFE )
                    taskSafe();
            }

#ifdef DIAG_PRINTFS 
            printf( "...rcvd semaphore token" );
#endif
        }
    }

    return( error );
}

/*****************************************************************************
** semTake - blocks the calling task until a token is available on the
**           specified v2pthread semaphore.
*****************************************************************************/
STATUS
   semTake( v2pt_sema4_t *semaphore, int max_wait )
{
    v2pthread_cb_t *our_tcb;
    STATUS error;

    error = OK;

    /*
    **  First ensure that the specified semaphore exists and that we have
    **  exclusive access to it.
    */
    pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                          (void *)&(semaphore->sema4_lock));
    if ( sema4_valid( semaphore ) )
    {
        /*
        **  If the semaphore is a mutex, check to see if this task already
        **  owns the token.
        */
        our_tcb = my_tcb();
        if ( ((semaphore->flags & SEM_TYPE_MASK) == MUTEX_SEMA4) &&
             (semaphore->current_owner == our_tcb) )
        {
            /*
            **  Current task already owns the mutex... simply increment the
            **  ownership recursion level and return.
            */
            semaphore->recursion_level++;
#ifdef DIAG_PRINTFS 
            printf( "... recursion level = %d", semaphore->recursion_level );
#endif
        }
        else
        {
            /*
            **  Either semaphore is not a mutex or current task doesn't own it
            **  Wait for timeout or acquisition of token
            */
            error = wait_for_token( semaphore, max_wait, our_tcb );
        } 

        /*
        **  Unlock the mutex for the condition variable and clean up.
        */
        pthread_mutex_unlock( &(semaphore->sema4_lock) );
    }
    else
    {
        error = S_objLib_OBJ_ID_ERROR;       /* Invalid semaphore specified */
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
