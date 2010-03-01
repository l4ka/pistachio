/*********************************************************************
 *                
 * Copyright (C) 2008-2010,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/x32/logging.cc
 * Description:   
 *                
 * @LICENSE@
 *                
 * $Id:$
 *                
 ********************************************************************/
#include INC_API(schedule.h)
#include INC_GLUE(config.h)
#include INC_PLAT(perfmon.h)

#include <linear_ptab.h>
#include INC_GLUE_SA(logging.h)
#include <kdb/tracepoints.h>

u16_t logging_selector[MAX_LOGIDS][LOG_MAX_RESOURCES];
u8_t logging_logfile[LOG_SPACE_SIZE];

u32_t logging_virtual_log_time = 0;
u32_t logging_current_timer_tick = 0;


u32_t evt_enabled = ~0U;


DECLARE_KMEM_GROUP(kmem_log);
EXTERN_KMEM_GROUP(kmem_pgtab);


#define DEBUG_LOGGING

#if defined(DEBUG_LOGGING)


DECLARE_TRACEPOINT (LOG_PMC);

#define TRACE_LOG_PMC(x...)		TRACEPOINT (LOG_PMC, x)
#define TRACE_LOG_PMIPC(x...)		TRACEPOINT (LOG_PMIPC, x)


//#define TRACE_INIT_LOG_PMC(x...)	TRACE_INIT(x)
//#define TRACE_INIT_LOG(x...)		TRACE_INIT(x)
#define TRACE_INIT_LOG_PMC(x...)	
#define TRACE_INIT_LOG(x...)	

#else /* DEBUG_LOGGING */

#define TRACE_LOG_PMC(x...)	
#define TRACE_INIT_LOG_PMC(x...)	
#define TRACE_INIT_LOG(x...)	

#endif /* DEBUG_LOGGING */

FEATURESTRING ("logging");

// 2048 entries/logid for logging context switches
#define LOG_LD_PMC_LOG_ENTRIES		11
#define LOG_PMC_LOG_ENTRIES		(1 << LOG_LD_PMC_LOG_ENTRIES)
#define	LOG_LD_PMC_LOG_BYTES            (LOG_LD_PMC_LOG_ENTRIES + 2)
#define	LOG_PMC_LOG_BYTES		(1 << LOG_LD_PMC_LOG_BYTES)

extern u8_t _logid_selectors_start[];
extern u8_t _logid_selectors_end[];

addr_t user_log_area_start;
extern u8_t _kernel_log_area_start[];
extern u8_t _kernel_log_area_end[];
addr_t kernel_log_area_start = (addr_t) _kernel_log_area_start;
addr_t kernel_log_area_end = (addr_t) _kernel_log_area_end;


typedef struct {
    logfile_control_t pmc_control[MAX_LOGIDS];			//    0 ..  128		0 ..   80
    u32_t padding1[(1 << LOG_LD_PMC_LOG_BYTES) / 4 - 32];	//  
    u32_t pmc_buffer[MAX_LOGIDS][LOG_PMC_LOG_ENTRIES];         //   128 ..  384       80 ..  180 
} __attribute__((packed)) logfile_t;

logfile_t *logfile;

u32_t evtenable_end SECTION(".log.evtenable.LOG_RESOURCE_END")  = 0xDEADBEEF;
u32_t evtlist_end SECTION(".log.evtlist.LOG_RESOURCE_END")  = 0xDEADBEEF;

extern u8_t _start_text[];
extern u8_t _end_text[];

extern u8_t _log_evtenable_start[];
extern u8_t _log_evtenable_end[];
extern u8_t _log_evtlist_start[];
extern u8_t _log_evtlist_end[];

word_t *log_evtenable_start = (word_t *) _log_evtenable_start;
word_t *log_evtenable_end = (word_t *) _log_evtenable_end;

word_t *log_evtlist_start = (word_t *) _log_evtlist_start;
word_t *log_evtlist_end = (word_t *) _log_evtlist_end;


