/*****************************************************************************
 * validate.c -  validation suite for testing the implementation of a
 *               Wind River VxWorks (R) kernel API
 *               in a POSIX Threads environment.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "v2pthread.h"
#include "vxw_hdrs.h"

typedef struct qmessage
{
    char  qname[4];
    ulong nullterm; 
    int   t_cycle;
    int   msg_no; 
} my_qmsg_t;

typedef union
{
    char      blk[16];
    my_qmsg_t msg;
} msgblk_t;

extern v2pthread_cb_t *
    my_tcb( void );
extern v2pthread_cb_t *
    tcb_for( int taskid );
extern void *
    ts_malloc( size_t blksize );

/*****************************************************************************
**  demo program global data structures
*****************************************************************************/

static int temp_taskid;
static int task1_id;
static int task2_id;
static int task3_id;
static int task4_id;
static int task5_id;
static int task6_id;
static int task7_id;
static int task8_id;
static int task9_id;
static int task10_id;

static MSG_Q_ID queue1_id;
static MSG_Q_ID queue2_id;
static MSG_Q_ID queue3_id;

static SEM_ID mutex1_id;
static SEM_ID mutex2_id;
static SEM_ID mutex3_id;

static SEM_ID sema41_id;
static SEM_ID sema42_id;
static SEM_ID sema43_id;

static SEM_ID enable1;
static SEM_ID enable2;
static SEM_ID enable3;
static SEM_ID enable4;
static SEM_ID enable5;
static SEM_ID enable6;
static SEM_ID enable7;
static SEM_ID enable8;
static SEM_ID enable9;
static SEM_ID enable10;

static SEM_ID complt1;
static SEM_ID complt2;
static SEM_ID complt3;
static SEM_ID complt4;
static SEM_ID complt5;
static SEM_ID complt6;
static SEM_ID complt7;
static SEM_ID complt8;
static SEM_ID complt9;
static SEM_ID complt10;

static WDOG_ID wdog1_id;
static WDOG_ID wdog2_id;

static int test_cycle;
static int wdog1_cycle;
static int wdog2_cycle;

static unsigned char task5_restarted = 0;

/*****************************************************************************
**  display_tcb
*****************************************************************************/
void display_tcb( ulong tid )
{
    
    int policy;
    int detachstate;
    struct sched_param schedparam;
    v2pthread_cb_t *cur_tcb;

    cur_tcb = tcb_for( tid );

    if ( cur_tcb == (v2pthread_cb_t *)NULL )
        return;

    printf(
         "\r\nTask Name: %s  Task ID: %d  Thread ID: %ld  Vxworks priority: %d",
            cur_tcb->taskname, cur_tcb->taskid, cur_tcb->pthrid,
            cur_tcb->vxw_priority );

    pthread_attr_getschedpolicy( &(cur_tcb->attr), &policy);
    switch (policy )
    {
        case SCHED_FIFO:
            printf( "\r\n    schedpolicy: SCHED_FIFO " );
            break;
        case SCHED_RR:
            printf( "\r\n    schedpolicy: SCHED_RR " );
            break;
        case SCHED_OTHER:
            printf( "\r\n    schedpolicy: SCHED_OTHER " );
            break;
        default :
            printf( "\r\n    schedpolicy: %d ", policy );
    }
    pthread_attr_getschedparam( &(cur_tcb->attr), &schedparam );
    printf( " priority %d ", schedparam.sched_priority );
    printf( " prv_priority %d ", (cur_tcb->prv_priority).sched_priority );
    pthread_attr_getdetachstate( &(cur_tcb->attr), &detachstate );
    printf( " detachstate %d ", detachstate );
}

/*****************************************************************************
**  validate_binary_semaphores
**         This function sequences through a series of actions to exercise
**         the various features and characteristics of v2pthreads semaphores
**
*****************************************************************************/
void validate_binary_semaphores( void )
{
    STATUS err;

    puts( "\r\n********** Binary Semaphore validation:" );

    /************************************************************************
    **  Non-Mutex Semaphore Flush Test
    ************************************************************************/
 
    puts( "\n.......... Next we enable Tasks 4, 7, and 10 to wait for" );
    puts( "           a token from enable1 in reverse-priority order." );
    puts( "           Then we flush enable1, waking all waiting tasks" );

    puts( "Task 1 enabling Tasks 4, 7, and 10 to consume enable1 tokens.");
    errno = 0;
    err = semGive( enable4 );
    if ( err == ERROR )
    {
        printf( "semGive of enable4 returned error %x\r\n", errno );
    }
    taskDelay( 2 );
    errno = 0;
    err = semGive( enable7 );
    if ( err == ERROR )
    {
        printf( "semGive of enable7 returned error %x\r\n", errno );
    }
    taskDelay( 2 );
    errno = 0;
    err = semGive( enable10 );
    if ( err == ERROR )
    {
        printf( "semGive of enable10 returned error %x\r\n", errno );
    }
    taskDelay( 2 );

    puts( "Task 1 flushing semaphore enable1." );
    errno = 0;
    err = semFlush( enable1 );
    if ( err == ERROR )
        printf( "\nTask 1 semFlush of enable1 returned error %x\r\n", errno );
    else
        printf( "\r\n" );

    puts( "Task 1 blocking until Tasks 4, 7, and 10 complete semFlush test." );
    errno = 0;
    err = semTake( complt4, WAIT_FOREVER );
    err = semTake( complt7, WAIT_FOREVER );
    err = semTake( complt10, WAIT_FOREVER );
}

/*****************************************************************************
**  validate_counting_semaphores
**         This function sequences through a series of actions to exercise
**         the various features and characteristics of v2pthreads semaphores
**
*****************************************************************************/
void validate_counting_semaphores( void )
{
    STATUS err;
    int i;

    puts( "\r\n********** Counting Semaphore validation:" );

    /************************************************************************
    **  Semaphore Creation Test
    ************************************************************************/
    puts( "\n.......... First we create three semaphores:" );
 
    puts( "\nCreating Counting Semaphore SEM1, FIFO queuing and 'locked'" );
    errno = 0;
    sema41_id = semCCreate( SEM_Q_FIFO, 0 );
    if ( errno != OK )
    {
        printf( "... returned error %x\r\n", errno );
    }
 
    puts( "Creating Counting Semaphore SEM2, FIFO queuing with 2 tokens" );
    errno = 0;
    sema42_id = semCCreate( SEM_Q_FIFO, 2 );
    if ( errno != OK )
    {
        printf( "... returned error %x\r\n", errno );
    }
 
    puts( "Creating Counting Semaphore SEM3, PRIORITY queuing and 'locked'" );
    errno = 0;
    sema43_id = semCCreate( SEM_Q_PRIORITY, 0 );
    if ( errno != OK )
    {
        printf( "... returned error %x\r\n", errno );
    }

    /************************************************************************
    **  Semaphore Waiting and Task Queueing Order Test
    ************************************************************************/
 
    puts( "\n.......... Next we enable Tasks 4, 7, and 10 to wait for" );
    puts( "           a token from SEM1 in reverse-priority order." );
    puts( "           Then we send three tokens to SEM1 and wait to see" );
    puts( "           the order in which the tasks get tokens." );
    puts( "           This tests the semaphore post and queueing logic." );
    puts( "           The token should be acquired by Tasks 4, 7, and 10" );
    puts( "           in that order." );

    puts( "Task 1 enabling Tasks 4, 7, and 10 to consume SEM1 tokens.");
    errno = 0;
    err = semGive( enable4 );
    if ( err == ERROR )
    {
        printf( "semGive of enable4 returned error %x\r\n", errno );
    }
    errno = 0;
    err = semGive( enable7 );
    if ( err == ERROR )
    {
        printf( "semGive of enable7 returned error %x\r\n", errno );
    }
    errno = 0;
    err = semGive( enable10 );
    if ( err == ERROR )
    {
        printf( "semGive of enable10 returned error %x\r\n", errno );
    }

    puts( "Task 1 blocking for handshake from Tasks 4, 7, and 10..." );
    err = semTake( complt4, WAIT_FOREVER );
    err = semTake( complt7, WAIT_FOREVER );
    err = semTake( complt10, WAIT_FOREVER );

    for ( i = 0; i < 3; i++ )
    {
        puts( "Task 1 sending token to semaphore SEM1." );
        errno = 0;
        err = semGive( sema41_id );
        if ( err == ERROR )
        {
            printf( "\nTask 1 send token to SEM1 returned error %x\r\n",
                    errno );
        }
    }

    puts( "Task 1 blocking for handshake from Tasks 4, 7, and 10..." );
    err = semTake( complt4, WAIT_FOREVER );
    err = semTake( complt7, WAIT_FOREVER );
    err = semTake( complt10, WAIT_FOREVER );
 
    puts( "\n.......... Next Tasks 4, 7, and 10 look for tokens from SEM2" );
    puts( "           in reverse-priority order.  However, SEM2 has only two" );
    puts( "           tokens available, so one task will fail to acquire one.");
    puts( "           Since the tasks did not wait on the semaphore, the");
    puts( "           loser of the race will return an error 0x3d0002");
    puts( "Task 1 enabling Tasks 4, 7, and 10 to consume SEM2 tokens.");
    errno = 0;
    err = semGive( enable4 );
    if ( err == ERROR )
    {
        printf( "semGive of enable4 returned error %x\r\n", errno );
    }
    errno = 0;
    err = semGive( enable7 );
    if ( err == ERROR )
    {
        printf( "semGive of enable7 returned error %x\r\n", errno );
    }
    errno = 0;
    err = semGive( enable10 );
    if ( err == ERROR )
    {
        printf( "semGive of enable10 returned error %x\r\n", errno );
    }

    puts( "Task 1 blocking for handshake from Tasks 4, 7, and 10..." );
    err = semTake( complt4, WAIT_FOREVER );
    err = semTake( complt7, WAIT_FOREVER );
    err = semTake( complt10, WAIT_FOREVER );
            
    puts( "\n.......... Next Tasks 4, 7, and 10 look for tokens from SEM3" );
    puts( "           in reverse-priority order.  However, SEM3 will be sent" );
    puts( "           only two tokens, so one task will fail to acquire one.");
    puts( "           Since the tasks do wait on the semaphore, the lowest");
    puts( "           priority task will return an errno 0x3d0004");

    puts( "Task 1 enabling Tasks 4, 7, and 10 to consume SEM3 tokens.");
    errno = 0;
    err = semGive( enable4 );
    if ( err == ERROR )
    {
        printf( "semGive of enable4 returned error %x\r\n", errno );
    }
    taskDelay( 2 );
    errno = 0;
    err = semGive( enable7 );
    if ( err == ERROR )
    {
        printf( "semGive of enable7 returned error %x\r\n", errno );
    }
    taskDelay( 2 );
    errno = 0;
    err = semGive( enable10 );
    if ( err == ERROR )
    {
        printf( "semGive of enable10 returned error %x\r\n", errno );
    }
    taskDelay( 2 );

    puts( "Task 1 blocking for handshake from Tasks 4, 7, and 10..." );
    errno = 0;
    err = semTake( complt4, WAIT_FOREVER );
    err = semTake( complt7, WAIT_FOREVER );
    err = semTake( complt10, WAIT_FOREVER );

    for ( i = 0; i < 2; i++ )
    {
        puts( "Task 1 sending token to semaphore SEM3." );
        errno = 0;
        err = semGive( sema43_id );
        if ( err == ERROR )
        {
            printf( "\nTask 1 send token to SEM3 returned error %x\r\n", errno );
        }
    }
    puts( "Task 1 blocking until Tasks 4, 7, and 10 consume SEM3 tokens." );
    errno = 0;
    err = semTake( complt4, WAIT_FOREVER );
    err = semTake( complt7, WAIT_FOREVER );
    err = semTake( complt10, WAIT_FOREVER );

    /************************************************************************
    **  Semaphore Deletion Test
    ************************************************************************/

    puts( "\n.......... Next Tasks 4, 7, and 10 look for tokens from SEM1" );
    puts( "           in priority order.  Task 1 will delete SEM1 before any" );
    puts( "           tokens become available.  Tasks 4, 7, and 10 should be" );
    puts( "           awakened and return errno 0x3d0001." );
    puts( "           SEM2 will be deleted with no tasks waiting." );
    puts( "           This tests the semDelete logic." );
    taskDelay( 2 );
    puts( "Task 1 deleting semaphore SEM1." );
    errno = 0;
    err = semDelete( sema41_id );
    if ( err == ERROR )
        printf( "\nTask 1 delete of SEM1 returned error %x\r\n", errno );
    else
        printf( "\r\n" );
    puts( "Task 1 deleting semaphore SEM2." );
    errno = 0;
    err = semDelete( sema42_id );
    if ( err == ERROR )
        printf( "\nTask 1 delete of SEM2 returned error %x\r\n", errno );
    else
        printf( "\r\n" );

    puts( "Task 1 blocking until Tasks 4, 7, and 10 complete semDelete test." );
    errno = 0;
    err = semTake( complt4, WAIT_FOREVER );
    err = semTake( complt7, WAIT_FOREVER );
    err = semTake( complt10, WAIT_FOREVER );

    /************************************************************************
    **  Semaphore-Not-Found Test
    ************************************************************************/

    puts( "\n.......... Finally, we verify the error codes returned when" );
    puts( "           a non-existent semaphore is specified." );

    errno = 0;
    err = semGive( sema41_id );
    printf( "\nsemGive for SEM1 returned error %x\r\n", errno );

    errno = 0;
    err = semTake( sema41_id, NO_WAIT );
    printf( "\nsemTake for SEM1 (no waiting) returned error %x\r\n", errno );

    errno = 0;
    err = semTake( sema41_id, WAIT_FOREVER );
    printf( "\nsemTake for SEM1 (wait forever) returned error %x\r\n", errno );

    errno = 0;
    err = semDelete( sema41_id );
    printf( "\nsemDelete for SEM1 returned error %x\r\n", errno );
}

