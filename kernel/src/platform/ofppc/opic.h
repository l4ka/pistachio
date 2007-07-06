/****************************************************************************
 *
 * Copyright (C) 2002, Karlsruhe University
 *
 * File path:	platform/ofppc/opic.h
 * Description:	The open pic driver.
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
 * $Id: opic.h,v 1.14 2003/09/24 19:04:58 skoglund Exp $
 *
 ***************************************************************************/

#ifndef __PLATFORM__OFPPC__OPIC_H__
#define __PLATFORM__OFPPC__OPIC_H__

#include INC_PLAT(1275tree.h)
#include INC_PLAT(opic.h)

#include INC_API(smp.h)

#define OPIC_MAX_SOURCES	2048
#define OPIC_MAX_CPUS		32
#define OPIC_MAX_ISU		16

#define OPIC_NUM_TIMERS		4
#define OPIC_NUM_IPI		4
#define OPIC_NUM_PRI		16
#define OPIC_NUM_VECTORS	256

#define OPIC_REG_ALIGN			16
#define OPIC_CPU_DISABLE		0
#define OPIC_TIMER_FIELDS		4
#define OPIC_SOURCE_FIELDS		2

#define OPIC_GLOBAL_OFFSET	(0x1000)
#define OPIC_REG(d)		((d)*OPIC_REG_ALIGN + OPIC_GLOBAL_OFFSET)
#define OPIC_FEATURE_REPORTING0_REG	OPIC_REG(0)
#define OPIC_FEATURE_REPORTING1_REG	OPIC_REG(1)
#define OPIC_GLOBAL_CONFIG0_REG		OPIC_REG(2)
#define OPIC_GLOBAL_CONFIG1_REG		OPIC_REG(3)
#define OPIC_VENDER0_REG		OPIC_REG(4)
#define OPIC_VENDER1_REG		OPIC_REG(5)
#define OPIC_VENDER2_REG		OPIC_REG(6)
#define OPIC_VENDER3_REG		OPIC_REG(7)
#define OPIC_VENDER_IDENT_REG		OPIC_REG(8)
#define OPIC_CPU_INIT_REG		OPIC_REG(9)
#define OPIC_IPI0_PRIORITY_REG		OPIC_REG(10)
#define OPIC_IPI1_PRIORITY_REG		OPIC_REG(11)
#define OPIC_IPI2_PRIORITY_REG		OPIC_REG(12)
#define OPIC_IPI3_PRIORITY_REG		OPIC_REG(13)
#define OPIC_SPURIOUS_REG		OPIC_REG(14)
#define OPIC_TIMER_FREQUENCY_REG	OPIC_REG(15)
#define OPIC_TIMER0_CURRENT_COUNT_REG	OPIC_REG(16)
#define OPIC_TIMER0_BASE_COUNT_REG	OPIC_REG(17)
#define OPIC_TIMER0_VECTOR_REG		OPIC_REG(18)
#define OPIC_TIMER0_CPU_REG		OPIC_REG(19)

#define OPIC_SRC_OFFSET		(0xf000 + OPIC_GLOBAL_OFFSET)
#define OPIC_SRC_TOT_SIZE	(OPIC_SOURCE_FIELDS * OPIC_MAX_SOURCES * OPIC_REG_ALIGN)
#define OPIC_SRC_REG(d)		((d)*OPIC_REG_ALIGN + OPIC_SRC_OFFSET)
#define OPIC_SRC0_VECTOR_REG		OPIC_SRC_REG(0)
#define OPIC_SRC0_CPU_REG		OPIC_SRC_REG(1)