void toggle_events(word_t evt, bool all)
{
    u32_t *entry = (u32_t *) log_evtlist_start;
    u16_t *repository16 = (u16_t *) log_evtenable_start;
    u8_t *repository8 = (u8_t *) log_evtenable_start;

    while (entry[0] != 0xDEADBEEF)
    {
	u32_t entry_type = entry[0];
	u16_t *entry_start16 = (u16_t *) entry[1];
	u16_t *entry_stop16 = (u16_t *)  entry[2];
	u8_t *entry_start8 = (u8_t *) entry[1];
	u8_t *entry_stop8 = (u8_t *)  entry[2];

	if ( (word_t) entry_start16 >= (word_t) _start_text && 
	     (word_t) entry_start16 <= (word_t) _end_text &&
	     (word_t) entry_stop16  >= (word_t) _start_text && 
	     (word_t) entry_stop16  <= (word_t) _end_text &&
	     (all || evt == entry_type))
	{	
	    
	    
	    TRACE_INIT_LOG("Logging: patching %02x - %02x from %x, type %d\n", 
	       entry_start8, entry_stop8, repository8, entry_type);
	    
	    ASSERT(( ((word_t) entry_stop16 - (word_t) entry_start16) > 2));

	    u16_t jmptmp16;
	    jmptmp16 = entry_start16[0];

	    entry_start16[0] = (u16_t) (entry_stop16 - entry_start16 -2) << 8 | 0xeb;
	    
	    //TRACE_INIT_LOG("Logging: patched near jump %x\n", (word_t) entry_start16[0]);
	
	    for (u8_t *cur8 = entry_stop8 - 1 ; cur8 >= entry_start8; cur8--)
	    {
		
		/*
		 * patch insns backwards
		 */
		if (cur8 - entry_start8 == 1) 
		{
		    cur8--;
		    
		    u16_t *cur16 = (u16_t *) cur8;		    
		    
		    *cur16 = repository16[cur16 - entry_start16];
		    repository16[cur16 - entry_start16] = jmptmp16;

		    //TRACE_INIT_LOG("ins: %x rep: %x\n", *(cur8+1), 
		    //   repository8[(cur8+1) - entry_start8]);
		    //TRACE_INIT_LOG("ins: %x rep: %x\n", *cur8, 
		    //   repository8[cur8 - entry_start8]);

		}
		else 
		{
		    u8_t tmp8 = *cur8;
		    *cur8 = repository8[cur8 - entry_start8];
		    repository8[cur8 - entry_start8] = tmp8;
		    //TRACE_INIT_LOG("ins: %x rep: %x\n", *cur8, 
		    //   repository8[cur8 - entry_start8]);

		}
		
	    }
	    //TRACE_INIT_LOG("Logging: real patch finished\n");
	}
	else 
	{
	    TRACE_INIT_LOG("Logging: skipping %x - %x from %x, type %d\n", 
		   entry_start8, entry_stop8, repository8,  entry_type);
	}
	repository8 += entry_stop8 - entry_start8;
	repository16 += entry_stop16 - entry_start16;
	entry+=4;

    }
    
    printf("Event %d %s\n", evt, (evt_enabled & (1 << evt)) ? "disabled" : "enabled");
    evt_enabled ^= (1 << evt); 
	
	
    x86_mmu_t::flush_tlb(true);
    __asm__ __volatile__ ("wbinvd \n\t");
    
}


void add_logging_kmem(memdesc_t *kmem_md)
{
    word_t log_size = LOG_AREA_SIZE * CONFIG_SMP_MAX_CPUS;
    addr_t log_low =  addr_align(addr_offset(kmem_md->low(), kmem_md->size() - log_size), LOG_SPACE_SIZE);
    
    ASSERT((word_t) log_low >= (word_t) kmem_md->low() && 
           (word_t) log_low + log_size <= (word_t) kmem_md->low() + kmem_md->size());
    
    TRACE_INIT("\treserve %x - %x (sz %d) for log area\n", log_low, kmem_md->high(), log_size);

    get_kip()->memory_info.insert(memdesc_t::arch_specific, 0, false, log_low, kmem_md->high());
    kmem_md->set (kmem_md->type(), kmem_md->subtype(), kmem_md->is_virtual(), kmem_md->low(), (addr_t) ((word_t) log_low-1));

    user_log_area_start = log_low;

    word_t selector_page = (LOG_SPACE_SIZE) / X86_PAGE_SIZE ;
        
    TRACE_INIT("Log mapping user @ %x, selector page %d mapping @ %x \n",
               user_log_area_start, selector_page, 
               addr_offset(user_log_area_start,  (word_t) LOG_SPACE_SIZE));
    
    get_kip()->logging_log_region = user_log_area_start;
    get_kip()->logging_selector_page = selector_page; 


}