/*****************************************************************************
**  validate_mutexes
**         This function sequences through a series of actions to exercise
**         the various features and characteristics of v2pthreads mutexes
**
*****************************************************************************/
void validate_mutexes( void )
{
    STATUS err;
    int i;

    puts( "\r\n********** Mutex Semaphore validation:" );

    /************************************************************************
    **  Mutex Semaphore Creation Test
    ************************************************************************/
    puts( "\n.......... First we create three mutex semaphores:" );
 
    puts( "\nCreating Mutex 1, FIFO queuing" );
    errno = 0;
    mutex1_id = semMCreate( SEM_Q_FIFO );
    if ( errno != OK )
    {
        printf( "... returned error %x\r\n", errno );
    }
    puts( "\nTask 1 locking Mutex 1" );
    errno = 0;
    err = semTake( mutex1_id, WAIT_FOREVER );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }
 
    puts( "\nCreating Mutex 2, FIFO queuing and Delete Safety" );
    errno = 0;
    mutex2_id = semMCreate( SEM_Q_FIFO | SEM_DELETE_SAFE );
    if ( errno != OK )
    {
        printf( "... returned error %x\r\n", errno );
    }
 
    puts( "\nCreating Mutex 3, PRIORITY queuing and Inversion Safety" );
    errno = 0;
    mutex3_id = semMCreate( SEM_Q_PRIORITY | SEM_INVERSION_SAFE );
    if ( errno != OK )
    {
        printf( "... returned error %x\r\n", errno );
    }

    /************************************************************************
    **  Mutex Semaphore Recursive semTake Test
    ************************************************************************/
    puts( "\n.......... Next we attempt to recursively lock Mutex 3." );
    puts( "           This should return no errors." );

    for ( i = 1; i < 4; i++ )
    {
        printf( "Task 1 recursively locking Mutex 3 - Locking pass %d", i );
        errno = 0;
        err = semTake( mutex3_id, WAIT_FOREVER );
        if ( err == ERROR )
            printf( " returned error %x\r\n", errno );
        else
            puts( "\r\n" );
    }

    /************************************************************************
    **  Mutex Semaphore Recursive semGive Test
    ************************************************************************/
    puts( "\n.......... Now we recursively unlock Mutex 3." );
    puts( "           This should return no errors on passes 1 through 3," );
    puts( "           but should return error 0x160068 on pass 4" );
    puts( "           since we only recursively locked the mutex 3 times." );

    for ( i = 1; i < 5; i++ )
    {
        printf( "Task 1 recursively unlocking Mutex 3 - Unlocking pass %d",
                i );
        errno = 0;
        err = semGive( mutex3_id );
        if ( err == ERROR )
            printf( " returned error %x\r\n", errno );
        else
            puts( "\r\n" );
    }

    /************************************************************************
    **  Mutex Semaphore semGive (Not Owner) Test 
    ************************************************************************/
    puts( "\n.......... Next we enable Task 2 to attempt to unlock Mutex 1," );
    puts( "           which Task 1 previously locked and still 'owns'." );
    puts( "           This should return an error 0x160068." );

    puts( "Task 1 signalling Task 2 to attempt to unlock Mutex 1.");
    errno = 0;
    err = semGive( enable2 );
    if ( err == ERROR )
    {
        printf( "semGive of enable2 returned error %x\r\n", errno );
    }

    puts( "Task 1 blocking for handshake from Task 2..." );
    errno = 0;
    err = semTake( complt2, WAIT_FOREVER );

    /************************************************************************
    **  Mutex Semaphore Flush Test
    ************************************************************************/

    puts( "\n.......... Next we attempt to flush Mutex 1.  Flushes are not" );
    puts( "           allowed for mutex semaphores, so this should fail with" );
    puts( "           an error 0x160068." );

    puts( "Task 1 attempting to flush Mutex 1." );
    errno = 0;
    err = semFlush( mutex1_id );
    if ( err == ERROR )
        printf( "\nTask 1 semFlush of Mutex 1 returned error %x\r\n", errno );
    else
        printf( "\r\n" );

    /************************************************************************
    **  Mutex Semaphore Priority Inversion Safety Test 
    ************************************************************************/
    puts( "\n.......... Next we test mutex priority inversion protection." );
    puts( "           First task 2 (priority 20) will lock Mutex 3.  Then");
    puts( "           task 1 (priority 5) will attempt to lock Mutex 3, and");
    puts( "           will block.  At this point task 2's priority should be");
    puts( "           boosted to equal that of task 1.  Task 2 will then");
    puts( "           unlock the mutex, at which point task 2 should drop");
    puts( "           back to its initial priority setting, and task 1 should");
    puts( "           acquire the mutex.");

    puts( "Task 1 enabling Task 2 to acquire ownership of Mutex 3.");
    errno = 0;
    err = semGive( enable2 );
    if ( err == ERROR )
    {
        printf( "semGive of enable2 returned error %x\r\n", errno );
    }

    puts( "Task 1 blocking for handshake from Task 2..." );
    errno = 0;
    err = semTake( complt2, WAIT_FOREVER );

    printf( "Task 1 attempting to lock Mutex 3" );
    errno = 0;
    err = semTake( mutex3_id, WAIT_FOREVER );
    if ( err == ERROR )
        printf( " returned error %x\r\n", errno );
    else
        puts( "\r\n" );

    puts( "Task 1 blocking until Task 2 completes priority inversion test." );
    errno = 0;
    err = semTake( complt2, WAIT_FOREVER );

    /************************************************************************
    **  Mutex Semaphore Task Deletion Safety Test 
    ************************************************************************/
    puts( "\n.......... Next we test mutex automatic deletion safety." );
    puts( "           First task 2 will lock Mutex 2, making task 2 safe");
    puts( "           from deletion.  Task 1 will then attempt to delete");
    puts( "           task 2, and should block.  Task 2 will then unlock the");
    puts( "           mutex, at which point task 2 should become deletable,");
    puts( "           and task 1 should complete the deletion of task 2.");

    puts( "Task 1 enabling Task 2 to acquire ownership of Mutex 2.");
    errno = 0;
    err = semGive( enable2 );
    if ( err == ERROR )
    {
        printf( "semGive of enable2 returned error %x\r\n", errno );
    }

    puts( "Task 1 blocking for handshake from Task 2..." );
    errno = 0;
    err = semTake( complt2, WAIT_FOREVER );

    printf( "Task 1 attempting to delete Task 2" );
    taskDelete( task2_id );

    puts( "\nTask 1 calling taskIdVerify to confirm Task 2 deleted..." );
    err = taskIdVerify( task2_id );
    if ( err == ERROR )
        puts( "taskIdVerify indicates Task 2 does not exist." );
    else
        puts( "taskIdVerify indicates Task 2 still exists." );

    /************************************************************************
    **  Mutex Semaphore Deletion Test 
    ************************************************************************/
    puts( "\n.......... Finally we test mutex deletion behavior for a" );
    puts( "           mutex owned by the task deleting it.  (This is the");
    puts( "           recommended technique for mutex deletion.)");
    puts( "           Then we test deletion of a mutex not owned by the task");
    puts( "           which is deleting it.  No errors should be returned.");

    puts( "Task 1 deleting Mutex 1." );
    errno = 0;
    err = semDelete( mutex1_id );
    if ( err == ERROR )
        printf( "\nTask 1 delete of Mutex 1 returned error %x\r\n", errno );
    else
        printf( "\r\n" );

    puts( "Task 1 deleting Mutex 3." );
    errno = 0;
    err = semDelete( mutex3_id );
    if ( err == ERROR )
        printf( "\nTask 1 delete of Mutex 3 returned error %x\r\n", errno );
    else
        printf( "\r\n" );

    puts( "Task 1 deleting Mutex 2." );
    errno = 0;
    err = semDelete( mutex2_id );
    if ( err == ERROR )
        printf( "\nTask 1 delete of Mutex 2 returned error %x\r\n", errno );
    else
        printf( "\r\n" );
}

