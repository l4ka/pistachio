/*****************************************************************************
 * msgQLib.c - defines the wrapper functions and data structures needed
 *             to implement a Wind River VxWorks (R) 'native' message queue API 
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include "v2pthread.h"
#include "vxw_defs.h"

#define SEND  0
#define URGNT 1
#define KILLD 2

/*****************************************************************************
**  v2pthread queue message type
*****************************************************************************/
typedef struct q_msg
{
    uint  msglen;
    char *msgbuf;
} q_msg_t;

/*****************************************************************************
**  Control block for v2pthread queue
**
**  The message list for a queue is organized into an array called an extent.
**  Actual send and fetch operations are done using a queue_head and
**  queue_tail pointer.  These pointers must 'rotate' through the extent to
**  create a logical circular buffer.  A single extra location is added
**  to ensure room for urgent messages even when the queue is 'full' for
**  normal messages.
**
*****************************************************************************/
typedef struct v2pt_mqueue
{
        /*
        ** Mutex and Condition variable for queue send/pend
        */
    pthread_mutex_t
        queue_lock;
    pthread_cond_t
        queue_send;

        /*
        ** Mutex and Condition variable for queue delete
        */
    pthread_mutex_t
        qdlet_lock;
    pthread_cond_t
        qdlet_cmplt;

        /*
        ** Mutex and Condition variable for queue-full pend 
        */
    pthread_mutex_t
        qfull_lock;
    pthread_cond_t
        queue_space;

        /*
        **  Pointer to next message pointer to be fetched from queue
        */
    q_msg_t *
        queue_head;

        /*
        **  Pointer to last message pointer sent to queue
        */
    q_msg_t *
        queue_tail;

        /*
        ** Type of send operation last performed on queue
        */
    int
        send_type;

        /*
        **  Pointer to first message in queue
        */
    q_msg_t *
        first_msg_in_queue;

        /*
        **  Pointer to last message in queue
        */
    q_msg_t *
        last_msg_in_queue;

        /*
        **  Pointer to next queue control block in queue list
        */
    struct v2pt_mqueue *
        nxt_queue;

        /*
        ** First task control block in list of tasks waiting to receive
        ** a message from queue
        */
    v2pthread_cb_t *
        first_susp;

        /*
        ** First task control block in list of tasks waiting for space to
        ** post messages to queue
        */
    v2pthread_cb_t *
        first_write_susp;

        /*
        ** Total number of messages currently sent to queue
        */
    int
        msg_count;

        /*
        ** Total (max) messages per queue
        */
    int
        msgs_per_queue;

        /*
        ** Maximum size of messages sent to queue
        */
    uint
        msg_len;

        /*
        ** sizeof( each element in queue ) used for subscript incr/decr.
        */
    size_t
        vmsg_len;

        /*
        ** Task pend order (FIFO or Priority) for queue
        */
    int
        order;
} v2pt_mqueue_t;

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
**  mqueue_list is a linked list of queue control blocks.  It is used to locate
**             queues by their ID numbers.
*/
static v2pt_mqueue_t *
    mqueue_list;

/*
**  mqueue_list_lock is a mutex used to serialize access to the queue list
*/
static pthread_mutex_t
    mqueue_list_lock = PTHREAD_MUTEX_INITIALIZER;


/*****************************************************************************
**  queue_valid - verifies whether the specified queue still exists, and if
**                so, locks exclusive access to the queue for the caller.
*****************************************************************************/
static int
   queue_valid( v2pt_mqueue_t *queue )
{
    
    v2pt_mqueue_t *current_qcb;
    int found_queue;

    found_queue = FALSE;

    /*
    **  Protect the queue list while we examine and modify it.
    */
    pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                          (void *)&mqueue_list_lock );
    pthread_mutex_lock( &mqueue_list_lock );

    if ( mqueue_list != (v2pt_mqueue_t *)NULL )
    {
        /*
        **  One or more queues already exist in the queue list...
        **  Scan the existing queues for a matching ID.
        */
        for ( current_qcb = mqueue_list; 
              current_qcb != (v2pt_mqueue_t *)NULL;
              current_qcb = current_qcb->nxt_queue )
        {
            if ( current_qcb == queue )
            {
                /*
                ** Lock mutex for queue access (it is assumed that a
                ** 'pthread_cleanup_push()' has already been performed
                **  by the caller in case of unexpected thread termination.)
                */
                pthread_mutex_lock( &(queue->queue_lock) );

                found_queue = TRUE;
                break;
            }
        }
    }
 
    /*
    **  Re-enable access to the queue list by other threads.
    */
    pthread_cleanup_pop( 1 );
 
    return( found_queue );
}

