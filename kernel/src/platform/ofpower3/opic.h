/****************************************************************************
 *
 * Copyright (C) 2002, Karlsruhe University
 *
 * File path:	platform/ofpower3/opic.h
 * Description:	OpenPIC controller definition.
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
 * $Id: opic.h,v 1.1 2003/10/22 07:52:09 cvansch Exp $
 *
 ***************************************************************************/

#ifndef __PLATFORM__OFPOWER3__OPIC_H__
#define __PLATFORM__OFPOWER3__OPIC_H__

#include INC_ARCH(io.h)

/* OpenPIC defines */
#define OPEN_PIC_GLOBAL_OFFSET		(0x1000)
#define OPEN_PIC_GLOBAL_PAD		(0xee00)
#define OPEN_PIC_NUM_IPI		(4)
#define OPEN_PIC_NUM_TIMERS		(4)
#define OPEN_PIC_MAX_SOURCES		(2048)
#define OPEN_PIC_MAX_PROCESSORS		(32)

/* OpenPIC registers are 128-bits in size, first 32-bits is the offset */
class open_pic_reg_t
{
public:
    u32_t   reg;
    u32_t   pad[3];
};

class open_pic_global_t
{
public:
    /* Features	    */
    open_pic_reg_t	feature_reporting0;		/* Read only	*/
    open_pic_reg_t	feature_reporting1;		/* Future	*/
    /* Global config    */
    open_pic_reg_t	global_config0;			/* Read/write	*/
    open_pic_reg_t	global_config1;			/* Future	*/
    /* Vendor specific  */
    open_pic_reg_t	vendor_specific[4];
    /* Vendor ID	    */
    open_pic_reg_t	vendor_id;			/* Read only	*/
    /* Processor init register  */
    open_pic_reg_t	processor_init;			/* Read/write	*/
    /* IPI vector/priority	    */
    open_pic_reg_t	ipi_vector_priority[ OPEN_PIC_NUM_IPI ];	/* Read/write	*/
    /* Spurious vector  */
    open_pic_reg_t	spurious_vector;		/* Read/write	*/
    /* Global timer registers   */
    open_pic_reg_t	timer_frequency;		/* Read/write	*/
    open_pic_reg_t	timer[ OPEN_PIC_NUM_TIMERS ];

    u8_t	_pad[ OPEN_PIC_GLOBAL_PAD ];
};

class open_pic_source_t
{
public:
    open_pic_reg_t  vector_priority;			/* Read/write	*/
    open_pic_reg_t  destination;			/* Read/write	*/
};

/* register structures */
class open_pic_feature0_t
{
public:
    union {
	struct {
	    BITFIELD5( u32_t, 
		version		: 8, 
		last_cpu	: 5, 
		unknown0	: 3, 
		last_source	: 11,
		unknown1	: 5 
	    );
	} x;
	u32_t raw;
    };
};  


class open_pic_processor_t
{
public:
    /* Private shadow registers	*/
    u32_t   ipi0_dispatch_shadow;			/* Write only	*/
    u8_t    _pad0[4];
    u32_t   ipi0_vector_priority_shadow;		/* Read/write	*/
    u8_t    _pad1[0x34];
    /* Interprocessor interupt command ports	*/
    open_pic_reg_t  ipi_dispatch[ OPEN_PIC_NUM_IPI ];	/* Write only	*/
    /* Current task priority register	*/
    open_pic_reg_t  current_task_priority;		/* Read/write	*/
    u8_t    _pad2[0x10];
    /* Interrupt acknowledge register	*/
    open_pic_reg_t  interrupt_acknowledge;		/* Read only	*/
    /* End of interrupt register	*/
    open_pic_reg_t  EOI;				/* Read/write	*/
    u8_t    _pad3[0xf40];
};

class open_pic_t
{
public:
    open_pic_feature0_t get_feature0();
    word_t get_timer_frequency();
private:
    u8_t    _pad[ OPEN_PIC_GLOBAL_OFFSET ];
    open_pic_global_t	global;
    /* Interrupt source config registers    */
    open_pic_source_t	source[ OPEN_PIC_MAX_SOURCES ];
    /* Per processor registers		    */
    open_pic_processor_t    processor[ OPEN_PIC_MAX_PROCESSORS ];
};

INLINE open_pic_feature0_t open_pic_t::get_feature0()
{
    open_pic_feature0_t t;
    t.raw = in32le( &this->global.feature_reporting0 );
    return t;
}

INLINE word_t open_pic_t::get_timer_frequency()
{
    return in32le( &this->global.timer_frequency );
}

#endif /* __PLATFORM__OFPOWER3__OPIC_H__ */