#define OPIC_CPU_OFFSET		(OPIC_SRC_OFFSET + OPIC_SRC_TOT_SIZE)
#define OPIC_CPU_SIZE		(0x1000)
#define OPIC_CPU_REG(d,cpu)	((d)*OPIC_REG_ALIGN + OPIC_CPU_OFFSET + cpu*OPIC_CPU_SIZE)
#define OPIC_CPU_IPI_DISPATCH0_REG(cpu)		OPIC_CPU_REG(4,cpu)
#define OPIC_CPU_IPI_DISPATCH1_REG(cpu)		OPIC_CPU_REG(5,cpu)
#define OPIC_CPU_IPI_DISPATCH2_REG(cpu)		OPIC_CPU_REG(6,cpu)
#define OPIC_CPU_IPI_DISPATCH3_REG(cpu)		OPIC_CPU_REG(7,cpu)
#define OPIC_CPU_CURRENT_PRIORITY_REG(cpu)	OPIC_CPU_REG(8,cpu)
#define OPIC_CPU_IRQ_ACK_REG(cpu)		OPIC_CPU_REG(10,cpu)
#define OPIC_CPU_EOI_REG(cpu)			OPIC_CPU_REG(11,cpu)

class opic_global_config0_t {
public:
    union {
	struct {
	    BITFIELD5( u32_t,
		base 		: 20,
		unknown0	: 9,
		disable_8259	: 1,
		unknown1	: 1,
		reset		: 1
		);
	} x;
	u32_t raw;
    };
};

class opic_feature0_t {
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

class opic_vector_priority_t {
public:
    union {
	struct {
	    BITFIELD9( u32_t,
		vector		: 8,
		unknown0	: 8,
		priority	: 4,
		unknown1	: 2,
		level		: 1,
		positive	: 1,
		unknown2	: 6,
		activity	: 1,
		mask		: 1
		);
	} x;
	u32_t raw;
    };
};

class opic_cpu_priority_t {
public:
    union {
	struct {
	    BITFIELD2( u32_t,
		priority : 4,
		unknown : 28
		);
	} x;
	u32_t raw;
    };
};

class opic_irq_ack_t {
public:
    union {
	struct {
	    BITFIELD2( u32_t,
		vector : 8,
		unknown : 24
		);
	} x;
	u32_t raw;
    };
};

class opic_timer_count_t {
public:
    union {
	struct {
	    BITFIELD2( u32_t,
		count : 31,
		disabled : 1
		);
	} x;
	u32_t raw;
    };
};

class intctrl_t 
{
public:
    enum priority_e {
	priority_timer = 1, 
	priority_std_source = 8,
	priority_std_ipi = 8,
	priority_spurious = 15
    };

    enum vector_e {
	timer0_vec = 0,
	timer1_vec = 1,
	timer2_vec = 2,
	timer3_vec = 3,
	timer_end_vec = timer3_vec,
	source_start_vec = timer_end_vec+1,
	source_end_vec = 253,
	ipi0_vec = source_end_vec+1,
	ipi_end_vec = ipi0_vec,
	spurious_vec = 255
    };

private:
    word_t opic_vaddr;
    word_t opic_paddr;
    word_t opic_size;

    word_t last_vector;
    word_t num_cpus;

public:
    /* L4 init functions. */
    void init_arch();
    void init_cpu( word_t cpu );
    void bat_map();
#if defined(CONFIG_SMP)
    void start_new_cpu( word_t cpu );
#endif

    /* L4 query functions. */
    word_t get_number_cpus() { return num_cpus; }
    word_t get_number_irqs() { return last_vector; }
    bool is_irq_available( word_t irq ) { return irq < last_vector; }

    /* L4 modification functions. */
    void mask( word_t irq );
    void mask_and_ack( word_t irq );
    bool unmask( word_t irq );
    void ack( word_t irq );
    void enable( word_t irq )	{ unmask( irq ); }
    void disable( word_t irq )	{ mask( irq ); }
    void set_cpu( word_t irq, word_t cpu );

    /* L4 irq processing functions. */
    void handle_irq( word_t irq );

