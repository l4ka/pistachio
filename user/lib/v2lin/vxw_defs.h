/*****************************************************************************
 * vxw_defs.h - declares the error message and other constants needed
 *              to implement a Wind River VxWorks (R) kernel API 
 *              in a POSIX Threads environment.
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

#if __cplusplus
extern "C" {
#endif

/*
**  Generic data types
*/
#ifndef BOOL
#define BOOL int
#endif

#ifndef STATUS
#define STATUS int
#endif

/*
**  Miscellaneous error codes
*/
#define TASK_ERRS                       0x00030000
#define MEM_ERRS                        0x00110000
#define MSGQ_ERRS                       0x00410000
#define OBJ_ERRS                        0x003d0000
#define SEM_ERRS                        0x00160000
#define SM_OBJ_ERRS                     0x00580000

#define S_memLib_NOT_ENOUGH_MEMORY      (MEM_ERRS + 1)

#define S_msgQLib_INVALID_MSG_LENGTH    (MSGQ_ERRS + 1)

#define S_objLib_OBJ_DELETED            (OBJ_ERRS + 3)
#define S_objLib_OBJ_ID_ERROR           (OBJ_ERRS + 1)
#define S_objLib_OBJ_TIMEOUT            (OBJ_ERRS + 4)
#define S_objLib_OBJ_UNAVAILABLE        (OBJ_ERRS + 2)

#define S_semLib_INVALID_OPERATION      (SEM_ERRS + 0x00000068)

#define S_smObjLib_NOT_INITIALIZED      (SM_OBJ_ERRS + 1)

#define S_taskLib_ILLEGAL_PRIORITY      (TASK_ERRS + 0x00000065)

/*
**  Timeout options
*/
#define NO_WAIT                         0
#define WAIT_FOREVER                    -1

/*
**  Message Queue Option Flags
*/
#define MSG_PRI_NORMAL                  0x00
#define MSG_PRI_URGENT                  0x01
#define MSG_Q_FIFO                      0x00
#define MSG_Q_PRIORITY                  0x01

/*
**  Semaphore Option Flags
*/
#define SEM_EMPTY                       0x00
#define SEM_FULL                        0x01
#define SEM_Q_FIFO                      0x00
#define SEM_Q_PRIORITY                  0x01
#define SEM_DELETE_SAFE                 0x04
#define SEM_INVERSION_SAFE              0x08

#if __cplusplus
}
#endif

