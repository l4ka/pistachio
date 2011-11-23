/*********************************************************************
 *                
 * Copyright (C) 2008-2011,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/x32/logging.h
 * Description:   
 *                
 * @LICENSE@
 *                
 * $Id:$
 *                
 ********************************************************************/
#ifndef __GLUE__V4_X86__X32__LOGGING_H__
#define __GLUE__V4_X86__X32__LOGGING_H__


/**********************************************************************
 *                       accounting and logging stuff 
 **********************************************************************/
#define LOG_SPACE_SIZE		(255 * X86_PAGE_SIZE)
#define LOG_SELECTOR_SIZE       (  1 * X86_PAGE_SIZE)
#define LOG_AREA_SIZE		(LOG_SPACE_SIZE + LOG_SELECTOR_SIZE)


/*
 * Must be a power of 2
 */
#define LOG_LD_MAX_RESOURCES	    2
#define LOG_MAX_RESOURCES	    (1 <<  LOG_LD_MAX_RESOURCES)
#define LOG_RESOURCE_PMC            0  /* performance counters at CPU exit */

#define LOG_MAX_LOGIDS			32
#define LOG_NULL_LOGID			(0xFFFFFFFF)
#define LOG_IDLE_LOGID			(0)
#define LOG_ROOTSERVER_LOGID		(1)


#include INC_API(sktcb.h)


#define L4_LOG_MASK(idx, mask)	((u32_t) (idx) & (mask))

#define L4_LOG_INC_LOG(idx, ctrl)      \
    idx = (u32_t *) (L4_LOG_MASK((idx + 1), ((1UL << ctrl->X.size_mask) -1)) |\
                         L4_LOG_MASK(idx, (~((1UL << ctrl->X.size_mask) -1))))

#define L4_LOG_INC_LOGN(idx, ctrl, n)				\
    idx = (u32_t *) (L4_LOG_MASK((idx + n), ((1UL << ctrl->X.size_mask) -1)) |\
                         L4_LOG_MASK(idx, (~((1UL << ctrl->X.size_mask) -1))))



/****************************************************************************************************************
 *					Buffered variables
 ***************************************************************************************************************/
extern u32_t logging_current_timer_tick UNIT("cpulocal");
extern u32_t logging_virtual_log_time UNIT("cpulocal");

/****************************************************************************************************************
 *					Domain Selectors
 ***************************************************************************************************************/

extern u16_t logging_selector[MAX_LOGIDS][LOG_MAX_RESOURCES] UNIT("logsel");
extern u8_t logging_logfile[LOG_SPACE_SIZE] UNIT("logfile");

extern addr_t user_log_area_start;
extern addr_t kernel_log_area_start, kernel_log_area_end;
extern void add_logging_kmem(memdesc_t *kmem_md);

/****************************************************************************************************************
 *					Log File Control
 ***************************************************************************************************************/


/*
 * Log control register
 */

#define LOG_EVENT_BIT		5
#define LOG_CURRENT_BIT	        6
#define LOG_CORRESPONDENT_BIT       7
#define LOG_TIMESTAMP_BIT	        8
#define LOG_PADDING_BIT	       10
#define LOG_COUNTER_BIT	       12
#define LOG_ADD_BIT		       13
#define LOG_ALL_FLAGS	       0xFFE0	       


#define LOG_TIMESTAMP_OFF		0x0
#define LOG_TIMESTAMP_VIRTUAL	0x1
#define LOG_TIMESTAMP_RDTSC		0x2
#define LOG_TIMESTAMP_TICK		0x3

#define LOG_TIMESTAMP_ENABLED	(0x3 << LOG_TIMESTAMP_BIT)
#define LOG_TIMESTAMP_NOT_VIRTUAL	(0x2 << LOG_TIMESTAMP_BIT)
#define LOG_TIMESTAMP_RDTSC_OR_TICK	(0x1 << LOG_TIMESTAMP_BIT)

#define LOG_CPU_TSC_PRECISION	8

#define LOG_SIZE_MASK_MASK		(0x1F) 
#define LOG_MAX_LOG_SIZE_MASK	(32768 -1)
#define LOG_PADDING_MASK		(0x3 << LOG_PADDING_BIT)

#define LOG_SIZE_N_PADDING_MASK	(LOG_SIZE_MASK_MASK | LOG_PADDING_MASK)


typedef union{

    struct{
	unsigned size_mask		:5;
	unsigned event			:1;	// 0 = off,  1 = log
	unsigned current		:1;	// 0 = off,  1 = log
	unsigned correspondent		:1;	// 0 = off,  1 = on
	unsigned timestamp		:2;	// 0 = off,  1 = virtual, 2 = tsc, 3 = tick
	unsigned padding		:2;	// 0 = +0, 1= +4, 2=+8, 3=+12 bytes
	unsigned counter		:1;	// 0 = off,  1 = on
	unsigned overwrite_add		:1;	// 0 = overwrite,  1 = add
	unsigned reserved		:2;

	u16_t current_offset;
	
    } __attribute__((packed)) X;
    
    u32_t raw;
    
} __attribute__((packed)) logfile_control_t;


void init_logging_cpu(cpuid_t current_cpu);


void toggle_events(word_t evt, bool all=false);
    