void init_logging_cpu(cpuid_t cpu)
{ 
   
    /*
     * Remap user log area
     */
    ASSERT(user_log_area_start);
    
    addr_t user_log_mapping = addr_offset(user_log_area_start, cpu * LOG_AREA_SIZE);
    addr_t kernel_log_mapping = kernel_log_area_start;

    for (word_t p = 0; p < LOG_AREA_SIZE; p += KERNEL_PAGE_SIZE)
    {
        pgent_t *pgent;
        pgent_t::pgsize_e pgsize;
        space_t *kspace = get_kernel_space();
        
        addr_t uaddr = addr_offset(user_log_mapping, p);
        addr_t kaddr = addr_offset(kernel_log_mapping, p);
        
        if (!kspace->lookup_mapping(kaddr, &pgent, &pgsize, cpu))
            panic("logging bug");
        
        
        //TRACEF("cpu %d u %x k %x phys %x virt %x sz %d\n", cpu, uaddr, kaddr,
        //     addr_offset(phys_to_virt(pgent->address(kspace, pgsize)), addr_mask(kaddr, page_mask (pgsize))), 
        //     addr_offset(pgent->address(kspace, pgsize), addr_mask(kaddr, page_mask (pgsize))), 
        //     KERNEL_PAGE_SIZE);
        
        /* Remove old cpulocal mappings */
        if (cpu != 0)
            kmem.free(kmem_pgtab, 
                      addr_offset(phys_to_virt(pgent->address(kspace, pgsize)), addr_mask(kaddr, page_mask (pgsize))), 
                      KERNEL_PAGE_SIZE);
                  
        pgent->set_entry(kspace, pgsize, uaddr, 7, 8, true);
    }
    
    
    x86_mmu_t::flush_tlb(true);
    
    TRACE_INIT("\tremapped log pages %p -> %p (CPU %d)\n", kernel_log_area_start, user_log_mapping, cpu);

    for (u8_t* p = (u8_t *)kernel_log_area_start; p < (u8_t *)kernel_log_area_end; p++)
	*p = 0;
   

    logfile = (logfile_t *) logging_logfile;
    TRACE_INIT("\tlog mapping kernel %x\n", logfile);

    //ENABLE_TRACEPOINT(LOG_PMC, ~0UL, ~0UL);
    setup_perfmon_cpu(cpu);
    

    /*
     * PMC configuration
     */
    TRACE_INIT_LOG_PMC("\t\tpmc: control = %x (%x), buffer = %x (%x), sizeof = %x\n",
		       
		       &logfile->pmc_control,  (word_t) &logfile->pmc_control -  (word_t) logfile,
		       &logfile->pmc_buffer,  (word_t) &logfile->pmc_buffer -  (word_t) logfile,
		       sizeof(logfile_control_t)
	);
    
    for (word_t logid = 0; logid < MAX_LOGIDS; logid++)
    {
	logging_selector[logid][LOG_RESOURCE_PMC] = 
	    (u16_t) ((word_t) &logfile->pmc_control[logid] - (word_t) logfile);


	logfile_control_t *ctrl = &logfile->pmc_control[logid];

	ctrl->X.current_offset = 
	    (u16_t) ((word_t) &logfile->pmc_buffer[logid][0] - (word_t) &logfile->pmc_control[logid]);
	ctrl->X.size_mask = LOG_LD_PMC_LOG_BYTES;		
	ctrl->X.event = 1;
	ctrl->X.current = 0;
	ctrl->X.counter = 1;		
	ctrl->X.overwrite_add = 0;		
	ctrl->X.correspondent = 0;
	ctrl->X.timestamp = LOG_TIMESTAMP_RDTSC;
	ctrl->X.padding = 0;

	
	TRACE_INIT_LOG_PMC("\t\t\t\tlog control dom %02d ofs=%p-%p size=%wd @ %p = %x\n",  
                           logid, 
                           ((word_t) &logfile->pmc_buffer[logid][0] - (word_t) logfile),
                           ((word_t) &logfile->pmc_buffer[logid][0] - (word_t) logfile) + (1UL << ctrl->X.size_mask),
                           (1UL << ctrl->X.size_mask), 
                           ctrl, ctrl->raw);

    }



}