    /* Debug functions. */
    void enable_timer( word_t timer );

#if defined(CONFIG_SMP)
    /* cpu functions. */
    void send_ipi0( word_t src_cpu, word_t dst_cpu_mask )
	{ out32le( OPIC_CPU_IPI_DISPATCH0_REG(src_cpu), dst_cpu_mask ); }
#endif

private:
    /* Setup. */
    of1275_device_t *find_opic();
    void scan_interrupt_tree( of1275_device_t *dev_opic);
    void scan_interrupt_map( of1275_device_t *dev_opic, of1275_device_t *node );
    void init_intctrl();

    void init_source( int source, int sense );
    void init_spurious();
    void init_timers();
    void init_all_ipi();

    /* Query. */
    bool is_timer_vector( word_t vector );
    bool is_source_vector( word_t vector );
    opic_feature0_t get_feature0();
    opic_global_config0_t get_global_config0();
    opic_cpu_priority_t get_current_task_priority( word_t cpu );
    opic_irq_ack_t get_irq_ack( word_t cpu );
    opic_vector_priority_t get_vector_priority( word_t reg );

    /* Modify. */
    void set_feature0( opic_feature0_t val ) 
	{ out32le(OPIC_FEATURE_REPORTING0_REG, val.raw); }
    void set_global_config0( opic_global_config0_t val )
	{ out32le(OPIC_GLOBAL_CONFIG0_REG, val.raw); }
    void set_current_task_priority( u32_t priority, word_t cpu );
    void disable_8259_pass_through( void );
    void clear_eoi( word_t cpu )
	{ out32le(OPIC_CPU_EOI_REG(cpu), 0); }
    void write_vector_priority( word_t reg, opic_vector_priority_t val );
    void set_reg_mask( word_t reg, u32_t mask );

    /* Timers. */
    u32_t get_timer_freq() { return in32le(OPIC_TIMER_FREQUENCY_REG); }
    u32_t get_timer_count( word_t timer );
    opic_vector_priority_t get_timer_vector_priority( word_t timer );
    void set_timer_vector_priority( word_t timer, opic_vector_priority_t val );
    void set_timer_cpu( word_t timer, u32_t cpu );
    void restart_timer( word_t timer );
    void timer_set_count( word_t timer, opic_timer_count_t val );
    void set_timer_freq( u32_t freq )
	{ out32le(OPIC_TIMER_FREQUENCY_REG, freq); }
    void mask_timer( word_t timer );
    void unmask_timer( word_t timer );

    /* Sources. */
    void mask_source( word_t source );
    void unmask_source( word_t source );
    void set_source_vector_priority( word_t source, opic_vector_priority_t val);
    void set_source_cpu( word_t source, u32_t cpu );