/*****************************************************************************
**  validate_msg_queues
**         This function sequences through a series of actions to exercise
**         the various features and characteristics of v2pthreads message
**         queues.
**
*****************************************************************************/
void validate_msg_queues( void )
{
    STATUS err;
    int message_num;
    int msg_count;
    msgblk_t msg;
    msgblk_t rcvd_msg;
    char msg_string[80];

    puts( "\r\n********** Message Queue validation:" );
    /************************************************************************
    **  Message Queue-full Test
    ************************************************************************/
    puts( "\n.......... First we created three message queues" );
    puts( "           Next we enable Tasks 3, 6, and 9 to consume" );
    puts( "           messages from MSQ1 in reverse-priority order." );
    puts( "           The enables are sent to lowest-priority tasks first." );

    puts( "Task 1 enabling Tasks 3, 6, and 9 to consume MSQ1 messages.");
    errno = 0;
    err = semGive( enable3 );
    if ( err == ERROR )
    {
        printf( "semGive of enable3 returned error %x\r\n", errno );
    }
    errno = 0;
    err = semGive( enable6 );
    if ( err == ERROR )
    {
        printf( "semGive of enable6 returned error %x\r\n", errno );
    }
    errno = 0;
    err = semGive( enable9 );
    if ( err == ERROR )
    {
        printf( "semGive of enable9 returned error %x\r\n", errno );
    }

    /*
    **  Delay to allow consumer tasks to recognize enable signals.
    */
    taskDelay( 2 );

    puts( "\n.......... Next we attempt to send nine messages to each queue" );
    puts( "           This tests message queue full logic." );
    puts( "           The message queue MSQ1 should return no errors" );
    puts( "           but MSQ2 should return five 0x3d0002 errs" );
    puts( "           and MSQ3 should return nine 0x3d0002 errs" );

    /*
    **  This is a 'sneaky trick' to null-terminate the object name string.
    */
    msg.msg.nullterm = (ulong)NULL; 

    /*
    **  Define nine unique messages to send to each message queue.
    */

    msg.msg.qname[0] = 'M';
    msg.msg.qname[1] = 'S';
    msg.msg.qname[2] = 'Q';
    for ( message_num = 1; message_num < 10; message_num++ )
    { 
        /*
        **  Post a unique message to each of the message queues
        */
        msg.msg.t_cycle = test_cycle;
        msg.msg.msg_no = message_num;
            
        msg.msg.qname[3] = '1';
        printf( "Task 1 sending msg %d to %s", message_num, msg.msg.qname );
        errno = 0;
        err = msgQSend( queue1_id, msg.blk, 16, NO_WAIT, MSG_PRI_NORMAL );
        if ( err == ERROR )
            printf( " returned error %x\r\n", errno );
        else
            printf( "\r\n" );
            
        msg.msg.qname[3] = '2';
        printf( "Task 1 sending msg %d to %s", message_num, msg.msg.qname );
        errno = 0;
        err = msgQSend( queue2_id, msg.blk, 16, NO_WAIT, MSG_PRI_NORMAL );
        if ( err == ERROR )
            printf( " returned error %x\r\n", errno );
        else
            printf( "\r\n" );
            
        msg.msg.qname[3] = '3';
        printf( "Task 1 sending msg %d to %s", message_num, msg.msg.qname );
        errno = 0;
        err = msgQSend( queue3_id, msg.blk, 16, NO_WAIT, MSG_PRI_NORMAL );
        if ( err == ERROR )
            printf( " returned error %x\r\n", errno );
        else
            printf( "\r\n" );
    }

    puts( "\n.......... Sending a message to a message queue which" );
    puts( "           is larger than the queue's maximum message size would" );
    puts( "           either have to truncate the message or cause buffer" );
    puts( "           overflow - neither of which is desirable.  For this" );
    puts( "           reason, an attempt to do this generates an error 0x410001." );
    puts( "           This tests the overlength message detection logic." );
    errno = 0;
    err = msgQSend( queue1_id, msg_string, 80, NO_WAIT, MSG_PRI_NORMAL );
    printf( "\nmsgQSend 80-byte msg for 16-byte MSQ1 returned error %x\r\n", errno );

    puts( "\n.......... Receiving a message from a message queue which" );
    puts( "           is larger than the caller's message buffer size would" );
    puts( "           either have to truncate the message or cause buffer" );
    puts( "           overflow - neither of which is desirable.  For this" );
    puts( "           reason, an attempt to do this generates an error 0x410001." );
    puts( "           This tests the underlength buffer detection logic." );

    errno = 0;
    err = msgQReceive( queue2_id, rcvd_msg.blk, 16, NO_WAIT );
    printf( "\n16-byte msgQReceive for 128-byte MSQ2 returned error %x\r\n", errno );
    /************************************************************************
    **  Waiting Task 'Queuing Order' (FIFO vs. PRIORITY) Test
    ************************************************************************/
    puts( "\n.......... Earlier we enabled Tasks 3, 6, and 9 to consume" );
    puts( "           messages from MSQ1 in reverse-priority order." );
    puts( "           The enables were sent to lowest-priority tasks first." );
    puts( "           Since the queues awaken tasks in PRIORITY order, this" );
    puts( "           tests the task queueing order logic.\r\n" );
    puts( "           Tasks 9, 6, and 3 - in that order - should each" );
    puts( "           receive 3 messages from MSQ1" );

    puts( "\r\nTask 1 blocking while messages are consumed..." );
    puts( "Task 1 waiting to receive ALL of complt3, complt6, complt9 tokens." );
    errno = 0;
    err = semTake( complt3, WAIT_FOREVER );
    err = semTake( complt6, WAIT_FOREVER );
    err = semTake( complt9, WAIT_FOREVER );

    puts( "\n.......... Next we send a message to zero-length MSQ3 with" );
    puts( "           Task 9 waiting on MSQ3... This should succeed." );
    puts( "           This tests the zero-length queue send logic." );

    puts( "Task 1 enabling Task 9 (priority 10) to consume MSQ3 messages.");
    errno = 0;
    err = semGive( enable9 );
    if ( err == ERROR )
    {
        printf( "semGive of enable9 returned error %x\r\n", errno );
    }

    puts( "Task 1 blocking for handshake from Task 9..." );
    errno = 0;
    err = semTake( complt9, WAIT_FOREVER );
    taskDelay( 2 );
            
    printf( "Task 1 Sending msg %d to %s", message_num, msg.msg.qname );
    msg.msg.msg_no = message_num;
    errno = 0;
    msgQSend( queue3_id, msg.blk, 16, NO_WAIT, MSG_PRI_NORMAL );
    if ( err == ERROR )
        printf( " returned error %x\r\n", errno );
    else
        printf( "\r\n" );

    puts( "\r\nTask 1 blocking while message is consumed..." );
    errno = 0;
    err = semTake( complt9, WAIT_FOREVER );

    /************************************************************************
    **  Message Queue-Delete Test
    ************************************************************************/
    puts( "\n.......... Next we enable Tasks 3, 6, and 9 to wait for" );
    puts( "           a message on MSQ1.  Then we delete MSQ1." );
    puts( "           This should wake each of Tasks 3, 6, and 9," );
    puts( "           and they should each return an error 0x3d0003." );
    puts( "           This tests the queue delete logic." );

    puts( "Task 1 enabling Tasks 3, 6, and 9 to consume MSQ1 messages.");
    errno = 0;
    err = semGive( enable3 );
    if ( err == ERROR )
    {
        printf( "semGive of enable3 returned error %x\r\n", errno );
    }
    err = semGive( enable6 );
    if ( err == ERROR )
    {
        printf( "semGive of enable6 returned error %x\r\n", errno );
    }
    errno = 0;
    err = semGive( enable9 );
    if ( err == ERROR )
    {
        printf( "semGive of enable9 returned error %x\r\n", errno );
    }

    puts( "Task 1 blocking for handshake from Tasks 3, 6, and 9..." );
    errno = 0;
    err = semTake( complt3, WAIT_FOREVER );
    err = semTake( complt6, WAIT_FOREVER );
    err = semTake( complt9, WAIT_FOREVER );
    taskDelay( 2 );
            
    puts( "\r\nTask 1 deleting MSQ1" );
    errno = 0;
    err = msgQDelete( queue1_id );
    if ( err == ERROR )
        printf( "Task 1 msgQDelete on MSQ1 returned error %x\r\n", errno );
    else
        printf( "\r\n" );

    puts( "\r\nTask 1 blocking until consumer tasks acknowledge deletion..." );
    puts( "Task 1 waiting to receive ALL of complt3, complt6, complt9 tokens." );
    errno = 0;
    err = semTake( complt3, WAIT_FOREVER );
    err = semTake( complt6, WAIT_FOREVER );
    err = semTake( complt9, WAIT_FOREVER );

    /************************************************************************
    **  Message Queue Wait-to-Send Test
    ************************************************************************/
    puts( "\n.......... During the queue-full tests above, four messages" );
    puts( "           were sent, filling message queue MSQ2." );
    puts( "           Now we will attempt to send another message and " );
    puts( "           Task 1 will block waiting on room in the queue for" );
    puts( "           the new message.  After a short delay, a consumer task" );
    puts( "           will receive the first message in the queue, creating" );
    puts( "           space for the new message and unblocking Task 1." );

    puts( "Task 1 enabling Task 3 to consume one MSQ2 message after delay.");
    errno = 0;
    err = semGive( enable3 );
    if ( err == ERROR )
         printf( "semGive of enable3 returned error %x\r\n", errno );
    puts( "Task 1 blocking for handshake from Task 3..." );
    errno = 0;
    err = semTake( complt3, WAIT_FOREVER );
            
    msg.msg.msg_no = ++message_num;
    msg.msg.qname[3] = '2';
    printf( "Task 1 waiting indefinitely to send msg %d to %s", message_num,
            msg.msg.qname );
    errno = 0;
    err = msgQSend( queue2_id, msg.blk, 16, WAIT_FOREVER, MSG_PRI_NORMAL );
    if ( err == ERROR )
        printf( " returned error %x\r\n", errno );
    else
        printf( "\r\nTask 1 sent msg %d to %s", message_num, msg.msg.qname );
    taskDelay( 10 );

    puts( "\n.......... Next we will attempt to send another message and " );
    puts( "           Task 1 will block waiting on room in the queue for" );
    puts( "           the new message.  After a 1 second delay, Task 1" );
    puts( "           will quit waiting for space and will return an error" );
    puts( "           message 0x3d0004." );

    msg.msg.msg_no = ++message_num;
    msg.msg.qname[3] = '2';
    printf( "Task 1 waiting up to 1 second to send msg %d to %s", message_num,
            msg.msg.qname );
    errno = 0;
    err = msgQSend( queue2_id, msg.blk, 16, 100, MSG_PRI_NORMAL );
    if ( err == ERROR )
        printf( " returned error %x\r\n", errno );
    else
        printf( "\r\n" );

    puts( "\n.......... Next Task 6 will attempt to send a message to MSQ3." );
    puts( "           Task 6 will block waiting on room in the queue for" );
    puts( "           the new message.  After a 1 second delay, Task 1" );
    puts( "           will delete MSQ3 and Task 6 will return an error" );
    puts( "           message 0x3d0003." );

    puts( "Task 1 enabling Task 6 to send one MSQ3 message.");
    errno = 0;
    err = semGive( enable6 );
    if ( err == ERROR )
         printf( "semGive of enable6 returned error %x\r\n", errno );
    puts( "Task 1 blocking for handshake from Task 6..." );
    errno = 0;
    err = semTake( complt6, WAIT_FOREVER );

    message_num++;
    taskDelay( 100 );
            
    puts( "Task 1 deleting MSQ3 with Task6 waiting for queue space" );
    errno = 0;
    err = msgQDelete( queue3_id );
    if ( err == ERROR )
        printf( "Task 1 msgQDelete on MSQ3 returned error %x\r\n", errno );
    else
        printf( "\r\n" );
            
    puts( "Task 1 blocking for handshake from Task 6..." );
    errno = 0;
    err = semTake( complt6, WAIT_FOREVER );

    /************************************************************************
    **  Message Queue Urgent Message Test
    ************************************************************************/
    puts( "\n.......... During the queue-full tests above, four messages" );
    puts( "           were sent, filling message queue MSQ2." );
    puts( "           Now we will send an urgent message and then enable" );
    puts( "           a consumer task to receive all the messages in MSQ2." );
    puts( "           The consumer task should receive five messages in all" );
    puts( "           from MSQ2, starting with the urgent message." );

    msg.msg.msg_no = ++message_num;
    msg.msg.qname[3] = '2';
    printf( "Task 1 Sending urgent msg %d to %s", message_num, msg.msg.qname );
    errno = 0;
    err = msgQSend( queue2_id, msg.blk, 16, NO_WAIT, MSG_PRI_URGENT );
    if ( err == ERROR )
        printf( " returned error %x\r\n", errno );
    else
        printf( "\r\n" );

    puts( "Task 1 enabling Task 6 to consume MSQ2 messages.");
    errno = 0;
    err = semGive( enable6 );
    if ( err == ERROR )
         printf( "semGive of enable6 returned error %x\r\n", errno );
    puts( "Task 1 blocking for handshake from Task 6..." );
    errno = 0;
    err = semTake( complt6, WAIT_FOREVER );
            
    puts( "\r\nTask 1 blocking while messages are consumed..." );
    errno = 0;
    err = semTake( complt6, WAIT_FOREVER );

    /************************************************************************
    **  Message Queue Number of Messages and Queue-Not_Found Test
    ************************************************************************/
    puts( "\n.......... Finally, we test the msgQNumMsgs logic..." );
    puts( "           Then we verify the error codes returned when" );
    puts( "           a non-existent queue is specified." );

    msg_count = msgQNumMsgs( queue2_id );
    if ( msg_count == ERROR )
        printf( "\nmsgQNumMsgs for MSQ2 returned error\r\n" );
    else
        printf( "\nmsgQNumMsgs for MSQ2 returned %d messages\r\n", msg_count );

    errno = 0;
    msg_count = msgQNumMsgs( queue1_id );
    if ( msg_count == ERROR )
        printf( "\nmsgQNumMsgs for MSQ1 returned error\r\n" );
    else
        printf( "\nmsgQNumMsgs for MSQ1 returned %d messages\r\n", msg_count );

    errno = 0;
    msgQSend( queue1_id, msg.blk, 16, NO_WAIT, MSG_PRI_NORMAL );
    printf( "\nmsgQSend for MSQ1 returned error %x\r\n", errno );

    errno = 0;
    msgQReceive( queue1_id, rcvd_msg.blk, 16, NO_WAIT );
    printf( "\nmsgQReceive for MSQ1 (no waiting) returned error %x\r\n", errno );

    errno = 0;
    msgQReceive( queue1_id, rcvd_msg.blk, 16, WAIT_FOREVER );
    printf( "\nmsgQReceive for MSQ1 (wait forever) returned error %x\r\n", errno );

    errno = 0;
    msgQDelete( queue1_id );
    printf( "\nmsgQDelete for MSQ1 returned error %x\r\n", errno );
}

/*****************************************************************************
**  t_250_msec
*****************************************************************************/
void t_250_msec( int dummy0 )
{
    wdStart( wdog2_id, 25, t_250_msec, 0 );
    
    wdog2_cycle += 250;
    
    semGive( enable8 );
}

/*****************************************************************************
**  t_500_msec
*****************************************************************************/
void t_500_msec( int dummy0 )
{
    wdStart( wdog1_id, 50, t_500_msec, 0 );
    
    wdog1_cycle += 500;
    
    semGive( enable5 );
}

/*****************************************************************************
**  t_1000_msec
*****************************************************************************/
void t_1000_msec( int dummy0 )
{
    wdog2_cycle += 1000;
    
    semGive( enable8 );
}