/*****************************************************************************
** link_qcb - appends a new queue control block pointer to the mqueue_list
*****************************************************************************/
static void
   link_qcb( v2pt_mqueue_t *new_mqueue )
{
    v2pt_mqueue_t *current_qcb;

    /*
    **  Protect the queue list while we examine and modify it.
    */
    pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                          (void *)&mqueue_list_lock );
    pthread_mutex_lock( &mqueue_list_lock );

    new_mqueue->nxt_queue = (v2pt_mqueue_t *)NULL;
    if ( mqueue_list != (v2pt_mqueue_t *)NULL )
    {
        /*
        **  One or more queues already exist in the queue list...
        **  Insert the new entry at the tail of the list.
        */
        for ( current_qcb = mqueue_list; 
              current_qcb->nxt_queue != (v2pt_mqueue_t *)NULL;
              current_qcb = current_qcb->nxt_queue );
        current_qcb->nxt_queue = new_mqueue;
#ifdef DIAG_PRINTFS 
        printf( "\r\nadd queue cb @ %p to list @ %p", new_mqueue, current_qcb );
#endif
    }
    else
    {
        /*
        **  this is the first queue being added to the queue list.
        */
        mqueue_list = new_mqueue;
#ifdef DIAG_PRINTFS 
        printf( "\r\nadd queue cb @ %p to list @ %p", new_mqueue, &mqueue_list );
#endif
    }
 
    /*
    **  Re-enable access to the queue list by other threads.
    */
    pthread_mutex_unlock( &mqueue_list_lock );
    pthread_cleanup_pop( 0 );
}

/*****************************************************************************
** unlink_qcb - removes a queue control block pointer from the mqueue_list
*****************************************************************************/
static v2pt_mqueue_t *
   unlink_qcb( v2pt_mqueue_t *qid )
{
    v2pt_mqueue_t *current_qcb;
    v2pt_mqueue_t *selected_qcb;

    selected_qcb =  (v2pt_mqueue_t *)NULL;

    if ( mqueue_list != (v2pt_mqueue_t *)NULL )
    {
        /*
        **  One or more queues exist in the queue list...
        **  Protect the queue list while we examine and modify it.
        */
        pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                              (void *)&mqueue_list_lock );
        pthread_mutex_lock( &mqueue_list_lock );

        /*
        **  Scan the queue list for a qcb with a matching queue ID
        */
        if ( mqueue_list == qid )
        {
            /*
            **  The first queue in the list matches the selected queue ID
            */
            selected_qcb = mqueue_list; 
            mqueue_list = selected_qcb->nxt_queue;
#ifdef DIAG_PRINTFS 
            printf( "\r\ndel queue cb @ %p from list @ %p", selected_qcb,
                    &mqueue_list );
#endif
        }
        else
        {
            /*
            **  Scan the next qcb for a matching qid while retaining a
            **  pointer to the current qcb.  If the next qcb matches,
            **  select it and then unlink it from the queue list.
            */
            for ( current_qcb = mqueue_list; 
                  current_qcb->nxt_queue != (v2pt_mqueue_t *)NULL;
                  current_qcb = current_qcb->nxt_queue )
            {
                if ( current_qcb->nxt_queue == qid )
                {
                    /*
                    **  Queue ID of next qcb matches...
                    **  Select the qcb and then unlink it by linking
                    **  the selected qcb's next qcb into the current qcb.
                    */
                    selected_qcb = current_qcb->nxt_queue;
                    current_qcb->nxt_queue = selected_qcb->nxt_queue;
#ifdef DIAG_PRINTFS 
                    printf( "\r\ndel queue cb @ %p from list @ %p",
                            selected_qcb, current_qcb );
#endif
                    break;
                }
            }
        }

        /*
        **  Re-enable access to the queue list by other threads.
        */
        pthread_mutex_unlock( &mqueue_list_lock );
        pthread_cleanup_pop( 0 );
    }

    return( selected_qcb );
}

/*****************************************************************************
** urgent_msg_to - sends a message to the front of the specified queue
*****************************************************************************/
static void
    urgent_msg_to( v2pt_mqueue_t *queue, char *msg, uint msglen )
{
    uint i;
    char *element;

    /*
    **  It is assumed when we enter this function that the queue has space
    **  to accept the message about to be sent. 
    **  Pre-decrement the queue_head (fetch) pointer, adjusting for
    **  possible wrap to the end of the queue;
    **  (Urgent messages are placed at the queue head so they will be the
    **  next message fetched from the queue - ahead of any
    **  previously-queued messages.)
    */
    element = (char *)queue->queue_head;
    element -= queue->vmsg_len;
    queue->queue_head = (q_msg_t *)element;

    if ( queue->queue_head < queue->first_msg_in_queue )
    {
        /*
        **  New queue_head pointer underflowed beginning of the extent...
        **  Wrap the queue_head pointer to the last message address
        **  in the extent allocated for the queue.
        */
        queue->queue_head = queue->last_msg_in_queue;
    }

#ifdef DIAG_PRINTFS 
        printf( " new queue_head @ %p", queue->queue_head );
#endif

    if ( msg != (char *)NULL )
    {
        element = (char *)&((queue->queue_head)->msgbuf);
        for ( i = 0; i < msglen; i++ )
        {
            *(element + i) = *(msg + i);
        }
    }
    (queue->queue_head)->msglen = msglen;

#ifdef DIAG_PRINTFS 
        printf( "\r\nsent urgent msg %p len %x to queue_head @ %p",
                msg, msglen, queue->queue_head );
#endif

    /*
    **  Increment the message counter for the queue
    */
    queue->msg_count++;

    /*
    ** Indicate type of send operation last performed on queue
    */
    queue->send_type = URGNT;
}