void x86_log_resource_access(u32_t resource, u32_t current, u32_t correspondent, u32_t counter)
{														
										
    u32_t dummy;												


    __asm__ __volatile__(											
														
	/* eax: event/resource		*/									
	/* ebx: free			*/									
	/* ecx: current logid		*/									
	/* edx: corresponent		*/									
	/* esi: free			*/									
	/* edi: counter			*/									
	/* ebp: free			*/									
														
	"push	%%ebp							\n\t" /* save ebp		*/	
														
	/* calculate log control address */									
	"mov	%%ecx, %%esi						\n\t" /* current logid		*/	
	"mov	%%esi, %%ebp						\n\t" /* save current logid	*/	
	"shl	$("MKSTR(LOG_LD_MAX_RESOURCES)"), %%esi		\n\t" /* max_resources		*/	
	"add	%%eax, %%esi						\n\t" /* add resource		*/	
	"mov	$2, %%ebx						\n\t" /* get offset		*/	
	"movw	logging_selector(%%ebx,%%esi, 8), %%si		\n\t" /* get offset		*/	
	"lea	logging_logfile(%%esi), %%esi			\n\t" /* add base		*/	
														
	/* get current entry offset	*/									
	"mov	(%%esi), %%ecx					\n\t" /* load control		*/	
	"mov	%%ecx, %%ebx						\n\t" /* entry offset		*/	
	"shr	$16, %%ebx						\n\t" /* entry offset		*/	
	"lea	(%%esi,%%ebx, 1), %%ebx				\n\t" /* current entry addr	*/	
														
	/* eax: event/resource		*/									
	/* ebx: current entry addr	*/									
	/* ecx: control			*/									
	/* edx: corresponent		*/									
	/* esi: control addr		*/									
	/* edi: counter			*/									
	/* ebp: current logid		*/									
	
	/* Log at all ? */										
	"test	$("MKSTR(LOG_ALL_FLAGS)"),%%ecx			\n\t" /* all flags		 */		
	"jz	10f							\n\t" /* don't log		 */		

	
	/* Log event ? */										
	"bt	$("MKSTR(LOG_EVENT_BIT)"),%%cx			\n\t" /* log event bit		 */		
	"jnc	1f							\n\t" /* don't log		 */		
	"movl	%%eax, (%%ebx)						\n\t" /* add resource		 */	
	"add	$4, %%ebx						\n\t" /* next entry		 */	
														
	"1:								\n\t"					

	/* Log current ? */										
	"bt	$("MKSTR(LOG_CURRENT_BIT)"),%%cx			\n\t" /* log event bit		 */		
	"jnc	2f							\n\t" /* don't log		 */		
	"movl	%%ebp, (%%ebx)						\n\t" /* add resource		 */	
	"add	$4, %%ebx						\n\t" /* next entry		 */	
														
	"2:								\n\t"					

	/* Log correspondent ? */										
	"bt	$("MKSTR(LOG_CORRESPONDENT_BIT)"),%%cx		\n\t" /* log correspondent bit	*/		
	"jnc	3f							\n\t" /* don't log		*/	
	"mov	%%edx, (%%ebx)						\n\t" /* log correspondent	*/	
	"add	$4, %%ebx						\n\t" /* next entry		*/	

	"3:								\n\t"					

	/* eax: free			*/									
	/* ebx: current entry addr	*/									
	/* ecx: control			*/									
	/* edx: free			*/									
	/* esi: control addr		*/									
	/* edi: counter			*/									
	/* ebp: free			*/									

	/* Log timestamp ?	*/											
	"test	$("MKSTR(LOG_TIMESTAMP_ENABLED)"),%%cx		\n\t" /* timestamp bits off?	*/
	"jz	7f							\n\t" /* timestamp = 00		*/			
	/* timestamp = 01,10,11 */	 
	"test	$("MKSTR(LOG_TIMESTAMP_NOT_VIRTUAL)"), %%cx		\n\t" /* virtual timer tick	*/		
	"jnz	4f							\n\t" /* timestamp = 10 or 11	*/
	/* timestamp = 01	*/
	"mov	logging_virtual_log_time, %%eax				\n\t" /* get virtual log time	*/	
	"incl	logging_virtual_log_time				\n\t" /* inc virtual log time	*/	
	"jmp	6f							\n\t"					
	"4:								\n\t"					
	/* timestamp = 10,11	*/
	"test	$("MKSTR(LOG_TIMESTAMP_RDTSC_OR_TICK)"), %%cx	\n\t" /* tsc or tick		*/		
	"jz	5f							\n\t" /* timestamp = 10		*/
	/* timestamp = 11	*/
	"mov	logging_current_timer_tick, %%eax			\n\t" /* get timer tick		*/	
	"jmp	6f							\n\t"					
	"5:								\n\t"					
	/* timestamp = 10	*/
	"rdtsc								\n\t" /* read hw tsc		*/	
	"shrd	$("MKSTR(LOG_CPU_TSC_PRECISION)"), %%edx, %%eax \n\t" /* lower precision	*/	
	"6:								\n\t"					
	"mov	%%eax, (%%ebx)						\n\t" /* log time stamp counter	*/	
	"add	$4, %%ebx						\n\t" /* next entry		*/	

	"7:								\n\t"					
	
	/* eax: free			*/									
	/* ebx: current entry addr	*/									
	/* ecx: control			*/									
	/* edx: free			*/									
	/* esi: control addr		*/									
	/* edi: counter			*/									
	/* ebp: free			*/									

	/* Log counter ? */											
	"subl	$4, %%ebx						\n\t" /* sub entry		*/		
	"bt	$("MKSTR(LOG_COUNTER_BIT)"),%%cx			\n\t" /* log counter bit	*/		
	"jnc	9f							\n\t" /* don't log		*/	
	"addl	$4, %%ebx						\n\t" /* add entry		*/		
	"bt	$("MKSTR(LOG_ADD_BIT)"),%%cx			\n\t" /* add counter bit	*/
	"jnc	8f							\n\t" /* overwrite		*/ 
	"addl	%%edi, (%%ebx)						\n\t" /* add counter		*/
	"jmp	10f							\n\t" /* assume single entry	*/
	
 	"8:								\n\t"	 				
	"mov	%%edi, (%%ebx)						\n\t" /* save counter		*/

	/* Padding and size mask */											
 	"9:								\n\t"	 				
	"mov	$1, %%edx						\n\t" /*			*/
	"and	$("MKSTR(LOG_SIZE_N_PADDING_MASK)"), %%ecx		\n\t" /* size in cl, pad in ch	*/
	"shl	%%cl, %%edx						\n\t" /* 1 << size mask		*/	
	"add	%%ch, %%bl						\n\t" /* next entry+=pad	*/	
	"and	$("MKSTR(LOG_MAX_LOG_SIZE_MASK)"), %%edx		\n\t" /* max. allowed size	*/
 	"dec	%%edx							\n\t" /* (1 << size mask) - 1	*/	
	
	/* Wraparound next entry for counter  */										
	
	"mov	%%ebx, %%ebp						\n\t" /* current entry		*/	
	"add	$4, %%ebp						\n\t" /* current entry + 1	*/	
	"and	%%edx, %%ebp						\n\t" /*  ce + 1  & mask	*/	
	"not	%%edx							\n\t" /* ~mask			*/	
	"and	%%edx, %%ebx						\n\t" /* current entry & mask	*/	
	"or	%%ebp, %%ebx						\n\t" /* next entry in ebx	*/	
	
	/* eax: free			*/									
	/* ebx: current entry addr	*/									
	/* ecx: free			*/									
	/* edx: free			*/									
	/* esi: control addr		*/									
	/* edi: free			*/									
	/* ebp: free			*/									
														
														
	"sub	%%esi, %%ebx						\n\t" /* new entry offset	*/	
	"mov	%%bx, 2(%%esi)						\n\t" /* save entry offset	*/	

	"10:								\n\t"
	"pop	%%ebp							\n\t" /* restore ebp		*/	
														
	: /* output */												
														
	  "=a" (dummy),												
	  "=c" (dummy),												
	  "=d" (dummy),												
	  "=D" (dummy)												
														
	: /* input  */		 										
														
	  "0"  (resource),											
	  "1"  (current),											
	  "2"  (correspondent),											
	  "3"  (counter)										
														
	: "ebx", "esi"											
	);													
														

}