/*****************************************************************************
**  validate_watchdog_timers
**         This function sequences through a series of actions to exercise
**         the various features and characteristics of v2pthreads message
**         queues.
**
*****************************************************************************/
void validate_watchdog_timers( void )
{
    STATUS err;

    puts( "\r\n********** Watchdog Timer validation:" );
    /************************************************************************
    **  WatchDog Timer Creation Test
    ************************************************************************/
    puts( "\n.......... First we create two inactive watchdog timers." );

    wdog1_cycle = 0;
    wdog2_cycle = 0;

    wdog1_id = wdCreate();
    if ( wdog1_id == (WDOG_ID)NULL )
        puts( "wdCreate failed to create Watchdog 1." );
    else
        puts( "wdCreate successfully created Watchdog 1." );

    wdog2_id = wdCreate();
    if ( wdog2_id == (WDOG_ID)NULL )
        puts( "wdCreate failed to create Watchdog 2." );
    else
        puts( "wdCreate successfully created Watchdog 2." );

    /************************************************************************
    **  WatchDog Timer Start Test
    ************************************************************************/
    puts( "\n.......... Next we start Watchdog 1 to time out after 1/2 second.");
    puts( "           Watchdog 1's timeout function will restart the timer" );
    puts( "           for another half second each time it expires." );
    puts( "           Then we start Watchdog 2 with a single 1 second delay." );

    puts( "Starting Watchdog 1" );
    errno = 0;
    err = wdStart( wdog1_id, 50, t_500_msec, 0 );
    if ( err == ERROR )
         printf( " returned error %x\r\n", errno );

    puts( "Starting Watchdog 2" );
    errno = 0;
    err = wdStart( wdog2_id, 25, t_1000_msec, 0 );
    if ( err == ERROR )
         printf( " returned error %x\r\n", errno );

    puts( "Task 1 sleeping for 2.5 seconds while timers run." );
    taskDelay( 250 );

    /************************************************************************
    **  WatchDog Timer Cancellation/Restart Test
    ************************************************************************/
    puts( "\n.......... Next we cancel Watchdog 1 and restart Watchdog 2" );
    puts( "           with a repeating 250 msec timeout function." );

    puts( "Cancelling Watchdog 1" );
    errno = 0;
    err = wdCancel( wdog1_id );
    if ( err == ERROR )
         printf( " returned error %x\r\n", errno );

    puts( "Restarting Watchdog 2" );
    errno = 0;
    err = wdStart( wdog2_id, 25, t_250_msec, 0 );
    if ( err == ERROR )
         printf( " returned error %x\r\n", errno );

    puts( "Task 1 sleeping for 1.5 seconds while timers run." );
    taskDelay( 150 );

    /************************************************************************
    **  WatchDog Timer Deletion Test
    ************************************************************************/
    puts( "\n.......... Finally we delete both Watchdog 1 and Watchdog 2" );

    puts( "Deleting Watchdog 1" );
    errno = 0;
    err = wdDelete( wdog1_id );
    if ( err == ERROR )
         printf( " returned error %x\r\n", errno );

    puts( "Deleting Watchdog 2" );
    errno = 0;
    err = wdDelete( wdog2_id );
    if ( err == ERROR )
         printf( " returned error %x\r\n", errno );

    puts( "Task 1 sleeping for 1 second to show timers not running." );
    taskDelay( 100 );
}

/*****************************************************************************
**  task10
*****************************************************************************/
int task10( int dummy0, int dummy1, int dummy2, int dummy3, int dummy4,
            int dummy5, int dummy6, int dummy7, int dummy8, int dummy9 )
{
    STATUS err;
    int i;

    for ( i = 0; i < 10; i++ )
    {
        taskDelay( 50 );
        puts( "\nTask 10 Not Suspended." );
    }

    /************************************************************************
    **  First wait on empty enable1 in pre-determined task order to test
    **  semaphore flush task waking order.
    ************************************************************************/
    puts( "\nTask 10 waiting on enable10 to begin acquiring token from enable1" );
    errno = 0;
    err = semTake( enable10, WAIT_FOREVER );
    if ( err == ERROR )
         printf( " returned error %x\r\n", errno );

    /*
    **  Consume one token from enable1.
    */
    puts( "\nTask 10 waiting indefinitely to acquire token from enable1" );
    errno = 0;
    if ( (err = semTake( enable1, WAIT_FOREVER )) == OK )
        printf( "\r\nTask 10 acquired token from enable1\r\n" );
    else
        printf( "\nTask 10 semTake on enable1 returned error %x\r\n", errno );

    puts( "Signalling complt10 to Task 1 - Task 10 ready to test SEM_Q_FIFO." );
    errno = 0;
    err = semGive( complt10 );
    if ( err == ERROR )
    {
        printf( " returned error %x", errno );
    }

    /************************************************************************
    **  Next wait on empty SEM1 in pre-determined task order to test
    **  task wait-queueing order ( FIFO vs. PRIORITY ).
    ************************************************************************/
    puts( "\nTask 10 waiting on enable10 to begin acquiring token from SEM1" );
    errno = 0;
    err = semTake( enable10, WAIT_FOREVER );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }

    puts( "Task 10 signalling complt10 to Task 1 to indicate Task 10 ready." );
    errno = 0;
    err = semGive( complt10 );
    if ( err == ERROR )
    {
        printf( " returned error %x", errno );
    }

    /*
    **  Consume one token from SEM1.
    */
    puts( "\nTask 10 waiting indefinitely to acquire token from SEM1" );
    errno = 0;
    if ( (errno = semTake( sema41_id, WAIT_FOREVER )) == OK )
        printf( "\r\nTask 10 acquired token from SEM1\r\n" );
    else
        printf( "\nTask 10 semTake on SEM1 returned error %x\r\n", errno );

    /************************************************************************
    **  Next wait on SEM2 to demonstrate semTake without wait.
    ************************************************************************/
    puts( "Signalling complt10 to Task 1 - Task 10 ready to test NO_WAIT." );
    errno = 0;
    err = semGive( complt10 );
    if ( err == ERROR )
    {
        printf( " returned error %x", errno );
    }

    puts( "\nTask 10 waiting on enable10 to begin acquiring token from SEM2" );
    errno = 0;
    err = semTake( enable10, WAIT_FOREVER );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }

    /*
    **  Consume a token from SEM2 without waiting.
    */
    puts( "\nTask 10 attempting to acquire token from SEM2 without waiting." );
    errno = 0;
    if ( (err = semTake( sema42_id, NO_WAIT )) == OK )
        printf( "\r\nTask 10 acquired token from SEM2\r\n" );
    else
        printf( "\nTask 10 semTake on SEM2 returned error %x\r\n", errno );
    puts( "Signalling complt10 to Task 1 - Task 10 ready to test SEM_Q_PRIORITY." );
    errno = 0;
    err = semGive( complt10 );
    if ( err == ERROR )
    {
        printf( " returned error %x @ %p\r\n", errno, &errno );
    }

    /************************************************************************
    **  Next wait on SEM3 in pre-determined task order to test
    **  task wait-queueing order ( FIFO vs. PRIORITY ).
    ************************************************************************/
    puts( "\nTask 10 waiting on enable10 to begin acquiring token from SEM3" );
    errno = 0;
    err = semTake( enable10, WAIT_FOREVER );
    if ( err == ERROR )
         printf( " returned error %x\r\n", errno );

    puts( "Task 10 signalling complt10 to Task 1 to indicate Task 10 ready." );
    errno = 0;
    err = semGive( complt10 );
    if ( err == ERROR )
    {
        printf( " returned error %x", errno );
    }

    /*
    **  Consume one token from SEM3.
    */
    puts( "\nTask 10 waiting up to 1 second to acquire token from SEM3" );
    errno = 0;
    if ( (err = semTake( sema43_id, 100 )) == OK )
        printf( "\r\nTask 10 acquired token from SEM3\r\n" );
    else
        printf( "\nTask 10 semTake on SEM3 returned error %x\r\n", errno );

    puts( "Signalling complt10 to Task 1 - Task 10 ready for semDelete test." );
    errno = 0;
    err = semGive( complt10 );
    if ( err == ERROR )
    {
        printf( " returned error %x", errno );
    }

    /*
    **  Consume one token from SEM1.
    */
    puts( "\nTask 10 waiting indefinitely to acquire token from SEM1" );
    errno = 0;
    if ( (err = semTake( sema41_id, WAIT_FOREVER )) == OK )
        printf( "\r\nTask 10 acquired token from SEM1\r\n" );
    else
        printf( "\nTask 10 semTake on SEM1 returned error %x\r\n", errno );

    puts( "Signalling complt10 to Task 1 - Task 10 finished semDelete test." );
    errno = 0;
    err = semGive( complt10 );
    if ( err == ERROR )
    {
        printf( " returned error %x", errno );
    }

    /*
    **  Tests all done... delete our own task.
    */
    puts( "\n.......... Task 10 deleting itself." );
    errno = 0;
    err = taskDelete( 0 );

    return( 0 );
}

/*****************************************************************************
**  task9
*****************************************************************************/
int task9( int dummy0, int dummy1, int dummy2, int dummy3, int dummy4,
           int dummy5, int dummy6, int dummy7, int dummy8, int dummy9 )
{
    STATUS err;
    msgblk_t msg;
    int i;

    /************************************************************************
    **  First wait on empty MSQ1 in pre-determined task order to test
    **  task wait-queueing order ( FIFO vs. PRIORITY ).
    ************************************************************************/
    puts( "\nTask 9 waiting on enable9 to begin receive on MSQ1" );
    errno = 0;
    err = semTake( enable9, WAIT_FOREVER );
    if ( err == ERROR )
         printf( " returned error %x\r\n", errno );

    /*
    **  Consume 3 messages from MSQ1.
    */
    puts( "\nTask 9 waiting indefinitely to receive 3 msgs on MSQ1" );
    for ( i = 0; i < 3; i++ )
    {
        errno = 0;
        err = msgQReceive( queue1_id, msg.blk, 16, WAIT_FOREVER );
        if ( err != ERROR )
        {
            printf( "\r\nTask 9 rcvd Test Cycle %d Msg No. %d from %s\r\n",
                    msg.msg.t_cycle, msg.msg.msg_no, msg.msg.qname );
        }
        else
        {
            printf( "\nTask 9 msgQReceive on MSQ1 returned error %x\r\n",
                    errno );
            break;
        }
    }
    puts( "Signalling complt9 to Task 1 - Task 9 finished queuing order test." );
    errno = 0;
    err = semGive( complt9 );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }

    /************************************************************************
    **  Next wait on empty MSQ3 to demonstrate q_send to zero-length queue.
    ************************************************************************/
    puts( "\nTask 9 waiting on enable9 to begin receive on MSQ3" );
    errno = 0;
    err = semTake( enable9, WAIT_FOREVER );
    if ( err == ERROR )
         printf( " returned error %x\r\n", errno );

    puts( "Task 9 signalling complt9 to Task 1 to indicate Task 9 ready." );
    errno = 0;
    err = semGive( complt9 );
    if ( err == ERROR )
    {
        printf( " returned error %x", errno );
    }

    /*
    **  Consume messages until 1 second elapses without an available message.
    */
    puts( "\nTask 9 waiting up to 1 sec to receive msgs on MSQ3" );
    while( 1 )
    {
        errno = 0;
        err = msgQReceive( queue3_id, msg.blk, 16, 100 );
        if ( err != ERROR )
        {
            printf( "\r\nTask 9 rcvd Test Cycle %d Msg No. %d from %s\r\n",
                    msg.msg.t_cycle, msg.msg.msg_no, msg.msg.qname );
        }
        else
        {
            printf( "\nTask 9 msgQReceive on MSQ3 returned error %x\r\n",
                    errno );
            break;
        }
    }
    puts( "Signalling complt9 to Task 1 - Task 9 finished zero-length test." );
    errno = 0;
    err = semGive( complt9 );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }

    /************************************************************************
    **  Now wait along with other tasks on empty MSQ1 to demonstrate
    **  queue delete behavior.
    ************************************************************************/
    puts( "\nTask 9 waiting on enable9 to begin receive on MSQ1" );
    errno = 0;
    err = semTake( enable9, WAIT_FOREVER );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }

    puts( "Task 9 signalling complt9 to Task 1 to indicate Task 9 ready." );
    errno = 0;
    err = semGive( complt9 );
    if ( err == ERROR )
    {
        printf( " returned error %x", errno );
    }

    /*
    **  Consume messages until 1 second elapses without an available message.
    */
    puts( "\nTask 9 waiting up to 1 sec to receive msgs on MSQ1" );
    while( 1 )
    {
        errno = 0;
        err = msgQReceive( queue1_id, msg.blk, 16, 100 );
        if ( err != ERROR )
        {
            printf( "\r\nTask 9 rcvd Test Cycle %d Msg No. %d from %s\r\n",
                    msg.msg.t_cycle, msg.msg.msg_no, msg.msg.qname );
        }
        else
        {
            printf( "\nTask 9 msgQReceive on MSQ1 returned error %x\r\n",
                    errno );
            break;
        }
    }
    puts( "Signalling complt9 to Task 1 - Task 9 finished msgQDelete test." );
    errno = 0;
    err = semGive( complt9 );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }

    /*
    **  Tests all done... delete our own task.
    */
    puts( "\n.......... Task 9 deleting itself." );
    errno = 0;
    err = taskDelete( 0 );

    return( 0 );
}