/*****************************************************************************
** send_msg_to - sends the specified message to the tail of the specified queue
*****************************************************************************/
static void
    send_msg_to( v2pt_mqueue_t *queue, char *msg, uint msglen )
{
    uint i;
    char *element;

    /*
    **  It is assumed when we enter this function that the queue has space
    **  to accept the message about to be sent.  Start by sending the
    **  message.
    */
    if ( msg != (char *)NULL )
    {
        element = (char *)&((queue->queue_tail)->msgbuf);
        for ( i = 0; i < msglen; i++ )
        {
            *(element + i) = *(msg + i);
        }
    }
    (queue->queue_tail)->msglen = msglen;

#ifdef DIAG_PRINTFS 
    printf( "\r\nsent msg %p len %x to queue_tail @ %p",
                msg, msglen, queue->queue_tail );
#endif

    /*
    **  Now increment the queue_tail (send) pointer, adjusting for
    **  possible wrap to the beginning of the queue.
    */
    element = (char *)queue->queue_tail;
    element += queue->vmsg_len;
    queue->queue_tail = (q_msg_t *)element;

    if ( queue->queue_tail > queue->last_msg_in_queue )
    {
        /*
        **  Wrap the queue_tail pointer to the first message address
        **  in the queue.
        */
        queue->queue_tail = queue->first_msg_in_queue;
    }

#ifdef DIAG_PRINTFS 
        printf( " new queue_tail @ %p", queue->queue_tail );
#endif

    /*
    **  Increment the message counter for the queue
    */
    queue->msg_count++;

    /*
    ** Indicate type of send operation last performed on queue
    */
    queue->send_type = SEND;

    /*
    **  Signal the condition variable for the queue
    */
    pthread_cond_broadcast( &(queue->queue_send) );
}

/*****************************************************************************
** notify_if_delete_complete - indicates if all tasks waiting on specified
**                             queue have successfully been awakened. 
*****************************************************************************/
static void
    notify_if_delete_complete( v2pt_mqueue_t *queue )
{
    /*
    **  All tasks pending on the specified queue are being awakened...
    **  If the calling task was the last task pending on the queue,
    **  signal the deletion-complete condition variable.
    */
    if ( (queue->first_susp == (v2pthread_cb_t *)NULL) &&
         (queue->first_write_susp == (v2pthread_cb_t *)NULL) )
    {
        /*
        ** Lock mutex for queue delete completion
        */
        pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                              (void *)&(queue->qdlet_lock) );
        pthread_mutex_lock( &(queue->qdlet_lock) );

        /*
        **  Signal the deletion-complete condition variable for the queue
        */
        pthread_cond_broadcast( &(queue->qdlet_cmplt) );

        /*
        **  Unlock the queue delete completion mutex. 
        */
        pthread_mutex_unlock( &(queue->qdlet_lock) );
        pthread_cleanup_pop( 0 );
    }
}


/*****************************************************************************
** fetch_msg_from - fetches the next message from the specified queue
*****************************************************************************/
static uint
    fetch_msg_from( v2pt_mqueue_t *queue, char *msg )
{
    char *element;
    uint i;
    uint msglen;

    /*
    **  It is assumed when we enter this function that the queue contains
    **  one or more messages to be fetched.
    **  Fetch the message from the queue_head message location.
    */
    if ( msg != (char *)NULL )
    {
        element = (char *)&((queue->queue_head)->msgbuf);
        msglen = (queue->queue_head)->msglen;
        for ( i = 0; i < msglen; i++ )
        {
            *(msg + i) = *(element + i);
        }
    }
    else
        msglen = 0;

#ifdef DIAG_PRINTFS 
    printf( "\r\nfetched msg of len %x from queue_head @ %p",
            msglen, queue->queue_head );
#endif

    /*
    **  Clear the message from the queue
    */
    element = (char *)&((queue->queue_head)->msgbuf);
    *element = (char)NULL;
    (queue->queue_head)->msglen = 0;

    /*
    **  Now increment the queue_head (send) pointer, adjusting for
    **  possible wrap to the beginning of the queue.
    */
    element = (char *)queue->queue_head;
    element += queue->vmsg_len;
    queue->queue_head = (q_msg_t *)element;

    if ( queue->queue_head > queue->last_msg_in_queue )
    {
        /*
        **  New queue_head pointer overflowed end of queue...
        **  Wrap the queue_head pointer to the first message address
        **  in the queue.
        */
        queue->queue_head = queue->first_msg_in_queue;
    }

#ifdef DIAG_PRINTFS 
    printf( " new queue_head @ %p", queue->queue_head );
#endif

    /*
    **  Decrement the message counter for the queue
    */
    queue->msg_count--;

    /*
    **  Now see if adequate space was freed in the queue and alert any tasks
    **  waiting for message space if adequate space now exists.
    */
    if ( queue->first_write_susp != (v2pthread_cb_t *)NULL )
    {
        if ( queue->msg_count <= (queue->msgs_per_queue - 1) )
        {

#ifdef DIAG_PRINTFS 
            printf( "\r\nqueue @ %p freed msg space for queue list @ %p",
                    queue, &(queue->first_write_susp) );
#endif
            /*
            **  Lock mutex for queue space
            */
            pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                                  (void *)&(queue->qfull_lock));
            pthread_mutex_lock( &(queue->qfull_lock) );

            /*
            **  Alert the waiting tasks that message space is available.
            */
            pthread_cond_broadcast( &(queue->queue_space) );

            /*
            **  Unlock the queue space mutex. 
            */
            pthread_cleanup_pop( 1 );
        }
    }
    return( msglen );
}