void log_pmc_counters( u32_t current, u32_t entry, 
		       u64_t tsc, u64_t uc, u64_t mqw, u64_t rb, 
		       u64_t mb,  u64_t mr, u64_t mlr, u64_t ldm )
{
    u32_t *current_idx = 
	(u32_t*) (u32_t) (((u32_t) &logfile->pmc_control[current]) 
			  + (u32_t) logfile->pmc_control[current].X.current_offset);
    

    if (!(logfile->pmc_control[current].raw))
	return;

    /* 0 -- tsc */
    *current_idx = (u32_t) tsc;
    
    /* encode exit/entry nr in lowermost bit of tsc */
    *current_idx &= 0xFFFFFFFE;
    *current_idx |= (entry & 0x1);
    
    L4_LOG_INC_LOG(current_idx, logfile->pmc_control);
    *current_idx = (u32_t) (tsc >> 32);
    L4_LOG_INC_LOG(current_idx, logfile->pmc_control);
    
    /* 8 -- uc */
    *current_idx = (u32_t) uc;
    L4_LOG_INC_LOG(current_idx, logfile->pmc_control);
    *current_idx = (u32_t) (uc >> 32);
    L4_LOG_INC_LOG(current_idx, logfile->pmc_control);

    /* 16 -- mqw */
    *current_idx = (u32_t) mqw;
    L4_LOG_INC_LOG(current_idx, logfile->pmc_control);
    *current_idx = (u32_t) (mqw >> 32);
    L4_LOG_INC_LOG(current_idx, logfile->pmc_control);

    /* 24 -- rb */
    *current_idx = (u32_t) rb;
    L4_LOG_INC_LOG(current_idx, logfile->pmc_control);
    *current_idx = (u32_t) (rb >> 32);
    L4_LOG_INC_LOG(current_idx, logfile->pmc_control);

    /* 32 -- mb */
    *current_idx = (u32_t) mb;
    L4_LOG_INC_LOG(current_idx, logfile->pmc_control);
    *current_idx = (u32_t) (mb >> 32);
    L4_LOG_INC_LOG(current_idx, logfile->pmc_control);

    /* 40 -- mr */
    *current_idx = (u32_t) mr;
    L4_LOG_INC_LOG(current_idx, logfile->pmc_control);
    *current_idx = (u32_t) (mr >> 32);
    L4_LOG_INC_LOG(current_idx, logfile->pmc_control);

    /* 48 -- mlr */
    *current_idx = (u32_t) mlr;
    L4_LOG_INC_LOG(current_idx, logfile->pmc_control);
    *current_idx = (u32_t) (mlr >> 32);
    L4_LOG_INC_LOG(current_idx, logfile->pmc_control);

    /* 56 -- ldm */
    *current_idx = (u32_t) ldm;
    L4_LOG_INC_LOG(current_idx, logfile->pmc_control);
    *current_idx = (u32_t) (ldm >> 32);
    L4_LOG_INC_LOG(current_idx, logfile->pmc_control);

    logfile->pmc_control[current].X.current_offset = ((u32_t)current_idx)-((u32_t)&logfile->pmc_control[current]);
}


