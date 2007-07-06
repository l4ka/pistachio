/*********************************************************************
 *                
 * Copyright (C) 2003, 2006,  University of New South Wales
 *                
 * File path:     glue/v4-sparc64/utcb.h
 * Description:
 *                
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *                
 * $Id: utcb.h,v 1.6 2006/10/20 16:30:59 reichelt Exp $
 *                
 ********************************************************************/

#ifndef __GLUE__V4_SPARC64__UTCB_H__
#define __GLUE__V4_SPARC64__UTCB_H__

#include INC_ARCH(registers.h)

/* Find how big the register window backing area needs to be. */
#if (NWINDOWS <= 8)        /* Can we fit it into 1KB?   */

#define NWINDOWS_MAX 8
#define UTCB_WIN_BITS   1  /* assuming UTCB_BITS == 10! */

#elif (NWINDOWS <= 24)     /* Can we fit it into 3KB?   */

#define NWINDOWS_MAX 24
#define UTCB_WIN_BITS   2  /* assuming UTCB_BITS == 10! */

#else

#error SPARC64 cpu register file is too big for UTCB!

#endif /* SPARC64_REG_WIN <= * */

#define UTCB_BITS (10 /* 1KB */ + UTCB_WIN_BITS)
#define PADDING_0 ((248 - 80) / sizeof(word_t))

#define UTCB_MASK (~((1 << UTCB_BITS) - 1))

#ifndef ASSEMBLY

#include INC_ARCH(types.h)
#include INC_API(types.h)
#include INC_API(thread.h)
#include INC_ARCH(frame.h)

class utcb_t {
public:
    threadid_t my_global_id;          /* 0    .. 8    */
    threadid_t pager;                 /* 8    .. 16   */
    threadid_t exception_handler;     /* 16   .. 24   */
    word_t     user_defined_handle;   /* 24   .. 32   */
    timeout_t  xfer_timeout;          /* 32   .. 40   */
    word_t     error_code;            /* 40   .. 48   */
    threadid_t intended_receiver;     /* 48   .. 56   */
    threadid_t virtual_sender;        /* 56   .. 64   */
    word_t     processor_no;          /* 64   .. 72   */
    u8_t       preempt_flags;         /* 72   .. 73   */
    u8_t       cop_flags;             /* 73   .. 74   */
    u16_t      __reserved0;           /* 74   .. 76   */
    u32_t      __reserved1;           /* 76   .. 80   */
    word_t     __padding0[PADDING_0]; /* 80   .. 248  */
    word_t     br[IPC_NUM_BR];        /* 248  .. 512  */
    word_t     mr[IPC_NUM_MR];        /* 512  .. 1024 */

    /********************************************************************
     * Register windows are normally spilled to/filled from the users   *
     * stack. However if we switch threads we need somewhere to         *
     * temporarily store the windows (and avoid touching user space)    *
     * so we stick them in the UTCB at 'reg_win'. SPARC v9 has a max of *
     * 32 windows but we can only fit a maximum of 24 in here and stay  *
     * under 4KB. So far this isn't a problem as the UltraSPARC I/II    *
     * have 8 windows while the UltraSPARC III has 16 windows.          *
     ********************************************************************/

    window_frame_t reg_win[NWINDOWS_MAX];

public:
    void set_my_global_id(threadid_t tid);
    word_t get_user_defined_handle();
    void set_user_defined_handle(word_t handle);
    threadid_t get_pager();
    void set_pager(threadid_t tid);
    threadid_t get_exception_handler();
    void set_exception_handler(threadid_t tid);
    u8_t get_preempt_flags();
    void set_preempt_flags(u8_t flags);
    u8_t get_cop_flags();
    word_t get_error_code();
    void set_error_code(word_t err);
    timeout_t get_xfer_timeout();
    threadid_t get_intended_receiver();
    threadid_t get_virtual_sender();
    void set_virtual_sender(threadid_t tid);

} __attribute__((packed)); // utcb_t

#include INC_API(generic-utcb.h)

#endif /* !ASSEMBLY */


#endif /* !__GLUE__V4_SPARC64__UTCB_H__ */
