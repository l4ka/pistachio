/*****************************************************************************
 * demo.c - demonstrates the implementation of a Wind River VxWorks (R) 
 *          application in a POSIX Threads environment.
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

#define EVENT1 1
#define EVENT2 2

extern v2pthread_cb_t *
   my_tcb( void );
extern void
    ts_free( char *blkaddr );
extern void *
    ts_malloc( size_t blksize );

/*****************************************************************************
**  demo program global data structures
*****************************************************************************/
static int task1_id;
static int task2_id;
static int task3_id;

static MSG_Q_ID queue2_id;
static MSG_Q_ID queue3_id;

static SEM_ID sema42_id;
static SEM_ID sema43_id;

/*****************************************************************************
**  display_tcb
*****************************************************************************/
static void display_tcb( void )
{
    
    int policy;
    v2pthread_cb_t *cur_tcb;

    cur_tcb = my_tcb();

    if ( cur_tcb == (v2pthread_cb_t *)NULL )
        return;

    printf(
         "\r\nTask Name: %s  Task ID: %d  Thread ID: %ld  Vxworks priority: %d",
            cur_tcb->taskname, cur_tcb->taskid, cur_tcb->pthrid,
            cur_tcb->vxw_priority );

    policy = (cur_tcb->attr).__schedpolicy;
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
    printf( " priority %d ", ((cur_tcb->attr).__schedparam).sched_priority );
    printf( " prv_priority %d ", (cur_tcb->prv_priority).sched_priority );
    printf( " detachstate %d ", (cur_tcb->attr).__detachstate );
}

/*
 * This example illustrates a producer/consumer problem where there is one
 * producer and two consumers. The producer waits till one or both consumers
 * are ready to receive a message (indicated by semaphores) and then posts a
 * message on the queue of that consumer. Messages consist of dynamically
 * allocated memory blocks: producer allocates a block, writes data into it
 * and sends it to consumers, then releases block after queuing message. 
 * Consumers signal ready to receive data, then fetch message and read the data
 */

/*-----------------------------------------------------------------------*/
/*
 *  task1  - This is the producer task. It waits till one of the consumers is
 *           ready to receive (by pending on 2 events; note that option of
 *           ev_receive is EV_ANY) and then posts a message on the consumer
 *           task's queue.
 */

int task1( int dummy0, int dummy1, int dummy2, int dummy3, int dummy4,
           int dummy5, int dummy6, int dummy7, int dummy8, int dummy9 )
{

STATUS err;
int i;
char *buffer;

taskDelay( 50 );

for (;;) {
    puts( "\r\ntask1 waiting for ready signal from either of SEM2 or SEM3" );

    err = semTake( sema42_id, NO_WAIT );
    if ( err == OK )
    {
        /*
        **  Received ready signal from task 2...
        */
        buffer = ts_malloc( 128 );
        if ( buffer != (char *)NULL )
        {
            for (i = 0; i < 10; i++)
                buffer[i] = 'A' + i;
            buffer[i] = 0;

            printf("\r\ntask1's message for task2: %s\n", buffer );

            err = msgQSend( queue2_id, buffer, 11, NO_WAIT, MSG_PRI_NORMAL );
            ts_free( buffer );
        }
        else
            puts( "\r\nNo memory for message to task 2" );
    }

    err = semTake( sema43_id, NO_WAIT );
    if ( err == OK )
    {
        /*
        **  Received ready signal from task 3...
        */
        buffer = ts_malloc( 128 );
        if ( buffer != (char *)NULL )
        {
            for (i = 0; i < 10; i++)
                buffer[i] = 'Z' - i;
            buffer[i] = 0;

            printf("\r\ntask1's message for task3: %s\n", buffer );

            err = msgQSend( queue3_id, buffer, 11, NO_WAIT, MSG_PRI_NORMAL );
            ts_free( buffer );
        }
        else
            puts( "\r\nNo memory for message to task 3" );
    }

    taskDelay( 1 );
  }
}

/*-----------------------------------------------------------------------*/
/*
 *  task2  - First consumer task. It tells the producer that it is ready to
 *           receive a message by posting an event on which producer is pending.
 *           Then it pends on the queue where producer posts the message.
 */