extern word_t *log_evtbptr_start;
extern word_t *log_evtbptr_end;
#define NOP1   ".byte 0x90					\n\t"	/* 1-byte no-op		*/
#define NOP2   ".byte 0x89,0xf6					\n\t"	/* 2-byte no-op		*/
#define NOP3   ".byte 0x8d,0x76,0x00				\n\t"	/* 3-byte no-op		*/
#define NOP4   ".byte 0x8d,0x74,0x26,0x00			\n\t"	/* 4-byte no-op		*/
#define NOP6   ".byte 0x8d,0xb6,0x00,0x00,0x00,0x00		\n\t"	/* 6-byte no-op		*/
#define NOP7   ".byte 0x8d,0xb4,0x26,0x00,0x00,0x00,0x00	\n\t"	/* 7-byte no-op		*/

#define DECLARE_LOG_EVT2(evt_type, log_function, arg1, arg2)			\
	 __DECLARE_LOG_EVT2(evt_type, log_function, arg1, arg2)		


#define __DECLARE_LOG_EVT2(evt_type, log_function, arg1, arg2)			\
__asm__ __volatile__(									\
    "1:						\n\t"					\
    "push %1					\n\t"	/* push arg		*/	\
    "push %0					\n\t"	/* push arg		*/	\
    "lea "#log_function", %%ecx		\n\t"	/* load call addr	*/	\
    "call  *%%ecx				\n\t"	/* transfer control	*/	\
    "pop %0					\n\t"	/* pop arg		*/	\
    "pop %1					\n\t"	/* pop arg		*/	\
    "2:						\n\t"					\
    ".section .log.evtenable."#evt_type", \"a\"	\n\t"					\
    ".byte 0x8d,0xb6,0x00,0x00,0x00,0x00	\n\t"	/* 6-byte no-op		*/	\
    ".byte 0x8d,0xb6,0x00,0x00,0x00,0x00	\n\t"	/* 6-byte no-op		*/	\
    ".previous					\n\t"					\
    ".section .log.evtlist."#evt_type", \"ax\"	\n\t"	/* insn.-point list	*/	\
    ".align 16					\n\t"					\
    ".long "#evt_type"				\n\t"	/* evt type		*/	\
    ".long 1b					\n\t"	/* store backpointer	*/	\
    ".long 2b					\n\t"	/* store backpointer	*/	\
    ".long 0					\n\t"	/* store backpointer	*/	\
    ".previous                           	\n\t"					\
    :: "r" (arg1), "r" (arg2)  : "ecx");


#define LOG_CODE2(name, arg1, arg2)			\
extern "C" void name (arg1, arg2);			\
extern "C" void name##handler(arg1, arg2);		\
void name##_wrapper()					\
{							\
    __asm__ __volatile__(                               \
        ".global "#name "		\n"		\
	"\t.type "#name",@function	\n"		\
	#name":				\n"		\
	"movl 4(%esp),  %ecx		\n"		\
	"push  %eax			\n"		\
	"movl 12(%esp),  %eax		\n"		\
	"push %edx			\n"		\
	"push  %eax			\n"		\
	"push  %ecx			\n"		\
	"call "#name"handler		\n"		\
	"pop %ecx			\n"		\
	"pop %eax			\n"		\
	"pop %edx			\n"		\
	"pop %eax			\n"		\
	"ret				\n"		\
	);						\
}							\
void name##handler(arg1, arg2)



#define DECLARE_LOG_EVT1(evt_type, log_function, arg1)				\
	 __DECLARE_LOG_EVT1(evt_type, log_function, arg1)		


#define __DECLARE_LOG_EVT1(evt_type, log_function, arg1)				\
__asm__ __volatile__(									\
    "1:						\n\t"					\
    "push %0					\n\t"	/* pop arg		*/	\
    "lea "#log_function", %%ecx			\n\t"	/* load call addr	*/	\
    "call  *%%ecx				\n\t"	/* transfer control	*/	\
    "pop %0					\n\t"	/* pop arg		*/	\
    "2:						\n\t"					\
    ".section .log.evtenable."#evt_type", \"a\"	\n\t"					\
    ".byte 0x8d,0xb6,0x00,0x00,0x00,0x00	\n\t"	/* 6-byte no-op		*/	\
    ".byte 0x8d,0x74,0x26,0x00			\n\t"	/* 4-byte no-op		*/	\
    ".previous					\n\t"					\
    ".section .log.evtlist."#evt_type", \"ax\"	\n\t"	/* insn.-point list	*/	\
    ".align 16					\n\t"					\
    ".long "#evt_type"				\n\t"	/* evt type		*/	\
    ".long 1b					\n\t"	/* store backpointer	*/	\
    ".long 2b					\n\t"	/* store backpointer	*/	\
    ".long 0					\n\t"	/* store backpointer	*/	\
    ".previous					\n\t"					\
    :: "r" (arg1) : "ecx");


#define LOG_CODE1(name, arg1)				\
void name (arg1);					\
extern "C" void name##handler(arg1);			\
extern "C" void name##_wrapper()			\
{							\
    __asm__ (						\
        ".global "#name "		\n"		\
	"\t.type "#name",@function	\n"		\
	#name":				\n"		\
	"movl 4(%esp),  %ecx		\n"		\
	"push  %eax			\n"		\
	"push %edx			\n"		\
	"push  %ecx			\n"		\
	"call "#name"handler		\n"		\
	"pop %ecx			\n"		\
	"pop %edx			\n"		\
	"pop %eax			\n"		\
	"ret				\n"		\
	);						\
}							\
void name##handler(arg1)


#define LOG_PMC(src, dst)               DECLARE_LOG_EVT2(LOG_RESOURCE_PMC, log_pmc, src, dst)


#endif /* !__GLUE__V4_X86__X32__LOGGING_H__ */