/*****************************************************************************
** data_extent_for - allocates space for queue data.  Data is allocated in
**                  a block large enough to hold (max_msgs + 1) messages.
*****************************************************************************/
static q_msg_t *
    data_extent_for( v2pt_mqueue_t *queue )
{
    char *new_extent;
    char *last_msg;
    size_t alloc_size;

    /*
    **  Calculate the number of bytes of memory needed for this extent.
    **  Start by calculating the size of each element of the extent array.
    **  Each (q_msg_t) element will contain an unsigned int byte length followed
    **  by a character array of queue->msg_len bytes.  First get the size
    **  of the q_msg_t 'header' excluding the start of the data array.
    **  Then add the size of the maximum-length message data.
    */
    queue->vmsg_len = sizeof( q_msg_t ) - sizeof( char * );
    queue->vmsg_len += (sizeof( char ) * queue->msg_len);

    /*
    **  The size of each array element is now known...
    **  Multiply it by the number of elements to get allocation size.
    */
    alloc_size = queue->vmsg_len * (queue->msgs_per_queue + 1);

    /*
    **  Now allocate a block of memory to contain the extent.
    */
    if ( (new_extent = (char *)ts_malloc( alloc_size )) != (char *)NULL )
    {
        /*
        **  Clear the memory block.  Note that this creates a NULL pointer
        **  for the nxt_extent link as well as zeroing the message array.
        */
        bzero( (void *)new_extent, (int)alloc_size );

        /*
        **  Link new data extent into the queue control block
        */
        last_msg = new_extent + (queue->vmsg_len * queue->msgs_per_queue);
        queue->first_msg_in_queue = (q_msg_t *)new_extent;
        queue->last_msg_in_queue = (q_msg_t *)last_msg;
    }
#ifdef DIAG_PRINTFS 
    printf( "\r\nnew extent @ %p for queue @ %p vmsg_len %x", new_extent,
            queue, queue->vmsg_len );
#endif
    return( (q_msg_t *)new_extent );
}

/*****************************************************************************
** msgQCreate - creates a v2pthread message queue
*****************************************************************************/
v2pt_mqueue_t *
    msgQCreate( int max_msgs, uint msglen,  int opt )
{
    v2pt_mqueue_t *queue;
    STATUS error;

    error = OK;

    /*
    **  First allocate memory for the queue control block
    */
    queue = (v2pt_mqueue_t *)ts_malloc( sizeof( v2pt_mqueue_t ) );
    if ( queue != (v2pt_mqueue_t *)NULL )
    {
        /*
        **  Ok... got a control block.
        */

        /*
        ** Total messages in memory allocation block (extent)
        ** (Extent has one extra for urgent message.)
        */
        queue->msgs_per_queue = max_msgs;

        /*
        ** Maximum size of messages sent to queue
        */
        queue->msg_len = msglen;

        /*
        **  Now allocate memory for the first queue data extent.
        */
        if ( data_extent_for( queue ) != (q_msg_t *)NULL )
        {
            /*
            **  Got both a control block and a data extent...
            **  Initialize the control block.
            */

            /*
            ** Mutex and Condition variable for queue send/pend
            */
            pthread_mutex_init( &(queue->queue_lock),
                                (pthread_mutexattr_t *)NULL );
            pthread_cond_init( &(queue->queue_send),
                               (pthread_condattr_t *)NULL );

            /*
            ** Mutex and Condition variable for queue delete
            */
            pthread_mutex_init( &(queue->qdlet_lock),
                                (pthread_mutexattr_t *)NULL );
            pthread_cond_init( &(queue->qdlet_cmplt),
                               (pthread_condattr_t *)NULL );

            /*
            ** Mutex and Condition variable for queue-full pend
            */
            pthread_mutex_init( &(queue->qfull_lock),
                                (pthread_mutexattr_t *)NULL );
            pthread_cond_init( &(queue->queue_space),
                               (pthread_condattr_t *)NULL );

            /*
            ** Pointer to next message pointer to be fetched from queue
            */
            queue->queue_head = queue->first_msg_in_queue;

            /*
            ** Pointer to last message pointer sent to queue
            */
            queue->queue_tail = queue->first_msg_in_queue;

            /*
            ** Type of send operation last performed on queue
            */
            queue->send_type = SEND;

            /*
            ** First task control block in list of tasks waiting to receive
            ** a message from queue
            */
            queue->first_susp = (v2pthread_cb_t *)NULL;

            /*
            ** First task control block in list of tasks waiting for space to
            ** post messages to queue
            */
            queue->first_write_susp = (v2pthread_cb_t *)NULL;

            /*
            ** Total number of messages currently sent to queue
            */
            queue->msg_count = 0;

            /*
            ** Task pend order (FIFO or Priority) for queue
            */
            if ( opt & MSG_Q_PRIORITY )
                queue->order = 1;
            else
                queue->order = 0;

            /*
            **  If no errors thus far, we have a new queue ready to link into
            **  the queue list.
            */
            if ( error == OK )
            {
                link_qcb( queue );
            }
            else
            {
                /*
                **  Oops!  Problem somewhere above.  Release control block
                **  and data memory and return.
                */
                ts_free( (void *)queue->first_msg_in_queue );
                ts_free( (void *)queue );
            }
        }
        else
        {
            /*
            **  No memory for queue data... free queue control block & return
            */
            ts_free( (void *)queue );
            error = S_memLib_NOT_ENOUGH_MEMORY;
        }
    }
    else
    {
        error = S_memLib_NOT_ENOUGH_MEMORY;
    }

    if ( error != OK )
    {
        errno = (int)error;
        queue = (v2pt_mqueue_t *)NULL;
    }

    return( queue );
}