/*****************************************************************************
**  task8
*****************************************************************************/
int task8( int dummy0, int dummy1, int dummy2, int dummy3, int dummy4,
           int dummy5, int dummy6, int dummy7, int dummy8, int dummy9 )
{
    STATUS err;

    /************************************************************************
    **  First wait on empty MSQ1 in pre-determined task order to test
    **  task wait-queueing order ( FIFO vs. PRIORITY ).
    ************************************************************************/
    puts( "\nTask 8 waiting on enable8 to begin testing Watchdog 2" );

    while ( 1 )
    {
        errno = 0;
        err = semTake( enable8, WAIT_FOREVER );
        if ( err == OK )
        {
             printf( "\r\nWatchdog 2 count %d msec", wdog2_cycle );
        }
        else
        {
             printf( " returned error %x\r\n", errno );
        }
    }

    return( 0 );
}

/*****************************************************************************
**  task7
*****************************************************************************/
int task7( int dummy0, int dummy1, int dummy2, int dummy3, int dummy4,
           int dummy5, int dummy6, int dummy7, int dummy8, int dummy9 )
{
    STATUS err;

    /************************************************************************
    **  First wait on empty enable1 in pre-determined task order to test
    **  semaphore flush task waking order.
    ************************************************************************/
    puts( "\nTask 7 waiting on enable7 to begin acquiring token from enable1" );
    errno = 0;
    err = semTake( enable7, WAIT_FOREVER );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }

    /*
    **  Consume one token from enable1.
    */
    puts( "\nTask 7 waiting indefinitely to acquire token from enable1" );
    errno = 0;
    if ( (err = semTake( enable1, WAIT_FOREVER )) == OK )
        printf( "\r\nTask 7 acquired token from enable1\r\n" );
    else
        printf( "\nTask 7 semTake on enable1 returned error %x\r\n", errno );

    puts( "Signalling complt7 to Task 1 - Task 7 ready to test SEM_Q_FIFO." );
    errno = 0;
    err = semGive( complt7 );
    if ( err == ERROR )
    {
        printf( " returned error %x", errno );
    }

    /************************************************************************
    **  Next wait on empty SEM1 in pre-determined task order to test
    **  task wait-queueing order ( FIFO vs. PRIORITY ).
    ************************************************************************/
    puts( "\nTask 7 waiting on enable7 to begin acquiring token from SEM1" );
    errno = 0;
    err = semTake( enable7, WAIT_FOREVER );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }

    puts( "Task 7 signalling complt7 to Task 1 to indicate Task 7 ready." );
    errno = 0;
    err = semGive( complt7 );
    if ( err == ERROR )
    {
        printf( " returned error %x", errno );
    }

    /*
    **  Consume one token from SEM1.
    */
    puts( "\nTask 7 waiting indefinitely to acquire token from SEM1" );
    errno = 0;
    if ( (err = semTake( sema41_id, WAIT_FOREVER )) == OK )
        printf( "\r\nTask 7 acquired token from SEM1\r\n" );
    else
        printf( "\nTask 7 semTake on SEM1 returned error %x\r\n", errno );

    /************************************************************************
    **  Next wait on SEM2 to demonstrate semTake without wait.
    ************************************************************************/
    puts( "Signalling complt7 to Task 1 - Task 7 ready to test NO_WAIT." );
    errno = 0;
    err = semGive( complt7 );
    if ( err == ERROR )
    {
        printf( " returned error %x", errno );
    }

    puts( "\nTask 7 waiting on enable7 to begin acquiring token from SEM2" );
    errno = 0;
    err = semTake( enable7, WAIT_FOREVER );
    if ( err == ERROR )
    { 
        printf( " returned error %x\r\n", errno );
    }

    /*
    **  Consume a token from SEM2 without waiting.
    */
    puts( "\nTask 7 attempting to acquire token from SEM2 without waiting." );
    errno = 0;
    if ( (err = semTake( sema42_id, NO_WAIT )) == OK )
        printf( "\r\nTask 7 acquired token from SEM2\r\n" );
    else
        printf( "\nTask 7 semTake on SEM2 returned error %x\r\n", errno );
    puts( "Signalling complt7 to Task 1 - Task 7 ready to test SEM_Q_PRIORITY." );
    errno = 0;
    err = semGive( complt7 );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }

    /************************************************************************
    **  Next wait on SEM3 in pre-determined task order to test priority-based
    **  task wait-queueing order ( FIFO vs. PRIORITY ).
    ************************************************************************/
    puts( "\nTask 7 waiting on enable7 to begin acquiring token from SEM3" );
    errno = 0;
    err = semTake( enable7, WAIT_FOREVER );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }

    puts( "Task 7 signalling complt7 to Task 1 to indicate Task 7 ready." );
    errno = 0;
    err = semGive( complt7 );
    if ( err == ERROR )
    {
        printf( " returned error %x", errno );
    }

    /*
    **  Consume one token from SEM3.
    */
    puts( "\nTask 7 waiting up to 1 second to acquire token from SEM3" );
    if ( (err = semTake( sema43_id, 100 )) == OK )
        printf( "\r\nTask 7 acquired token from SEM3\r\n" );
    else
        printf( "\nTask 7 semTake on SEM3 returned error %x\r\n", errno );

    puts( "Signalling complt7 to Task 1 - Task 7 ready for semDelete test." );
    errno = 0;
    err = semGive( complt7 );
    if ( err == ERROR )
    {
        printf( " returned error %x", errno );
    }

    /*
    **  Consume one token from SEM1.
    */
    puts( "\nTask 7 waiting indefinitely to acquire token from SEM1" );
    errno = 0;
    if ( (err = semTake( sema41_id, WAIT_FOREVER )) == OK )
        printf( "\r\nTask 7 acquired token from SEM1\r\n" );
    else
        printf( "\nTask 7 semTake on SEM1 returned error %x\r\n", errno );

    puts( "Signalling complt7 to Task 1 - Task 7 finished semDelete test." );
    errno = 0;
    err = semGive( complt7 );
    if ( err == ERROR )
    {
        printf( " returned error %x", errno );
    }

    /*
    **  Tests all done... delete our own task.
    */
    puts( "\n.......... Task 7 deleting itself." );
    errno = 0;
    err = taskDelete( 0 );

    return( 0 );
}

/*****************************************************************************
**  task6
*****************************************************************************/
int task6( int dummy0, int dummy1, int dummy2, int dummy3, int dummy4,
           int dummy5, int dummy6, int dummy7, int dummy8, int dummy9 )
{
    STATUS err;
    msgblk_t msg;
    union
    {
        char     blk[128];
        my_qmsg_t msg;
    } bigmsg;
    int i;

    /************************************************************************
    **  First wait on empty MSQ1 in pre-determined task order to test
    **  task wait-queueing order ( FIFO vs. PRIORITY ).
    ************************************************************************/
    puts( "\nTask 6 waiting on enable6 to begin receive on MSQ1" );
    errno = 0;
    err = semTake( enable6, WAIT_FOREVER );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }

    /*
    **  Consume 3 messages from MSQ1.
    */
    puts( "\nTask 6 waiting indefinitely to receive 3 msgs on MSQ1" );
    for ( i = 0; i < 3; i++ )
    {
        errno = 0;
        err = msgQReceive( queue1_id, msg.blk, 16, WAIT_FOREVER );
        if ( err != ERROR )
        {
            printf( "\r\nTask 6 rcvd Test Cycle %d Msg No. %d from %s\r\n",
                    msg.msg.t_cycle, msg.msg.msg_no, msg.msg.qname );
        }
        else
        {
            printf( "\nTask 6 msgQReceive on MSQ1 returned error %x\r\n",
                    errno );
            break;
        }
    }
    puts( "Signalling complt6 to Task 1 - Task 6 finished queuing order test." );
    errno = 0;
    err = semGive( complt6 );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }

    /************************************************************************
    **  Now wait along with other tasks on empty MSQ1 to demonstrate
    **  queue delete behavior.
    ************************************************************************/
    puts( "\nTask 6 waiting on enable6 to begin receive on MSQ1" );
    errno = 0;
    err = semTake( enable6, WAIT_FOREVER );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }

    puts( "Task 6 signalling complt6 to Task 1 to indicate Task 6 ready." );
    errno = 0;
    err = semGive( complt6 );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }

    /*
    **  Consume messages until 1 second elapses without an available message.
    */
    puts( "\nTask 6 waiting up to 1 sec to receive msgs on MSQ1" );
    while( 1 )
    {
        errno = 0;
        err = msgQReceive( queue1_id, msg.blk, 16, 100 );
        if ( err != ERROR )
        {
            printf( "\r\nTask 6 rcvd Test Cycle %d Msg No. %d from %s\r\n",
                    msg.msg.t_cycle, msg.msg.msg_no, msg.msg.qname );
        }
        else
        {
            printf( "\nTask 6 msgQReceive on MSQ1 returned error %x\r\n",
                    errno );
            break;
        }
    }
    puts( "Signalling complt6 to Task 1 - Task 6 finished msgQDelete test." );
    errno = 0;
    err = semGive( complt6 );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }

    /************************************************************************
    **  Now send a message to MSQ3 to demonstrate wait-to-send behavior
    **  during queue deletion.
    ************************************************************************/
    puts( "\nTask 6 waiting on enable6 to send msg to MSQ3" );
    errno = 0;
    err = semTake( enable6, WAIT_FOREVER );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }

    puts( "Task 6 signalling complt6 to Task 1 to indicate Task 6 ready." );
    errno = 0;
    err = semGive( complt6 );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }

    msg.msg.msg_no = 13;
    msg.msg.qname[3] = '3';
    printf( "\r\nTask 6 waiting indefinitely to send msg 13 to MSQ3\r\n" );
    errno = 0;
    err = msgQSend( queue3_id, msg.blk, 16, WAIT_FOREVER, MSG_PRI_NORMAL );
    if ( err == ERROR )
    printf( "\r\nTask 6 msgQSend of msg 13 to MSQ3 returned error %x\r\n",
            errno );
    else
        printf( "\r\n" );

    puts( "Signalling complt6 to Task 1 - Task 6 finished msgQSend test." );
    errno = 0;
    err = semGive( complt6 );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }

    /************************************************************************
    **  Now wait for messages on MSQ2 to demonstrate urgent message send
    **  behavior.
    ************************************************************************/
    puts( "\nTask 6 waiting on enable6 to begin receive on MSQ2" );
    errno = 0;
    err = semTake( enable6, WAIT_FOREVER );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }

    puts( "Task 6 signalling complt6 to Task 1 to indicate Task 6 ready." );
    errno = 0;
    err = semGive( complt6 );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }

    /*
    **  Consume messages until no available messages remain.
    */
    puts( "\nTask 6 receiving msgs without waiting on MSQ2" );
    while( 1 )
    {
        errno = 0;
        err = msgQReceive( queue2_id, bigmsg.blk, 128, NO_WAIT );
        if ( err != ERROR )
        {
            printf( "\r\nTask 6 rcvd Test Cycle %d Msg No. %d from %s\r\n",
                    bigmsg.msg.t_cycle, bigmsg.msg.msg_no, bigmsg.msg.qname );
        }
        else
        {
            printf( "\nTask 6 msgQReceive on MSQ2 returned error %x\r\n",
                    errno );
            break;
        }
    }
    puts( "Signalling complt6 to Task 1 - Task 6 finished MSG_PRI_URGENT test." );
    errno = 0;
    err = semGive( complt6 );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }

    /*
    **  Tests all done... delete our own task.
    */
    puts( "\n.......... Task 6 deleting itself." );
    errno = 0;
    err = taskDelete( 0 );

    return( 0 );
}

/*****************************************************************************
**  task5
*****************************************************************************/
int task5( int dummy0, int dummy1, int dummy2, int dummy3, int dummy4,
           int dummy5, int dummy6, int dummy7, int dummy8, int dummy9 )
{
    STATUS err;
    int i;
    v2pthread_cb_t *tcb;

    while ( !task5_restarted )
        taskDelay( 1 );

    tcb = tcb_for( task5_id );

    if ( task5_restarted == (unsigned char)1 )
    {
        task5_restarted++;

        puts( "\nTask 5 calling taskRestart to restart itself" );
        errno = 0;
        err = taskRestart( 0 );
        if ( err == ERROR )
        {
            printf( " returned error %x\r\n", errno );
        }
    }
    else
        puts( "\nTask 5 restarted itself." );

    for ( i = 0; 1; i += 10 )
    {
        /* 
        printf( "\r\npthrid %ld %d ms", tcb->pthrid, i );
        if ( tcb->pthrid  > 0L )
            break;
        */
        usleep( 10000L );
            break;
    }
    /************************************************************************
    **  First wait on empty MSQ1 in pre-determined task order to test
    **  task wait-queueing order ( FIFO vs. PRIORITY ).
    ************************************************************************/
    puts( "\nTask 5 waiting on enable5 to begin testing Watchdog 1" );

    while ( 1 )
    {
        errno = 0;
        err = semTake( enable5, WAIT_FOREVER );
        if ( err == OK )
        {
             printf( "\r\nWatchdog 1 count %d msec", wdog1_cycle );
        }
        else
        {
             printf( " returned error %x\r\n", errno );
        }
    }

    return( 0 );
}