LOG_CODE2(log_pmc, u32_t src_tcb, u32_t dst_tcb)
{
    tcb_t *src = (tcb_t *) src_tcb;
    tcb_t *dst = (tcb_t *) dst_tcb;

#if defined(CONFIG_DEBUG)
    if (src == get_kdebug_tcb() || dst == get_kdebug_tcb())
        return;
#endif

    word_t src_logid = src->sched_state.get_logid();
    word_t dst_logid = dst->sched_state.get_logid();
	
    if (src_logid == dst_logid)
	return;

    u64_t tsc=0, uc=0, mlr=0, mqw=0, rb=0, mb=0, mr=0, ldm=0;  
    
    tsc = x86_rdtsc();
    uc = x86_rdpmc(0);
    mlr = x86_rdpmc(1);
    mqw = x86_rdpmc(4);
    rb = x86_rdpmc(5);
    mb = x86_rdpmc(12);
    mr = x86_rdpmc(13);
    ldm = x86_rdpmc(14);

    TRACE_LOG_PMC("log pmc exit : logid=%x, logfile=%wx offset=%wx (%x %x %x)\n",
                  src_logid, 
                  ((u32_t*) (u32_t) (((u32_t) &logfile->pmc_control[src_logid]) 
                                     + (u32_t) logfile->pmc_control[src_logid].X.current_offset)),
                  (u32_t) logfile->pmc_control[src_logid].X.current_offset,
                  (u32_t) (tsc >> 32), (u32_t) (tsc & ~1), (u32_t) uc); 

    TRACE_LOG_PMC("log pmc entry: logid=%x, logfile=%wx offset=%wx (%x %x %x)\n",
                  dst_logid, 
                  ((u32_t*) (u32_t) (((u32_t) &logfile->pmc_control[dst_logid]) 
                                     + (u32_t) logfile->pmc_control[dst_logid].X.current_offset)),
                  (u32_t) logfile->pmc_control[dst_logid].X.current_offset,
                  (u32_t) (tsc >> 32), (u32_t) ((tsc & ~1) | 1), (u32_t) uc); 
    

    log_pmc_counters(src_logid, 0, tsc, uc, mqw, rb, mb, mr, mlr, ldm);
    log_pmc_counters(dst_logid, 1, tsc, uc, mqw, rb, mb, mr, mlr, ldm);



}