/*****************************************************************************
** waiting_on_q_space - returns a nonzero result unless a qualifying event
**                      occurs on the specified queue which should cause the
**                      pended task to be awakened.  The qualifying events
**                      are:
**                          (1) message space is freed in the queue and the 
**                              current task is selected to receive it
**                          (2) the queue is deleted
*****************************************************************************/
static int
    waiting_on_q_space( v2pt_mqueue_t *queue, struct timespec *timeout,
                        int *retcode )
{
    int result;
    struct timeval now;
    ulong usec;

    if ( queue->send_type & KILLD )
    {
        /*
        **  Queue has been killed... waiting is over.
        */
        result = 0;
        *retcode = 0;
    }
    else
    {
        /*
        **  Queue still in service... check for message space availability.
        **  Initially assume no message space available for our task
        */
        result = 1;

        /*
        **  Multiple messages removed from the queue may be represented by
        **  only a single signal to the condition variable, so continue
        **  checking for a message slot for our task as long as more space
        **  is available.  Also note that for a 'zero-length' queue, the
        **  presence of a task waiting on the queue for our message will
        **  allow our message to be posted to the queue.
        */
        while ( (queue->msg_count <= (queue->msgs_per_queue - 1)) ||
                ((queue->msgs_per_queue == 0) &&
                 (queue->first_susp != (v2pthread_cb_t *)NULL)) )
        {
            /*
            **  Message slot available... see if it's for our task.
            */
            if ( signal_for_my_task( &(queue->first_write_susp),
                                     queue->order ) )
            {
                /*
                **  Message slot was destined for our task... waiting is over.
                */
                result = 0;
                *retcode = 0;
                break;
            }
            else
            {
                /*
                **  Message slot isn't for our task... continue waiting.
                **  Sleep awhile to allow other tasks ahead of ours in the
                **  list of tasks waiting on the queue to get their
                **  messages, bringing our task to the head of the list.
                */
                pthread_mutex_unlock( &(queue->qfull_lock) );
                taskDelay( 1 );
                pthread_mutex_lock( &(queue->qfull_lock) );
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
** waitToSend - sends the queue message if sufficient space becomes available
**              within the allotted waiting interval.
*****************************************************************************/
STATUS
   waitToSend( v2pt_mqueue_t *queue, char *msg, uint msglen, int wait, int pri )
{
    v2pthread_cb_t *our_tcb;
    struct timeval now;
    struct timespec timeout;
    int retcode;
    long sec, usec;
    STATUS error;

    error = OK;

    if ( wait != NO_WAIT )
    {
        /*
        **  Add tcb for task to list of tasks waiting on queue
        */
        our_tcb = my_tcb();
#ifdef DIAG_PRINTFS 
        printf( "\r\ntask @ %p wait on queue space list @ %p", our_tcb,
                &(queue->first_write_susp) );
#endif

        link_susp_tcb( &(queue->first_write_susp), our_tcb );

        retcode = 0;

        /*
        **  Unlock the queue mutex so other tasks can receive messages. 
        */
        pthread_mutex_unlock( &(queue->queue_lock) );

        /*
        **  Caller expects to wait for queue space, with or without a timeout.
        */
        if ( wait == WAIT_FOREVER )
        {
            /*
            **  Infinite wait was specified... wait without timeout.
            */
            while ( waiting_on_q_space( queue, 0, &retcode ) )
            {
                pthread_cond_wait( &(queue->queue_space),
                                   &(queue->qfull_lock) );
            }
        }
        else
        {
            /*
            **  Wait on queue message space with timeout...
            **  Calculate timeout delay in seconds and microseconds.
            */
            sec = 0;
            usec = wait * V2PT_TICK * 1000;
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
            **  Wait for queue message space for the current task or for the
            **  timeout to expire.  The loop is required since the task
            **  may be awakened by signals for messages which are
            **  not ours, or for signals other than from a message send.
            */
            while ( (waiting_on_q_space( queue, &timeout, &retcode )) &&
                    (retcode != ETIMEDOUT) )
            {
                retcode = pthread_cond_timedwait( &(queue->queue_space),
                                                  &(queue->qfull_lock),
                                                  &timeout );
            }
        }

        /*
        **  Re-lock the queue mutex before manipulating its control block. 
        */
        pthread_mutex_lock( &(queue->queue_lock) );

        /*
        **  Remove the calling task's tcb from the pended task list
        **  for the queue.  Clear our TCB's suspend list pointer in
        **  case the queue was killed & its ctrl blk deallocated.
        */
        unlink_susp_tcb( &(queue->first_write_susp), our_tcb );
        our_tcb->suspend_list = (v2pthread_cb_t **)NULL;

        /*
        **  See if we were awakened due to a msgQDelete on the queue.
        */
        if ( queue->send_type & KILLD )
        {
            notify_if_delete_complete( queue );
            error = S_objLib_OBJ_DELETED;
#ifdef DIAG_PRINTFS 
            printf( "...queue deleted" );
#endif
        }
        else
        {
            /*
            **  See if we timed out or if we got a message slot
            */
            if ( retcode == ETIMEDOUT )
            {
                /*
                **  Timed out without obtaining a message slot
                */
                error = S_objLib_OBJ_TIMEOUT;
#ifdef DIAG_PRINTFS 
                printf( "...timed out" );
#endif
            }
            else
            {
                /*
                **  A message slot was freed on the queue for this task...
                */
#ifdef DIAG_PRINTFS 
                printf( "...rcvd queue msg space" );
#endif

                if ( pri == MSG_PRI_URGENT )
                {
                    /*
                    **  Stuff the new message onto the front of the queue.
                    */
                    urgent_msg_to( queue, msg, msglen );

                    /*
                    **  Signal the condition variable for the queue
                    */
                    pthread_cond_broadcast( &(queue->queue_send) );
                }
                else
                    /*
                    **  Send the new message to the back of the queue.
                    */
                    send_msg_to( queue, msg, msglen );
            }
        }
    }
    else
    {
        /*
        **  Queue is full and no waiting allowed... return QUEUE FULL error
        */
        error = S_objLib_OBJ_UNAVAILABLE;
    }
    return( error );
}

/*****************************************************************************
** msgQSend - posts a message to the tail of a v2pthread queue and awakens the
**            first selected task pended on the queue.
*****************************************************************************/
STATUS
   msgQSend( v2pt_mqueue_t *queue, char *msg, uint msglen, int wait, int pri )
{
#ifdef DIAG_PRINTFS 
    v2pthread_cb_t *our_tcb;
#endif
    STATUS error;

    error = OK;

    /*
    **  First ensure that the specified queue exists and that we have
    **  exclusive access to it.
    */
    pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                          (void *)&(queue->queue_lock));
    if ( queue_valid( queue ) )
    {
        /*
        **  Okay... the queue is legitimate and we have exclusive access...
        **  Make sure caller's message is within max message size for queue.
        */
        if ( msglen > queue->msg_len )
        {
           error = S_msgQLib_INVALID_MSG_LENGTH;
        }
        else
        {

#ifdef DIAG_PRINTFS 
            our_tcb = my_tcb();
            if ( pri == MSG_PRI_URGENT )
                printf( "\r\ntask @ %p urgent send to queue list @ %p",
                        our_tcb, &(queue->first_susp) );
            else
                printf( "\r\ntask @ %p send to queue list @ %p", our_tcb,
                        &(queue->first_susp) );
#endif

            /*
            **  See how many messages are already sent into the queue
            */
            if ( queue->msg_count > queue->msgs_per_queue )
            {
                /*
                **  Queue is full... if waiting on space is allowed, wait
                **  until space becomes available or the timeout expires.
                **  If space becomes available, send the caller's message.
                */
                error = waitToSend( queue, msg, msglen, wait, pri );
            }
            else
            {
                if ( queue->msg_count == queue->msgs_per_queue )
                {
                    if ( (queue->msgs_per_queue == 0) &&
                         (queue->first_susp != (v2pthread_cb_t *)NULL) )
                    {
                        /*
                        **  Special case... Send the new message.
                        */
                        send_msg_to( queue, msg, msglen );
                    }
                    else
                    {
                        if ( pri == MSG_PRI_URGENT )
                        {
                            /*
                            **  Stuff the new message onto the queue.
                            */
                            urgent_msg_to( queue, msg, msglen );

                            /*
                            **  Signal the condition variable for the queue
                            */
                            pthread_cond_broadcast( &(queue->queue_send) );
                        }
                        else
                            /*
                            **  Queue is full... if waiting on space is
                            **  allowed, wait until space becomes available
                            **  or the timeout expires.  If space becomes
                            **  available, send the caller's message.
                            */
                            error = waitToSend( queue, msg, msglen, wait, pri );
                    }
                }
                else
                {
                    if ( pri == MSG_PRI_URGENT )
                    {
                        /*
                        **  Stuff the new message onto the front of the queue.
                        */
                        urgent_msg_to( queue, msg, msglen );

                        /*
                        **  Signal the condition variable for the queue
                        */
                        pthread_cond_broadcast( &(queue->queue_send) );
                    }
                    else
                        /*
                        **  Send the new message to the back of the queue.
                        */
                        send_msg_to( queue, msg, msglen );
                }
            }
        }

        /*
        **  Unlock the queue mutex. 
        */
        pthread_mutex_unlock( &(queue->queue_lock) );
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
** delete_mqueue - takes care of destroying the specified queue and freeing
**                any resources allocated for that queue
*****************************************************************************/
static void
   delete_mqueue( v2pt_mqueue_t *queue )
{
    /*
    **  First remove the queue from the queue list
    */
    unlink_qcb( queue );

    /*
    **  Next delete extent allocated for queue data.
    */
    ts_free( (void *)queue->first_msg_in_queue );

    /*
    **  Finally delete the queue control block itself;
    */
    ts_free( (void *)queue );
}

/*****************************************************************************
** msgQDelete - removes the specified queue from the queue list and frees
**              the memory allocated for the queue control block and extents.
*****************************************************************************/
STATUS
   msgQDelete( v2pt_mqueue_t *queue )
{
    STATUS error;

    error = OK;

    /*
    **  First ensure that the specified queue exists and that we have
    **  exclusive access to it.
    */
    pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                          (void *)&(queue->queue_lock));
    if ( queue_valid( queue ) )
    {
        /*
        **  Okay... the queue is legitimate and we have exclusive access...
        **  Declare the send type
        */
        queue->send_type = KILLD;

        /*
        **  Block while any tasks are still pended on the queue
        */
        taskLock();
        if ( (queue->first_susp != (v2pthread_cb_t *)NULL) ||
             (queue->first_write_susp != (v2pthread_cb_t *)NULL) )
        {
            /*
            ** Lock mutex for queue delete completion
            */
            pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                                  (void *)&(queue->qdlet_lock) );
            pthread_mutex_lock( &(queue->qdlet_lock) );

            /*
            **  Signal the condition variable for tasks waiting on
            **  messages in the queue
            */
            pthread_cond_broadcast( &(queue->queue_send) );

            /*
            **  Unlock the queue send mutex. 
            */
            pthread_mutex_unlock( &(queue->queue_lock) );

            /*
            ** Lock mutex for queue space
            */
            pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                                  (void *)&(queue->qfull_lock));
            pthread_mutex_lock( &(queue->qfull_lock) );

            /*
            **  Signal the condition variable for tasks waiting on
            **  space to post messages into the queue
            */
            pthread_cond_broadcast( &(queue->queue_space) );

            /*
            **  Unlock the queue space mutex. 
            */
            pthread_cleanup_pop( 1 );

            /*
            **  Wait for all pended tasks to receive deletion signal.
            **  The last task to receive the deletion signal will signal the
            **  deletion-complete condition variable.
            */
            while ( (queue->first_susp != (v2pthread_cb_t *)NULL) &&
                    (queue->first_write_susp != (v2pthread_cb_t *)NULL) )
            {
                pthread_cond_wait( &(queue->qdlet_cmplt),
                                   &(queue->qdlet_lock) );
            }

            /*
            **  Unlock the queue delete completion mutex. 
            */
            pthread_cleanup_pop( 1 );
        }
        else
        {
            /*
            **  Unlock the queue mutex. 
            */
            pthread_mutex_unlock( &(queue->queue_lock) );
        }

        /*
        **  No other tasks are pending on the queue by this point...
        **  Now physically delete the queue.
        */
        delete_mqueue( queue );
        taskUnlock();
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
** waiting_on_q_msg - returns a nonzero result unless a qualifying event
**                    occurs on the specified queue which should cause the
**                    pended task to be awakened.  The qualifying events
**                    are:
**                        (1) a message is sent to the queue and the current
**                            task is selected to receive it
**                        (2) the queue is deleted
*****************************************************************************/
static int
    waiting_on_q_msg( v2pt_mqueue_t *queue, struct timespec *timeout,
                       int *retcode )
{
    int result;
    struct timeval now;
    ulong usec;

    if ( queue->send_type & KILLD )
    {
        /*
        **  Queue has been killed... waiting is over.
        */
        result = 0;
        *retcode = 0;
    }
    else
    {
        /*
        **  Queue still in service... check for message availability.
        **  Initially assume no message for our task
        */
        result = 1;

        /*
        **  Multiple messages sent to the queue may be represented by only
        **  a single signal to the condition variable, so continue
        **  checking for a message for our task as long as more messages
        **  are available.
        */
        while ( queue->msg_count > 0 )
        {
            /*
            **  Message arrived... see if it's for our task.
            */
            if ( signal_for_my_task( &(queue->first_susp), queue->order ) )
            {
                /*
                **  Message was  destined for our task... waiting is over.
                */
                result = 0;
                *retcode = 0;
                break;
            }
            else
            {
                /*
                **  Message isn't for our task... continue waiting.
                **  Sleep awhile to allow other tasks ahead of ours in the
                **  list of tasks waiting on the queue to get their
                **  messages, bringing our task to the head of the list.
                */
                pthread_mutex_unlock( &(queue->queue_lock) );
                taskDelay( 1 );
                pthread_mutex_lock( &(queue->queue_lock) );
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
** msgQReceive - blocks the calling task until a message is available in the
**               specified v2pthread queue.
*****************************************************************************/
int
   msgQReceive( v2pt_mqueue_t *queue, char *msgbuf, uint buflen, int max_wait )
{
    v2pthread_cb_t *our_tcb;
    struct timeval now;
    struct timespec timeout;
    int retcode;
    int msglen;
    long sec, usec;
    STATUS error;

    error = OK;
    msglen = 0;

    /*
    **  First ensure that the specified queue exists and that we have
    **  exclusive access to it.
    */
    pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                          (void *)&(queue->queue_lock));
    if ( queue_valid( queue ) )
    {
        /*
        **  Okay... the queue is legitimate and we have exclusive access...
        **  Ensure that caller's buffer is big enough for max message size 
        **  specified for queue.
        */
        if ( buflen < queue->msg_len )
        {
           error = S_msgQLib_INVALID_MSG_LENGTH;
        }
        else
        {
            /*
            **  Add tcb for task to list of tasks waiting on queue
            */
            our_tcb = my_tcb();
#ifdef DIAG_PRINTFS 
            printf( "\r\ntask @ %p wait on queue list @ %p", our_tcb,
                    &(queue->first_susp) );
#endif

            link_susp_tcb( &(queue->first_susp), our_tcb );

            /*
            **  If tasks waiting to write to a zero-length queue, notify
            **  waiting task that we're ready to receive a message.
            */
            if ( ((queue->msgs_per_queue == 0) &&
                 (queue->first_write_susp != (v2pthread_cb_t *)NULL)) )
            {
                /*
                **  Lock mutex for queue space
                */
                pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                                      (void *)&(queue->qfull_lock));
                pthread_mutex_lock( &(queue->qfull_lock) );

                /*
                **  Alert the waiting tasks that message space is available.
                */
                pthread_cond_broadcast( &(queue->queue_space) );

                /*
                **  Unlock the queue space mutex. 
                */
                pthread_cleanup_pop( 1 );
            }

            retcode = 0;

            if ( max_wait == NO_WAIT )
            {
                /*
                **  Caller specified no wait on queue message...
                **  Check the condition variable with an immediate timeout.
                */
                gettimeofday( &now, (struct timezone *)NULL );
                timeout.tv_sec = now.tv_sec;
                timeout.tv_nsec = now.tv_usec * 1000;
                while ( (waiting_on_q_msg( queue, &timeout, &retcode )) &&
                        (retcode != ETIMEDOUT) )
                {
                    retcode = pthread_cond_timedwait( &(queue->queue_send),
                                                      &(queue->queue_lock),
                                                      &timeout );
                }
            }
            else
            {
                /*
                **  Caller expects to wait on queue, with or without a timeout.
                */
                if ( max_wait == WAIT_FOREVER )
                {
                    /*
                    **  Infinite wait was specified... wait without timeout.
                    */
                    while ( waiting_on_q_msg( queue, 0, &retcode ) )
                    {
                        pthread_cond_wait( &(queue->queue_send),
                                           &(queue->queue_lock) );
                    }
                }
                else
                {
                    /*
                    **  Wait on queue message arrival with timeout...
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
                    **  Wait for a queue message for the current task or for the
                    **  timeout to expire.  The loop is required since the task
                    **  may be awakened by signals for messages which are
                    **  not ours, or for signals other than from a message send.
                    */
                    while ( (waiting_on_q_msg( queue, &timeout, &retcode )) &&
                            (retcode != ETIMEDOUT) )
                    {
                        retcode = pthread_cond_timedwait( &(queue->queue_send),
                                                          &(queue->queue_lock),
                                                          &timeout );
                    }
                }
            }

            /*
            **  Remove the calling task's tcb from the waiting task list
            **  for the queue.  Clear our TCB's suspend list pointer in
            **  case the queue was killed & its ctrl blk deallocated.
            */
            unlink_susp_tcb( &(queue->first_susp), our_tcb );
            our_tcb->suspend_list = (v2pthread_cb_t **)NULL;

            /*
            **  See if we were awakened due to a msgQDelete on the queue.
            */
            if ( queue->send_type & KILLD )
            {
                notify_if_delete_complete( queue );
                error = S_objLib_OBJ_DELETED;
                *((char *)msgbuf) = (char)NULL;
#ifdef DIAG_PRINTFS 
                printf( "...queue deleted" );
#endif
            }
            else
            {
                /*
                **  See if we timed out or if we got a message
                */
                if ( retcode == ETIMEDOUT )
                {
                    /*
                    **  Timed out without a message
                    */
                    if ( max_wait == NO_WAIT )
                        error = S_objLib_OBJ_UNAVAILABLE;
                    else
                        error = S_objLib_OBJ_TIMEOUT;
                    *((char *)msgbuf) = (char)NULL;
#ifdef DIAG_PRINTFS 
                    printf( "...timed out" );
#endif
                }
                else
                {
                    /*
                    **  A message was sent to the queue for this task...
                    **  Retrieve the message and clear the queue contents.
                    */
                    msglen = (int)fetch_msg_from( queue, (char *)msgbuf );
#ifdef DIAG_PRINTFS 
                    printf( "...rcvd queue msg @ %p", msgbuf );
#endif
                }
            }
        }

        /*
        **  Unlock the mutex for the condition variable.
        */
        pthread_mutex_unlock( &(queue->queue_lock) );
    }
    else
    {
        error = S_objLib_OBJ_ID_ERROR;       /* Invalid queue specified */
        *((char *)msgbuf) = (char)NULL;
    }

    /*
    **  Clean up the opening pthread_cleanup_push()
    */
    pthread_cleanup_pop( 0 );

    if ( error != OK )
    {
        errno = (int)error;
        msglen = (int)ERROR;
    }

    return( msglen );
}

/*****************************************************************************
** msgQNumMsgs - returns the number of messages currently posted to the
**               specified queue.
*****************************************************************************/
int
   msgQNumMsgs( v2pt_mqueue_t *queue )
{
    int num_msgs;

    /*
    **  First ensure that the specified queue exists and that we have
    **  exclusive access to it.
    */
    pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                          (void *)&(queue->queue_lock));
    if ( queue_valid( queue ) )
    {
        /*
        **  Okay... the queue is legitimate and we have exclusive access...
        **  Get the number of messages currently posted to the queue.
        */
        num_msgs = queue->msg_count;

        /*
        **  Unlock the mutex for the condition variable.
        */
        pthread_mutex_unlock( &(queue->queue_lock) );
    }
    else
    {
        num_msgs = (int)ERROR;
    }

    /*
    **  Clean up the opening pthread_cleanup_push()
    */
    pthread_cleanup_pop( 0 );

    return( num_msgs );
}