/*****************************************************************************
**  task4
*****************************************************************************/
int task4( int dummy0, int dummy1, int dummy2, int dummy3, int dummy4,
           int dummy5, int dummy6, int dummy7, int dummy8, int dummy9 )
{
    STATUS err;

    /************************************************************************
    **  First wait on empty enable1 in pre-determined task order to test
    **  semaphore flush task waking order.
    ************************************************************************/
    puts( "\nTask 4 waiting on enable4 to begin acquiring token from enable1" );
    errno = 0;
    err = semTake( enable4, WAIT_FOREVER );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }

    /*
    **  Consume one token from enable1.
    */
    puts( "\nTask 4 waiting indefinitely to acquire token from enable1" );
    errno = 0;
    if ( (err = semTake( enable1, WAIT_FOREVER )) == OK )
        printf( "\r\nTask 4 acquired token from enable1\r\n" );
    else
        printf( "\nTask 4 semTake on enable1 returned error %x\r\n", errno );

    puts( "Signalling complt4 to Task 1 - Task 4 ready to test SEM_Q_FIFO." );
    errno = 0;
    err = semGive( complt4 );
    if ( err == ERROR )
    {
        printf( " returned error %x", errno );
    }

    /************************************************************************
    **  Next wait on empty SEM1 in pre-determined task order to test
    **  task wait-queueing order ( FIFO vs. PRIORITY ).
    ************************************************************************/
    puts( "\nTask 4 waiting on enable4 to begin acquiring token from SEM1" );
    errno = 0;
    err = semTake( enable4, WAIT_FOREVER );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }

    puts( "Task 4 signalling complt4 to Task 1 to indicate Task 4 ready." );
    errno = 0;
    err = semGive( complt4 );
    if ( err == ERROR )
    {
        printf( " returned error %x", errno );
    }

    /*
    **  Consume one token from SEM1.
    */
    puts( "\nTask 4 waiting indefinitely to acquire token from SEM1" );
    errno = 0;
    if ( (err = semTake( sema41_id, WAIT_FOREVER )) == OK )
        printf( "\r\nTask 4 acquired token from SEM1\r\n" );
    else
        printf( "\nTask 4 semTake on SEM1 returned error %x\r\n", errno );

    /************************************************************************
    **  Next wait on SEM2 to demonstrate semTake without wait.
    ************************************************************************/
    puts( "Signalling complt4 to Task 1 - Task 4 ready to test NO_WAIT." );
    errno = 0;
    err = semGive( complt4 );
    if ( err == ERROR )
    {
        printf( " returned error %x", errno );
    }

    puts( "\nTask 4 waiting on enable4 to begin acquiring token from SEM2" );
    errno = 0;
    err = semTake( enable4, WAIT_FOREVER );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }

    /*
    **  Consume a token from SEM2 without waiting.
    */
    puts( "\nTask 4 attempting to acquire token from SEM2 without waiting." );
    errno = 0;
    if ( (err = semTake( sema42_id, NO_WAIT )) == OK )
        printf( "\r\nTask 4 acquired token from SEM2\r\n" );
    else
        printf( "\nTask 4 semTake on SEM2 returned error %x\r\n", errno );
    puts( "Signalling complt4 to Task 1 - Task 4 ready to test SEM_Q_PRIORITY." );
    errno = 0;
    err = semGive( complt4 );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }

    /************************************************************************
    **  Next wait on SEM3 in pre-determined task order to test
    **  task wait-queueing order ( FIFO vs. PRIORITY ).
    ************************************************************************/
    puts( "\nTask 4 waiting on enable4 to begin acquiring token from SEM3" );
    errno = 0;
    err = semTake( enable4, WAIT_FOREVER );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }

    puts( "Task 4 signalling complt4 to Task 1 to indicate Task 4 ready." );
    errno = 0;
    err = semGive( complt4 );
    if ( err == ERROR )
    {
        printf( " returned error %x", errno );
    }

    /*
    **  Consume one token from SEM3.
    */
    puts( "\nTask 4 waiting up to 1 second to acquire token from SEM3" );
    errno = 0;
    if ( (err = semTake( sema43_id, 100 )) == OK )
        printf( "\r\nTask 4 acquired token from SEM3\r\n" );
    else
        printf( "\nTask 4 semTake on SEM3 returned error %x\r\n", errno );

    puts( "Signalling complt4 to Task 1 - Task 4 ready for semDelete test." );
    errno = 0;
    err = semGive( complt4 );
    if ( err == ERROR )
    {
        printf( " returned error %x", errno );
    }

    /*
    **  Consume one token from SEM1.
    */
    puts( "\nTask 4 waiting indefinitely to acquire token from SEM1" );
    errno = 0;
    if ( (err = semTake( sema41_id, WAIT_FOREVER )) == OK )
        printf( "\r\nTask 4 acquired token from SEM1\r\n" );
    else
        printf( "\nTask 4 semTake on SEM1 returned error %x\r\n", errno );

    puts( "Signalling complt4 to Task 1 - Task 4 finished semDelete test." );
    errno = 0;
    err = semGive( complt4 );
    if ( err == ERROR )
    {
        printf( " returned error %x", errno );
    }

    /*
    **  Tests all done... delete our own task.
    */
    puts( "\n.......... Task 4 deleting itself." );
    errno = 0;
    err = taskDelete( 0 );

    return( 0 );
}


/*****************************************************************************
**  task3
*****************************************************************************/
int task3( int dummy0, int dummy1, int dummy2, int dummy3, int dummy4,
           int dummy5, int dummy6, int dummy7, int dummy8, int dummy9 )
{
    STATUS err;
    msgblk_t msg;
    union
    {
        char     blk[128];
        my_qmsg_t msg;
    } bigmsg;
    int i;

    /************************************************************************
    **  First wait on empty MSQ1 in pre-determined task order to test
    **  task wait-queueing order ( FIFO vs. PRIORITY ).
    ************************************************************************/
    puts( "\nTask 3 waiting on enable3 to begin receive on MSQ1" );
    errno = 0;
    err = semTake( enable3, WAIT_FOREVER );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }

    /*
    **  Consume 3 messages from MSQ1.
    */
    puts( "\nTask 3 waiting indefinitely to receive 3 msgs on MSQ1" );
    for ( i = 0; i < 3; i++ )
    {
        errno = 0;
        err = msgQReceive( queue1_id, msg.blk, 16, WAIT_FOREVER );
        if ( err != ERROR )
        {
            printf( "\r\nTask 3 rcvd Test Cycle %d Msg No. %d from %s\r\n",
                    msg.msg.t_cycle, msg.msg.msg_no, msg.msg.qname );
        }
        else
        {
            printf( "\nTask 3 msgQReceive on MSQ1 returned error %x\r\n",
                    errno );
            break;
        }
    }
    puts( "Signalling complt3 to Task 1 - Task 3 finished queuing order test." );
    errno = 0;
    err = semGive( complt3 );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }

    /************************************************************************
    **  Now wait along with other tasks on empty MSQ1 to demonstrate
    **  queue delete behavior.
    ************************************************************************/
    puts( "\nTask 3 waiting on enable3 to begin receive on MSQ1" );
    errno = 0;
    err = semTake( enable3, WAIT_FOREVER );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }

    puts( "Task 3 signalling complt3 to Task 1 to indicate Task 3 ready." );
    errno = 0;
    err = semGive( complt3 );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }

    /*
    **  Consume messages until 1 second elapses without an available message.
    */
    puts( "\nTask 3 waiting up to 1 sec to receive msgs on MSQ1" );
    while( 1 )
    {
        errno = 0;
        err = msgQReceive( queue1_id, msg.blk, 16, 100 );
        if ( err != ERROR )
        {
            printf( "\r\nTask 3 rcvd Test Cycle %d Msg No. %d from %s\r\n",
                    msg.msg.t_cycle, msg.msg.msg_no, msg.msg.qname );
        }
        else
        {
            printf( "\nTask 3 msgQReceive on MSQ1 returned error %x\r\n",
                    errno );
            break;
        }
    }
    puts( "Signalling complt3 to Task 1 - Task 3 finished msgQDelete test." );
    errno = 0;
    err = semGive( complt3 );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }

    /************************************************************************
    **  Now wait along with other tasks on empty MSQ1 to demonstrate
    **  queue delete behavior.
    ************************************************************************/
    puts( "\nTask 3 waiting on enable3 to begin receive on MSQ2" );
    errno = 0;
    err = semTake( enable3, WAIT_FOREVER );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }

    puts( "Task 3 signalling complt3 to Task 1 to indicate Task 3 ready." );
    errno = 0;
    err = semGive( complt3 );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }

    /*
    **  Delay 1 sec before consuming message.
    */
    puts( "\r\nTask 3 delaying one second before consuming MSQ2 message." );
    taskDelay( 100 );

    /*
    **  Consume messages until 1 second elapses without an available message.
    */
    puts( "\nTask 3 waiting up to 1 sec to receive 1 msg on MSQ2" );
    errno = 0;
    err = msgQReceive( queue2_id, bigmsg.blk, 128, 100 );
    if ( err != ERROR )
    {
        printf( "\r\nTask 3 rcvd Test Cycle %d Msg No. %d from %s\r\n",
                bigmsg.msg.t_cycle, bigmsg.msg.msg_no, bigmsg.msg.qname );
    }
    else
        printf( "\nTask 3 msgQReceive on MSQ2 returned error %x\r\n", errno );

    puts( "Signalling complt3 to Task 1 - Task 3 finished Wait-to-Send test." );
    errno = 0;
    err = semGive( complt3 );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }

    /*
    **  Tests all done... delete our own task.
    */
    puts( "\n.......... Task 3 deleting itself." );
    errno = 0;
    err = taskDelete( 0 );

    return( 0 );
}


/*****************************************************************************
**  task2
*****************************************************************************/
int task2( int dummy0, int dummy1, int dummy2, int dummy3, int dummy4,
           int dummy5, int dummy6, int dummy7, int dummy8, int dummy9 )
{
    STATUS err;

    /************************************************************************
    **  First wait on empty MSQ1 in pre-determined task order to test
    **  task wait-queueing order ( FIFO vs. PRIORITY ).
    ************************************************************************/
    puts( "\nTask 2 waiting on enable2 to begin semGive (Not Owner) test" );
    errno = 0;
    err = semTake( enable2, WAIT_FOREVER );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }

    /*
    **  Attempt to unlock Mutex 1, which Task 2 does not currently own.
    */
    printf( "\r\nTask 2 attempting to unlock Mutex 1" );
    errno = 0;
    err = semGive( mutex1_id );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }
    puts( "Signalling complt2 to Task 1 - Task 2 finished semGive (Not Owner) test." );
    errno = 0;
    err = semGive( complt2 );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }

    /************************************************************************
    **  Now wait on enable2 to demonstrate mutex priority inversion protection
    **  behavior.
    ************************************************************************/
    puts( "\nTask 2 waiting on enable2 to begin priority inversion test" );
    errno = 0;
    err = semTake( enable2, WAIT_FOREVER );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }

    puts( "\nTask 2 acquiring ownership of Mutex 3" );
    errno = 0;
    err = semTake( mutex3_id, WAIT_FOREVER );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }

    display_tcb( task2_id );
    printf( "\r\n" );

    puts( "\nTask 2 signalling complt2 to Task 1 to indicate Mutex 3 acquired." );
    errno = 0;
    err = semGive( complt2 );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }

    puts( "\r\nTask 1 blocked... task 2 priority boosted as shown." );
    display_tcb( task2_id );
    printf( "\r\n" );

    taskDelay( 20 );

    puts( "\nTask 2 unlocking Mutex 3" );
    errno = 0;
    err = semGive( mutex3_id );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }

    taskDelay( 2 );

    puts( "\r\nTask 1 priority restored as shown after releasing Mutex 3." );
    display_tcb( task2_id );
    printf( "\r\n" );

    puts( "\nSignalling complt2 to Task 1 - Task 2 finished priority inversion test." );
    errno = 0;
    err = semGive( complt2 );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }

    /************************************************************************
    **  Now wait on enable2 to demonstrate mutex automatic deletion safety
    **  behavior.
    ************************************************************************/
    puts( "\nTask 2 waiting on enable2 to begin auto deletion safety test" );
    errno = 0;
    err = semTake( enable2, WAIT_FOREVER );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }

    puts( "\nTask 2 acquiring ownership of Mutex 2" );
    errno = 0;
    err = semTake( mutex2_id, WAIT_FOREVER );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }

    puts( "\nTask 2 signalling complt2 to Task 1 to indicate Mutex 2 acquired." );
    errno = 0;
    err = semGive( complt2 );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }

    puts( "\r\nTask 1 blocked... task 2 not deletable while it owns Mutex 2." );

    puts( "\nTask 2 unlocking Mutex 2 and becoming deletable again" );
    errno = 0;
    err = semGive( mutex2_id );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }

    taskDelay( 200 );

    return( 0 );
}

