/*****************************************************************************
 * v2pthread.h - declares the wrapper functions and data structures needed
 *               to implement a Wind River VxWorks (R) kernel API 
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

#include <pthread.h>
//#include <asm/atomic.h>

#if __cplusplus
extern "C" {
#endif

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE  !FALSE
#endif

#define V2PT_TICK 10 /* milliseconds per v2pthread scheduler tick */

/*
**  Task Scheduling Priorities in v2pthread are higher as numbers decrease...
**  define the largest and smallest possible priority numbers.
*/
#define MIN_V2PT_PRIORITY 255
#define MAX_V2PT_PRIORITY 0

#ifndef OK
#define OK     0      /* Normal return value */
#endif
#ifndef ERROR
#define ERROR  -1     /* Error return value */
#endif
/*****************************************************************************
**  Task state bit masks
*****************************************************************************/
#define READY   0x0000
#define PEND    0x0001
#define DELAY   0x0002
#define SUSPEND 0x0004
#define TIMEOUT 0x0008
#define INH_PRI 0x0010
#define DEAD    0x0080
#define RDY_MSK 0x008f

/*****************************************************************************
**  Control block for pthread wrapper for v2pthread task
*****************************************************************************/
typedef struct v2pt_pthread_ctl_blk
{
        /*
        ** Task ID for task
        */
    int
        taskid;

        /*
        ** Task Name
        */
    char
        *taskname;

        /*
        ** Thread ID for task
        */
    pthread_t pthrid;

        /*
        ** Thread attributes for task
        */
    pthread_attr_t
        attr;

        /*
        ** Previous scheduler priority for task
        */
    struct sched_param
        prv_priority;

        /*
        ** Execution entry point address for task
        */
    int (*entry_point)( int, int, int, int, int, int, int, int, int, int );

        /*
        ** Task parameter block address
        */
    int 
        parms[10];

        /*
        ** Flag indicating if task control block allocated dynamically ( == 0 )
        ** or statically ( == 1 )
        */
    int
        static_tcb;

        /*
        ** Option flags for task
        */
    int
        flags;

        /*
        ** Task state
        */
    int
        state;

        /*
        ** Task v2pthread priority level
        */
    int
        vxw_priority;

        /*
        ** Pointer to suspended task list for object task is waiting on
        */
    struct v2pt_pthread_ctl_blk **
        suspend_list;

        /*
        ** Next task control block in list of tasks waiting on object
        */
    struct v2pt_pthread_ctl_blk *
        nxt_susp;

        /*
        ** Nesting level for number of taskSafe calls
        */
    int
        delete_safe_count;

        /*
        ** Mutex and Condition variable for task delete 'pend'
        */
    pthread_mutex_t
        tdelete_lock;
    pthread_cond_t
        t_deletable;

        /*
        ** Mutex and Condition variable for task delete 'broadcast'
        */
    pthread_mutex_t
        dbcst_lock;
    pthread_cond_t
        delete_bcplt;

        /*
        ** First task control block in list of tasks waiting on this task
        ** (for deletion purposes)
        */
    struct v2pt_pthread_ctl_blk *
        first_susp;

        /*
        ** Next task control block in list
        */
    struct v2pt_pthread_ctl_blk *
        nxt_task;
} v2pthread_cb_t;

#if __cplusplus
}
#endif