    /* Low-level i/o. */
    void out32le( word_t reg, u32_t val );
    void out32be( word_t reg, u32_t val );
    u32_t in32le( word_t reg );
    u32_t in32be( word_t reg );
};

/**
 * Little-endian I/O write to an open-pic register.
 * @param reg	The register offset.
 * @param val	The big-endian value, which will be byte reversed when written.
 */
INLINE void intctrl_t::out32le( word_t reg, u32_t val )
{
    asm volatile( "stwbrx %0, 0, %1 ; eieio ;" 
	    : 
	    : "r" (val), "r" (reg + this->opic_vaddr) );
}

/**
 * Big-endian I/O write to an open-pic register.
 * @param reg	The register offset.
 * @param val	The value to write to the register.
 */
INLINE void intctrl_t::out32be( word_t reg, u32_t val )
{
    asm volatile( "stw %0, 0(%1) ; eieio ;" 
	    : 
	    : "r" (val), "r" (reg + this->opic_vaddr) );
}

/**
 * Little-endian I/O read from an open-pic register.
 * @param reg	The register offset.
 *
 * The return value is converted to big-endian.
 */
INLINE u32_t intctrl_t::in32le( word_t reg )
{
    u32_t val;
    asm volatile( "lwbrx %0, 0, %1 ; eieio ;" 
	    : "=r" (val) 
	    : "r" (reg + this->opic_vaddr) );
    return val;
}

/**
 * Big-endian I/O read from an open-pic register.
 * @param reg	The register offset.
 */
INLINE u32_t intctrl_t::in32be( word_t reg )
{
    u32_t val;
    asm volatile( "lwz %0, 0, %1 ; eieio ;" 
	    : "=r" (val) 
	    : "r" (reg + this->opic_vaddr) );
    return val;
}

INLINE opic_feature0_t intctrl_t::get_feature0()
{
    opic_feature0_t t;
    t.raw = this->in32le( OPIC_FEATURE_REPORTING0_REG );
    return t;
}

INLINE opic_vector_priority_t intctrl_t::get_vector_priority( word_t reg )
{
    opic_vector_priority_t t;
    t.raw = this->in32le( reg );
    return t;
}

INLINE void intctrl_t::set_timer_vector_priority( word_t timer, opic_vector_priority_t val )
{
    ASSERT( timer < OPIC_NUM_TIMERS );
    this->write_vector_priority( 
	    OPIC_TIMER0_VECTOR_REG + OPIC_REG_ALIGN*timer*OPIC_TIMER_FIELDS, 
	    val );
}

INLINE opic_vector_priority_t intctrl_t::get_timer_vector_priority(
	word_t timer)
{
    opic_vector_priority_t vp;

    ASSERT( timer < OPIC_NUM_TIMERS );
    vp.raw = this->in32le(
	    OPIC_TIMER0_VECTOR_REG + OPIC_REG_ALIGN*timer*OPIC_TIMER_FIELDS );
    return vp;
}

INLINE void intctrl_t::set_timer_cpu( word_t timer, u32_t cpu )
{
    ASSERT( timer < OPIC_NUM_TIMERS );
    this->out32le( 
	    OPIC_TIMER0_CPU_REG + OPIC_REG_ALIGN*timer*OPIC_TIMER_FIELDS, 
	    cpu );
}

INLINE void intctrl_t::set_source_vector_priority( word_t source, opic_vector_priority_t val )
{
    this->write_vector_priority( 
	    OPIC_SRC0_VECTOR_REG + source*OPIC_REG_ALIGN*OPIC_SOURCE_FIELDS,
	    val );
}

INLINE void intctrl_t::set_source_cpu( word_t source, u32_t cpu )
{
    this->out32le(
	    OPIC_SRC0_CPU_REG + source*OPIC_REG_ALIGN*OPIC_SOURCE_FIELDS,
	    cpu );
}

INLINE opic_global_config0_t intctrl_t::get_global_config0()
{
    opic_global_config0_t val;
    val.raw = this->in32le( OPIC_GLOBAL_CONFIG0_REG );
    return val;
}

INLINE opic_cpu_priority_t intctrl_t::get_current_task_priority( word_t cpu )
{
    opic_cpu_priority_t val;
    val.raw = this->in32le( OPIC_CPU_CURRENT_PRIORITY_REG(cpu) );
    return val;
}

INLINE opic_irq_ack_t intctrl_t::get_irq_ack( word_t cpu )
{
    opic_irq_ack_t val;
    val.raw = this->in32le( OPIC_CPU_IRQ_ACK_REG(cpu) );
    return val;
}

INLINE void intctrl_t::restart_timer( word_t timer )
{
    opic_timer_count_t t;
    t.x.count = 0x1000;
    t.x.disabled = 0;
    this->timer_set_count( timer, t );
}

INLINE void intctrl_t::timer_set_count( word_t timer, opic_timer_count_t val )
{
    ASSERT( timer < OPIC_NUM_TIMERS );
    this->out32le( 
	    OPIC_TIMER0_BASE_COUNT_REG + OPIC_REG_ALIGN*timer*OPIC_TIMER_FIELDS,
	    val.raw );
}

INLINE u32_t intctrl_t::get_timer_count( word_t timer )
{
    ASSERT( timer < OPIC_NUM_TIMERS );
    return this->in32le(
	    OPIC_TIMER0_CURRENT_COUNT_REG + 
	    OPIC_REG_ALIGN*timer*OPIC_TIMER_FIELDS );
}

INLINE void intctrl_t::set_current_task_priority( u32_t priority, word_t cpu )
{
    opic_cpu_priority_t p;
    p.x.priority = priority;
    this->out32le( OPIC_CPU_CURRENT_PRIORITY_REG(cpu), p.raw );
}

INLINE void intctrl_t::mask_source( word_t source )
{
    this->set_reg_mask( 
	    OPIC_SRC0_VECTOR_REG + source*OPIC_REG_ALIGN*OPIC_SOURCE_FIELDS,
	    1 );
}

INLINE void intctrl_t::unmask_source( word_t source )
{
    this->set_reg_mask( 
	    OPIC_SRC0_VECTOR_REG + source*OPIC_REG_ALIGN*OPIC_SOURCE_FIELDS,
	    0 );
}

INLINE void intctrl_t::mask_timer( word_t timer )
{
    this->set_reg_mask(
	    OPIC_TIMER0_VECTOR_REG + OPIC_REG_ALIGN*timer*OPIC_TIMER_FIELDS,
	    1 );
}

INLINE void intctrl_t::unmask_timer( word_t timer )
{
    this->set_reg_mask(
	    OPIC_TIMER0_VECTOR_REG + OPIC_REG_ALIGN*timer*OPIC_TIMER_FIELDS,
	    0 );
}

INLINE void intctrl_t::set_reg_mask( word_t reg, u32_t mask )
{
    opic_vector_priority_t t;

    t = this->get_vector_priority( reg );
    t.x.mask = mask;
    this->write_vector_priority( reg, t );
}

/**************************************************************************/

INLINE bool intctrl_t::is_timer_vector( word_t vector )
{
    return (vector >= intctrl_t::timer0_vec) &&
	(vector <= intctrl_t::timer_end_vec);
}

INLINE bool intctrl_t::is_source_vector( word_t vector )
{
    return (vector >= intctrl_t::source_start_vec) &&
	(vector <= this->last_vector);
}

INLINE void intctrl_t::set_cpu( word_t irq, word_t cpu )
{
    if( EXPECT_TRUE(this->is_source_vector(irq)) )
	this->set_source_cpu( irq - intctrl_t::source_start_vec, cpu );

    else if( this->is_timer_vector(irq) )
	this->set_timer_cpu( irq - intctrl_t::timer0_vec, cpu );

    else
	enter_kdebug( "unknown irq" );
}

INLINE void intctrl_t::mask( word_t irq )
{
    if( EXPECT_TRUE(this->is_source_vector(irq)) )
	this->mask_source( irq - intctrl_t::source_start_vec );

    else if( this->is_timer_vector(irq) )
	this->mask_timer( irq - intctrl_t::timer0_vec );

    else
	enter_kdebug( "unknown irq" );
}

INLINE bool intctrl_t::unmask( word_t irq )
{
    if( EXPECT_TRUE(this->is_source_vector(irq)) )
	this->unmask_source( irq - intctrl_t::source_start_vec );

    else if( this->is_timer_vector(irq) )
	this->unmask_timer( irq - intctrl_t::timer0_vec );

    else
	enter_kdebug( "unknown irq" );

    return false;
}

INLINE void intctrl_t::mask_and_ack( word_t irq )
{
    this->mask( irq );
    this->clear_eoi( get_current_cpu() );
}

INLINE void intctrl_t::ack( word_t irq )
{
    this->clear_eoi( get_current_cpu() );
}

#endif	/* __PLATFORM__OFPPC__OPIC_H__ */