/*****************************************************************************
**  temp_task
*****************************************************************************/
int temp_task( int dummy0, int dummy1, int dummy2, int dummy3, int dummy4,
           int dummy5, int dummy6, int dummy7, int dummy8, int dummy9 )
{
    int i;
    STATUS err;

    puts( "\n.......... Call taskIdSelf to get Temporary Task's ID." );
    puts( "\nTemp Task calling taskIdSelf to get task ID" );
    temp_taskid = taskIdSelf();

    puts( "\nTemp Task waiting on enable1 to begin task deletion test" );
    errno = 0;
    err = semTake( enable1, WAIT_FOREVER );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }

    puts( "\nTemp Task calling taskSafe() to prevent task deletion." );
    taskSafe();

    puts( "Signalling complt1 to Task 1 - Temp Task ready to test deletion control" );
    errno = 0;
    err = semGive( complt1 );
    if ( err == ERROR )
        printf( " returned error %x", errno );

    for ( i = 0; i < 3; i++ )
    {
        puts( "\nTemp Task Not Deleted." );
        taskDelay( 50 );
    }

    puts( "\nTemp Task calling taskUnsafe() to allow task deletion." );
    taskUnsafe();

    while ( 1 )
    {
        taskDelay( 20 );
    }
}

/*****************************************************************************
**  validate_tasks
**         This function sequences through a series of actions to exercise
**         the various features and characteristics of v2pthreads tasks
**
*****************************************************************************/
void validate_tasks( void )
{
    STATUS err;
    int oldpriority;
    int my_taskid;
    int i, num_tasks;
    int tid_list[15];
    v2pthread_cb_t fixed_tcb;
    v2pthread_cb_t *temp_tcb;

    puts( "\r\n********** Task validation:" );

    /************************************************************************
    **  Task Creation/Start and Preemptibility Control Test
    ************************************************************************/
    puts( "\n.......... Next call taskLock to make Task 1 non-preemptible." );
    puts( "\r\nTask 1 going non-preemptible (locking scheduler)." );
    errno = 0;
    taskLock();
    display_tcb( task1_id );
    printf( "\r\n" );

    puts( "\n.......... Now start each of the consumer tasks." );

    puts( "Starting Task 2 at priority level 20" );
    errno = 0;
    task2_id = taskSpawn( "TSK2", 20, 0, 0, task2,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );

    puts( "Starting Task 3 at priority level 20" );
    errno = 0;
    task3_id = taskSpawn( "TSK3", 20, 0, 0, task3,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );

    puts( "Starting Task 4 at priority level 20" );
    errno = 0;
    task4_id = taskSpawn( "TSK4", 20, 0, 0, task4,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );

    puts( "Starting Task 5 at priority level 15" );
    errno = 0;
    task5_id = taskSpawn( "TSK5", 15, 0, 0, task5,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );

    puts( "Starting Task 6 at priority level 15" );
    errno = 0;
    task6_id = taskSpawn( "TSK6", 15, 0, 0, task6,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );

    puts( "Starting Task 7 at priority level 15" );
    errno = 0;
    task7_id = taskSpawn( "TSK7", 15, 0, 0, task7,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );

    puts( "Starting Task 8 at priority level 10" );
    errno = 0;
    task8_id = taskSpawn( "TSK8", 10, 0, 0, task8,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );

    puts( "Starting Task 9 at priority level 10" );
    errno = 0;
    task9_id = taskSpawn( "TSK9", 10, 0, 0, task9,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );

    puts( "Starting Task 10 at priority level 10" );
    errno = 0;
    task10_id = taskSpawn( "TSKA", 10, 0, 0, task10,
                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );

    puts( "\n.......... Next call taskUnlock to make Task 1 preemptible again." );
    puts( "\r\nTask 1 going preemptible (unlocking scheduler)." );
    err = taskUnlock();
    display_tcb( task1_id );
    printf( "\r\n" );

    puts( "\n.......... Then create a temporary task without starting it." );

    puts( "Creating Temporary Task at priority level 10" );
    /*
    temp_tcb = ts_malloc( sizeof( v2pthread_cb_t ) );
    */

    /*
    **  Use a statically allocated task control block for temporary task.
    */
    temp_tcb = &fixed_tcb;
    errno = 0;
    err = taskInit( temp_tcb, "TEMP_TASK", 10, 0, 0, 0, temp_task,
                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
    if ( err == ERROR )
    {
        printf( "\ntaskInit for TEMP_TASK returned error %x\r\n", errno );
    }
    else
    {
        puts( "\n.......... Next call taskName with a NULL taskid to check our task's name." );
        printf( "Our Task is named %s\r\n", taskName( 0 ) );

        puts( "\n.......... Call taskName to check Temporary Task's name." );
        printf( "Temporary Task is named %s\r\n", taskName( temp_tcb->taskid ) );

        puts( "\n.......... Next call taskIsReady to check task's status." );
        puts( "           The task should not be ready before is is started." );
        if ( taskIsReady( temp_tcb->taskid ) )
            puts( "taskIsReady indicates Temporary Task is ready." );
        else
            puts( "taskIsReady indicates Temporary Task not ready." );

        puts( "\n.......... Next call taskActivate to start the Temporary Task." );
        errno = 0;
        err = taskActivate( temp_tcb->taskid );
        if ( err == ERROR )
        {
            printf( "\ntaskActivate for TEMP_TASK returned error %x\r\n",
                    errno );
        }
    }

    /************************************************************************
    **  kernelTimeSlice() test
    ************************************************************************/
    puts( "\n.......... Next call kernelTimeSlice() to turn off timeslicing." );
    display_tcb( task5_id );
    printf( "\r\n" );

    puts( "Task 1 calling kernelTimeSlice() to turn timeslicing OFF.\r\n" );
    err = kernelTimeSlice( 0 );
    display_tcb( task5_id );
    printf( "\r\n" );

    puts( "Task 1 calling kernelTimeSlice() to turn timeslicing ON.\r\n" );
    err = kernelTimeSlice( 5 );
    display_tcb( task5_id );
    printf( "\r\n" );

    /************************************************************************
    **  Task Suspend and Resume Test
    ************************************************************************/
    puts( "Task 1 sleeping for 2 seconds to allow task 10 to run.\r\n" );
    taskDelay( 200 );
    puts( "\n.......... Next call taskSuspend to suspend Task 10." );
    puts( "Task 1 calling taskSuspend for task 10.\r\n" );
    errno = 0;
    err = taskSuspend( task10_id );
    if ( err == ERROR )
         printf( "\ntaskSuspend returned error %x\r\n", errno );
    puts( "Task 1 sleeping for 1.5 seconds to allow task 10 to run.\r\n" );
    puts( "           Since task 10 is printing a message every 1/2 second," );
    puts( "           the absence of messages demonstrates that the" );
    puts( "           suspension overrides timeouts, etc." );
    taskDelay( 150 );
    puts( "\n.......... Next call taskSuspend a second time to suspend Task 10." );
    puts( "           The second call should have no effect." );
    puts( "Task 1 calling taskSuspend for task 10.\r\n" );
    errno = 0;
    err = taskSuspend( task10_id );
   if ( err == ERROR )
    {
        printf( "\ntaskSuspend returned error %x\r\n", errno );
    }

    puts( "\n.......... Next call taskIsSuspended to check Task 10's status." );
    if ( taskIsSuspended( task10_id ) )
        puts( "taskIsSuspended indicates Task 10 is suspended." );
    else
        puts( "taskIsSuspended indicates Task 10 not suspended." );

    puts( "\n.......... Next call taskResume to make Task 10 runnable again." );
    puts( "Task 1 calling taskResume for task 10.\r\n" );
    errno = 0;
    err = taskResume( task10_id );
    if ( err == ERROR )
    {
        printf( "\ntaskResume returned error %x\r\n", errno );
    }

    puts( "\n.......... Next call taskIsSuspended to check Task 10's status." );
    if ( taskIsSuspended( task10_id ) )
        puts( "           taskIsSuspended indicates Task 10 is suspended." );
    else
        puts( "           taskIsSuspended indicates Task 10 not suspended." );
    puts( "\r\nTask 1 sleeping for 4 seconds to allow task 10 to run.\r\n" );
    taskDelay( 400 );

    /************************************************************************
    **  Task Deletion Control Test
    ************************************************************************/
    puts( "\n.......... Now test task deletion control." );
    puts( "           The temporary task will call taskSafe() to prevent" );
    puts( "           itself from being deleted, and will signal Task 1." );
    puts( "           Task 1 will then attempt to delete the task, and" );
    puts( "           Task 1 should block until the temporary task calls" );
    puts( "           taskUnsafe(), at which time the deletion will proceed." );

    puts( "Task 1 signalling Temporary Task to begin task deletion safety test.");
    errno = 0;
    err = semGive( enable1 );
    if ( err == ERROR )
    {
        printf( "semGive of enable1 returned error %x\r\n", errno );
    }

    puts( "Task 1 blocking for handshake from Temporary Task..." );
    err = semTake( complt1, WAIT_FOREVER );

    puts( "\n.......... Call taskIdVerify to verify ID of Temporary Task..." );
    puts( "           This call and subsequent calls use the task ID" );
    puts( "           returned earlier by Temp Task's call to taskIdSelf." );
    err = taskIdVerify( temp_taskid );
    if ( err == ERROR )
    {
        puts( "taskIdVerify indicates Temporary Task does not exist." );
    }
    else       
    {
        puts( "taskIdVerify indicates Temporary Task does exist." );
        puts( "\n.......... Call taskIsReady to verify it indicates task ready." );
        if ( taskIsReady( temp_taskid ) )
            puts( "taskIsReady indicates Temporary Task is ready." );
        else
            puts( "taskIsReady indicates Temporary Task not ready." );

        puts( "\n.......... Call taskDelete to delete Temporary Task..." );
        taskDelete( temp_taskid );
        puts( "\n.......... Call taskIdVerify to confirm Temporary Task deleted..." );
        err = taskIdVerify( temp_taskid );
        if ( err == ERROR )
            puts( "taskIdVerify indicates Temporary Task does not exist." );
        else
            puts( "taskIdVerify indicates Temporary Task still exists." );
    }
 
    puts( "\n.......... Now test forced task deletion." );
    puts( "           First recreate the temporary task just deleted." );
    puts( "           The temporary task will call taskSafe() to prevent" );
    puts( "           itself from being deleted, and will signal Task 1." );
    puts( "           Task 1 will then attempt to forcibly delete the task," );
    puts( "           and the deletion should proceed without waiting for" );
    puts( "           the temporary task to make a taskUnsafe call." );

    puts( "Starting New (Unnamed) Temporary Task at priority level 10" );
    errno = 0;
    temp_taskid = taskSpawn( (char *)0, 10, 0, 0, temp_task,
                             0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );

    puts( "Task 1 signalling Temporary Task to begin forced task deletion test.");
    errno = 0;
    err = semGive( enable1 );
    if ( err == ERROR )
    {
        printf( "semGive of enable1 returned error %x\r\n", errno );
    }

    puts( "Task 1 blocking for handshake from Temporary Task..." );
    err = semTake( complt1, WAIT_FOREVER );

    puts( "\n.......... Call taskIdVerify to verify ID of Temporary Task..." );
    err = taskIdVerify( temp_taskid );
    if ( err == ERROR )
    {
        puts( "taskIdVerify indicates Temporary Task does not exist." );
    }
    else       
    {
        puts( "taskIdVerify indicates Temporary Task does exist." );
        puts( "\n.......... Call taskIsReady to verify it indicates task ready." );
        if ( taskIsReady( temp_taskid ) )
            puts( "taskIsReady indicates Temporary Task is ready." );
        else
            puts( "taskIsReady indicates Temporary Task not ready." );

        puts( "\n.......... Call taskName to check Temporary Task's name." );
        printf( "Temporary Task automatically named %s\r\n", taskName( temp_taskid ) );

        puts( "\n.......... Call taskDeleteForce to delete Temporary Task..." );
        taskDeleteForce( temp_taskid );
        puts( "\n.......... Call taskIdVerify to confirm Temporary Task deleted..." );
        err = taskIdVerify( temp_taskid );
        if ( err == ERROR )
            puts( "taskIdVerify indicates Temporary Task does not exist." );
        else
            puts( "taskIdVerify indicates Temporary Task still exists." );
    }
 
    /************************************************************************
    **  Task Get/Set Priority Test
    ************************************************************************/
    puts( "\n.......... Next test dynamic task priority manipulation." );
    puts( "           Call taskPriorityGet to save Task 2's priority." );
    errno = 0;
    err = taskPriorityGet( task2_id, &oldpriority );
    if ( err == ERROR )
    {
        printf( "\ntaskPriorityGet returned error %x\r\n", errno );
    }
    puts( "\n.......... Next call taskPrioritySet to raise priority on Task 2." );
    puts( "\r\nTask 1 setting priority to 18 for Task 2." );
    errno = 0;
    err = taskPrioritySet( task2_id, 18 );
    if ( err == ERROR )
    {
        printf( "\ntaskPrioritySet returned error %x\r\n", errno );
    }
    display_tcb( task2_id );
    printf( "\r\n" );

    puts( "\n.......... Next call taskPrioritySet to restore Task 2's priority." );
    puts( "\r\nTask 1 restoring Task 2 to original priority setting." );
    errno = 0;
    err = taskPrioritySet( task2_id, oldpriority );
    if ( err == ERROR )
    {
        printf( "\ntaskPrioritySet returned error %x\r\n", errno );
    }
    display_tcb( task2_id );
    printf( "\r\n" );

    /************************************************************************
    **  Task Restart Test
    ************************************************************************/
    puts( "\n.......... Next, we test the task restart logic..." );
    puts( "           Task 8 will be restarted, and Task 1 will sleep" );
    puts( "           briefly to allow Task 8 to run." );
    
    display_tcb( task8_id );
    printf( "\r\n" );

    puts( "\r\nTask 1 calling taskRestart for Task 8" );
    errno = 0;
    err = taskRestart( task8_id );
    if ( err == ERROR )
    {
        printf( "... returned error %x\r\n", errno );
    }
    taskDelay( 2 );
    
    display_tcb( task8_id );
    printf( "\r\n" );

    puts( "\n.......... Next, we test the task self-restart logic..." );
    puts( "           Task 5 will be restarted, and Task 1 will sleep" );
    puts( "           briefly to allow Task 5 to run." );
    
    display_tcb( task5_id );
    printf( "\r\n" );

    task5_restarted++;
    taskDelay( 500 );
    
    display_tcb( task5_id );
    printf( "\r\n" );
    /************************************************************************
    **  Task Identification Test
    ************************************************************************/
    puts( "\n.......... Finally, we test the task Identification logic..." );

    puts( "\r\nTask 1 calling taskNameToId." );
    errno = 0;
    my_taskid = taskNameToId( "TSK3" );
    if ( err == ERROR )
        printf( "\ntaskNameToId for TSK3 returned error %x\r\n", errno );
    else
        printf( "\ntaskNameToId for TSK3 returned ID %x... task3_id == %x\r\n",
                 my_taskid, task3_id );

    puts( "\r\nTask 1 calling taskTcb." );
    temp_tcb = taskTcb( task3_id );
    printf( "\ntaskTcb for TSK3 returned TCB @ %p containing\r\n", temp_tcb );
    display_tcb( temp_tcb->taskid );
    printf( "\r\n" );

    puts( "\r\nTask 1 calling taskIdListGet." );
    num_tasks = taskIdListGet( tid_list, 15 );
    printf( "\ntaskIdListGet returned the following %d Task IDs\r\n",
            num_tasks );
    for ( i = 0; i < num_tasks; i++ )
        printf( "%d ", tid_list[i] );
    printf( "\r\n" );
}

/*****************************************************************************
**  task1
**         This is the 'sequencer' task.  It orchestrates the production of
**         messages, enables, and semaphore tokens in such a way as to provide
**         a predictable sequence of enables for the validation record.  All
**         other tasks 'handshake' with task1 to control the timing of their
**         activities.
**
*****************************************************************************/
int task1( int dummy0, int dummy1, int dummy2, int dummy3, int dummy4,
           int dummy5, int dummy6, int dummy7, int dummy8, int dummy9 )
{
    STATUS err;

    /*
    **  Indicate messages originated with the first test cycle.
    */
    test_cycle = 1;

    puts( "\r\n********** Binary Semaphore creation:" );

    /************************************************************************
    **  Binary Semaphore Creation Test
    ************************************************************************/

    puts( "\n.......... First we create twenty binary semaphores which will" );
    puts( "           be used for task synchronization during tests." );
 
    puts("\nCreating Binary Semaphore enable1, FIFO queuing and 'locked'");
    errno = 0;
    enable1 = semBCreate( SEM_Q_FIFO, SEM_EMPTY );
    if ( errno != OK )
    {
        printf( "... returned error %x\r\n", errno );
    }
 
    puts("\nCreating Binary Semaphore complt1, FIFO queuing and 'locked'");
    errno = 0;
    complt1 = semBCreate( SEM_Q_FIFO, SEM_EMPTY );
    if ( errno != OK )
    {
        printf( "... returned error %x\r\n", errno );
    }
 
    puts("\nCreating Binary Semaphore enable2, FIFO queuing and 'locked'");
    errno = 0;
    enable2 = semBCreate( SEM_Q_FIFO, SEM_EMPTY );
    if ( errno != OK )
    {
        printf( "... returned error %x\r\n", errno );
    }
 
    puts("\nCreating Binary Semaphore complt2, FIFO queuing and 'locked'");
    errno = 0;
    complt2 = semBCreate( SEM_Q_FIFO, SEM_EMPTY );
    if ( errno != OK )
    {
        printf( "... returned error %x\r\n", errno );
    }
 
    puts("\nCreating Binary Semaphore enable3, FIFO queuing and 'locked'");
    errno = 0;
    enable3 = semBCreate( SEM_Q_FIFO, SEM_EMPTY );
    if ( errno != OK )
    {
        printf( "... returned error %x\r\n", errno );
    }
 
    puts("\nCreating Binary Semaphore complt3, FIFO queuing and 'locked'");
    errno = 0;
    complt3 = semBCreate( SEM_Q_FIFO, SEM_EMPTY );
    if ( errno != OK )
    {
        printf( "... returned error %x\r\n", errno );
    }
 
    puts("\nCreating Binary Semaphore enable4, FIFO queuing and 'locked'");
    errno = 0;
    enable4 = semBCreate( SEM_Q_FIFO, SEM_EMPTY );
    if ( errno != OK )
    {
        printf( "... returned error %x\r\n", errno );
    }
 
    puts("\nCreating Binary Semaphore complt4, FIFO queuing and 'locked'");
    errno = 0;
    complt4 = semBCreate( SEM_Q_FIFO, SEM_EMPTY );
    if ( errno != OK )
    {
        printf( "... returned error %x\r\n", errno );
    }
 
    puts("\nCreating Binary Semaphore enable5, FIFO queuing and 'locked'");
    errno = 0;
    enable5 = semBCreate( SEM_Q_FIFO, SEM_EMPTY );
    if ( errno != OK )
    {
        printf( "... returned error %x\r\n", errno );
    }
 
    puts("\nCreating Binary Semaphore complt5, FIFO queuing and 'locked'");
    errno = 0;
    complt5 = semBCreate( SEM_Q_FIFO, SEM_EMPTY );
    if ( errno != OK )
    {
        printf( "... returned error %x\r\n", errno );
    }
 
    puts("\nCreating Binary Semaphore enable6, FIFO queuing and 'locked'");
    errno = 0;
    enable6 = semBCreate( SEM_Q_FIFO, SEM_EMPTY );
    if ( errno != OK )
    {
        printf( "... returned error %x\r\n", errno );
    }
 
    puts("\nCreating Binary Semaphore complt6, FIFO queuing and 'locked'");
    errno = 0;
    complt6 = semBCreate( SEM_Q_FIFO, SEM_EMPTY );
    if ( errno != OK )
    {
        printf( "... returned error %x\r\n", errno );
    }
 
    puts("\nCreating Binary Semaphore enable7, FIFO queuing and 'locked'");
    errno = 0;
    enable7 = semBCreate( SEM_Q_FIFO, SEM_EMPTY );
    if ( errno != OK )
    {
        printf( "... returned error %x\r\n", errno );
    }
 
    puts("\nCreating Binary Semaphore complt7, FIFO queuing and 'locked'");
    errno = 0;
    complt7 = semBCreate( SEM_Q_FIFO, SEM_EMPTY );
    if ( errno != OK )
    {
        printf( "... returned error %x\r\n", errno );
    }
 
    puts("\nCreating Binary Semaphore enable8, FIFO queuing and 'locked'");
    errno = 0;
    enable8 = semBCreate( SEM_Q_FIFO, SEM_EMPTY );
    if ( errno != OK )
    {
        printf( "... returned error %x\r\n", errno );
    }
 
    puts("\nCreating Binary Semaphore complt8, FIFO queuing and 'locked'");
    errno = 0;
    complt8 = semBCreate( SEM_Q_FIFO, SEM_EMPTY );
    if ( errno != OK )
    {
        printf( "... returned error %x\r\n", errno );
    }
 
    puts("\nCreating Binary Semaphore enable9, FIFO queuing and 'free'");
    errno = 0;
    enable9 = semBCreate( SEM_Q_FIFO, SEM_FULL );
    if ( errno != OK )
    {
        printf( "... returned error %x\r\n", errno );
    }
 
    puts( "\nTask 1 locking Binary Semaphore enable9" );
    errno = 0;
    err = semTake( enable9, WAIT_FOREVER );
    if ( err == ERROR )
    {
        printf( " returned error %x\r\n", errno );
    }
 
    puts("\nCreating Binary Semaphore complt9, FIFO queuing and 'locked'");
    errno = 0;
    complt9 = semBCreate( SEM_Q_FIFO, SEM_EMPTY );
    if ( errno != OK )
    {
        printf( "... returned error %x\r\n", errno );
    }
 
    puts("\nCreating Binary Semaphore enable10, FIFO queuing and 'locked'");
    errno = 0;
    enable10 = semBCreate( SEM_Q_FIFO, SEM_EMPTY );
    if ( errno != OK )
    {
        printf( "... returned error %x\r\n", errno );
    }
 
    puts("\nCreating Binary Semaphore complt10, FIFO queuing and 'locked'");
    errno = 0;
    complt10 = semBCreate( SEM_Q_FIFO, SEM_EMPTY );
    if ( errno != OK )
    {
        printf( "... returned error %x\r\n", errno );
    }
 
    validate_tasks();

    test_cycle++;
    validate_binary_semaphores();

    test_cycle++;
    validate_counting_semaphores();

    test_cycle++;
    validate_mutexes();

    test_cycle++;
    validate_msg_queues();

    validate_watchdog_timers();

    taskDelay( 10 );
    fputs("Validation tests completed - enter 'q' to quit...", stderr );

    /*
    **  Tests all done... twiddle our thumbs until user quits program.
    */
    for( ;; )
    {
        taskDelay( 50 );
    }

    return( 0 );
}

/*****************************************************************************
**  user system initialization
*****************************************************************************/
int main ( int argc, char **argv )
{

		v2lin_init();
		
    printf( "\r\n" );

    puts( "Creating MsgQueue 1, Priority Queuing, with 9 1-byte to 16-byte messages" );
    errno = 0;
    queue1_id = msgQCreate( 9, 16, MSG_Q_PRIORITY );
    if ( errno != OK )
    {
        printf( "... returned error %x\r\n", errno );
    }
 
    puts( "Creating MsgQueue 2, Priority Queuing, with 4 1-byte to 128-byte messages" );
    errno = 0;
    queue2_id = msgQCreate( 4, 128, MSG_Q_PRIORITY );
    if ( errno != OK )
    {
        printf( "... returned error %x\r\n", errno );
    }
 
    puts( "Creating MsgQueue 3, Priority Queuing, with 0 1-byte to 16-byte messages" );
    errno = 0;
    queue3_id = msgQCreate( 0, 16, MSG_Q_PRIORITY );
    if ( errno != OK )
    {
        printf( "... returned error %x\r\n", errno );
    }

    /* Turn on round-robin timeslicing */
    enableRoundRobin();

    errno = 0;
    puts( "Starting Task 1 at priority level 5" );
    task1_id = taskSpawn( "TSK1", 5, 0, 0, task1,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );

    printf( "\r\n" );
    
		/*****************************************************************************
		**  user system shutdown and resource cleanup
		*****************************************************************************/
    while ( getchar() != (int)'q' )
        sleep( 1 );
    return 0;
}