int task2( int dummy0, int dummy1, int dummy2, int dummy3, int dummy4,
           int dummy5, int dummy6, int dummy7, int dummy8, int dummy9 )
{
STATUS err;
int i;
char foo;
char msg[128];

  for (;;) {
    err = semGive( sema42_id );

    puts( "\r\ntask2 waiting on message in QUEUE2" );
    errno = 0;
    err = msgQReceive( queue2_id, msg, 128, WAIT_FOREVER );
    if ( err == ERROR )
        printf( "\nmsgQReceive for QUEUE2 returned error %x\r\n", errno );
    else
    {
        printf("\r\ntask2 received message from task1: %s\n", msg);

        for (i = 0; msg[i]; i++)
            foo ^= msg[i];
    }
  }
}

/*-----------------------------------------------------------------------*/
/*
 *  task3  - Second consumer task. Same as consumer1. The 2 consumers use
 *           different events and queues and they alternate because of the
 *           time slicing.
 */

int task3( int dummy0, int dummy1, int dummy2, int dummy3, int dummy4,
           int dummy5, int dummy6, int dummy7, int dummy8, int dummy9 )
{
STATUS err;
int i;
char foo;
char msg[128];

  for (;;) {
    err = semGive( sema43_id );

    puts( "\r\ntask3 waiting on message in QUEUE3" );
    errno = 0;
    err = msgQReceive( queue3_id, msg, 128, WAIT_FOREVER );
    if ( err == ERROR )
        printf( "\nmsgQReceive for QUEUE3 returned error %x\r\n", errno );
    else
    {
        printf("\r\ntask3 received message from task1: %s\n", msg);

        for (i = 0; msg[i]; i++)
            foo += msg[i];
    }
  }
}

/*-----------------------------------------------------------------------*/

/*****************************************************************************
**  user system initialization
*****************************************************************************/
void
    user_sysinit( void )
{
    printf( "\r\nCreating Queue QUEUE2" );
    errno = 0;
    queue2_id = msgQCreate( 1, 128, MSG_Q_PRIORITY );
    if ( errno != OK )
        printf( "... returned error %x\r\n", errno );
 
    printf( "\r\nCreating Queue QUEUE3" );
    errno = 0;
    queue3_id = msgQCreate( 3, 128, MSG_Q_FIFO );
    if ( errno != OK )
        printf( "... returned error %x\r\n", errno );
   
    printf( "\r\nCreating Semaphore SEM2" );
    errno = 0;
    sema42_id = semBCreate( SEM_Q_FIFO, SEM_EMPTY );
    if ( errno != OK )
        printf( "... returned error %x\r\n", errno );
  
    printf( "\r\nCreating Semaphore SEM3" );
    errno = 0;
    sema43_id = semBCreate( SEM_Q_FIFO, SEM_EMPTY );
    if ( errno != OK )
        printf( "... returned error %x\r\n", errno );

    /* Turn on round-robin timeslicing */
    enableRoundRobin();
  
    puts( "\r\nCreating Task 1" );
    task1_id = taskSpawn( "TSK1", 10, 0, 0, task1,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );

    puts( "\r\nCreating Task 2" );
    task2_id = taskSpawn( "TSK2", 10, 0, 0, task2,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );

    puts( "\r\nCreating Task 3" );
    task3_id = taskSpawn( "TSK3", 10, 0, 0, task3,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
}

/*****************************************************************************
**  user system shutdown and resource cleanup
*****************************************************************************/
void
    user_syskill( void )
{
    STATUS err;

    while ( getchar() != (int)'q' )
        sleep( 1 );

    puts( "\r\nDeleting Task 1" );
    err = taskDelete( task1_id );

    puts( "\r\nDeleting Task 2" );
    err = taskDelete( task2_id );

    puts( "\r\nDeleting Task 3" );
    err = taskDelete( task3_id );

    printf( "\r\nDeleting Semaphore SEM3" );
    errno = 0;
    err = semDelete( sema43_id );
    if ( err == ERROR )
        printf( "... returned error %x\r\n", errno );

    printf( "\r\nDeleting Semaphore SEM2" );
    errno = 0;
    err = semDelete( sema42_id );
    if ( err == ERROR )
        printf( "... returned error %x\r\n", errno );

    printf( "\r\nDeleting Queue QUEUE3" );
    errno = 0;
    err = msgQDelete( queue3_id );
    if ( err == ERROR )
        printf( "... returned error %x\r\n", errno );
    else
        printf( "\r\n" );

    printf( "\r\nDeleting Queue QUEUE2" );
    errno = 0;
    err = msgQDelete( queue2_id );
    if ( err == ERROR )
        printf( "... returned error %x\r\n", errno );
    else
        printf( "\r\n" );

    printf( "\r\n" );
}

