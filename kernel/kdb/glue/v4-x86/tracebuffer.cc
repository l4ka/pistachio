/*********************************************************************
 *                
 * Copyright (C) 2002-2003, 2007-2008,  Karlsruhe University
 *                
 * File path:     kdb/glue/v4-x86/tracebuffer.cc
 * Description:   Tracebuffer for PC99 platform
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
 ********************************************************************/
#include <debug.h>
#include <linear_ptab.h>
#include <generic/lib.h>
#include <kdb/kdb.h>
#include <kdb/input.h>
#include <kdb/tracepoints.h>
#include <kdb/tracebuffer.h>
#include INC_API(thread.h)
#include INC_API(tcb.h)
#include INC_GLUE(timer.h)

#if defined(CONFIG_TRACEBUFFER)

FEATURESTRING ("tracebuffer");

#define TB_WRAP	80
extern void list_tp_choices (void);

#if defined(CONFIG_TBUF_PERFMON)
#define IF_PERFMON(a...) a
#else
#define IF_PERFMON(a...)
#endif

extern word_t local_apic_cpu_mhz;

static inline int SECTION(SEC_KDEBUG) strlen(const char* p) { int i=0; while (*(p++)) i++; return i; };
extern void putc(const char c);


template<typename T> static void pmc_print(T pmc)
{
    T divisor = 0;
    int digits = 0, num = 0;

    const int width = 4;
    
    /* calculate number of digits */
    if (pmc == 0) 
	digits = 0;
    else
	for (divisor = 1, digits = 1; pmc/divisor >= 10; divisor *= 10, digits++);

    while (num < max(width - digits, 0))
    {
	putc('0');
	num++;
    }
    
    while (num < width)
    {
	ASSERT(divisor);
	char d = (pmc/divisor) % 10;
	putc(d + '0');
	
	divisor /= 10;
	num++;
    }

    if (digits > width)
    {
        putc('e');
	putc(digits-width + '0');
    }
    else
    {
	putc(' ');
	putc(' ');
    }

    putc(' ');
    
}

template<typename T> T pmc_delta(T cur, T old)
{
    return (cur >= old) ? cur - old : cur + (T) -1 - old;
}



DECLARE_CMD_GROUP (tracebuf);


class tbuf_handler_t
{
public:
    static const word_t max_filters = 4;

private:   
    word_t id[max_filters];
    tcb_t *tcb[max_filters];
    word_t typemask;
    word_t cpumask;
    u64_t tsc;
    
    bool cpu_pass(tracerecord_t *t)
	{
	    return ((cpumask & (1UL << t->cpu)) != 0);
	}

    bool type_pass(tracerecord_t *t)
	{
	    return (typemask & ((t->ktype << 16) | t->utype));
	}

    bool tsc_pass(tracerecord_t *t)
	{
	    if (tsc == 0) return true;
	    
	    u64_t ttsc = t->tsc;
	    return (ttsc >= tsc);
	}

    
    bool id_pass(tracerecord_t *t)
	{
	    if (id[0] == NULL || ((word_t) id[0] == t->id))
		return true;
	    
	    for (word_t i=1; i < max_filters; i++)
	    {
		if (id[i] == NULL)
		    return false;
		if ((word_t) id[i] == t->id)
		    return true;
	    }
	    
	    return false;
	}


    bool tcb_pass(tracerecord_t *t)
	{
	    tcb_t *rtcb = t->is_kernel_event() 
		? addr_to_tcb((addr_t) t->thread)
		: get_current_space()->get_tcb(threadid (t->thread));

	    if (tcb[0] == NULL || (tcb[0] == rtcb))
		return true;
	    
	    for (word_t i=1; i < max_filters; i++)
	    {
		if (tcb[i] == rtcb)
		    return true;
		if (tcb[i] == NULL)
		    return false;
	    }
	    return false;
	}


public:
    
    tbuf_handler_t()
	{ invalidate_filters(); }
    
    void invalidate_filters()
	{
	    for (word_t i=0; i < max_filters; i++)
	    {
		id[i] = NULL;
		tcb[i] = NULL;
	    }
	    cpumask = typemask = ~0UL;
	    tsc = 0;

	}
    
    void dump_filters()
	{
	    
	    printf("Record  filters:\n");
	    printf("\tTypemask: [%x]\n", get_tracebuffer()->mask);
	    
	    printf("Display filters:\n");
	    printf("\tCPU:      [%x]\n", cpumask);
	    printf("\tTypemask: [%x]\n", typemask);
	    printf("\tTSC:      [%x/%x]\n", (u32_t) (tsc >> 32), (u32_t) tsc);
	    printf("\tTracepoints: \n");
	    for (word_t i=0; i < max_filters; i++)
	    {
		if (id[i] == NULL)
		    break;
		printf("\t\t%2d: %8d %s\n", i, id[i], tp_list.get(id[i]-1)->name);
	    }    
	    
	    printf("\tTCBs: \n");
	    for (word_t i=0; i < max_filters; i++)
	    {
		if (tcb[i] == NULL)
		    break;
		printf("\t\t%2d: %8t\n", i, (tcb_t *) tcb[i]);
	    }    
    


	}
    
    void set_cpumask(word_t mask) {  cpumask = mask; }
    word_t get_cpumask() { return this->cpumask; }
    
    void set_typemask(word_t mask) {  typemask = mask; }
    word_t get_typemask() { return this->typemask; }
    
    void set_tsc(u64_t t) {  tsc = t; }
    u64_t get_tsc() { return this->tsc; }

    void set_id(word_t idx, word_t id)
	{ 
	    ASSERT(idx < max_filters);
	    this->id[idx] = id;
	}
    
    word_t get_id(word_t idx)
	{ 
	    ASSERT(idx < max_filters);
	    return this->id[idx];
	}
    

    void set_tcb(word_t idx, tcb_t *t)
	{ 
	    ASSERT(idx < max_filters);
	    tcb[idx] = t;
	}
    
    bool pass(tracerecord_t *t)
	{ return cpu_pass(t) && type_pass(t) && id_pass(t) && tsc_pass(t) && tcb_pass(t); }

    
    /* Tbuf handling */
    void set_tbuf_typemask(word_t mask)
	{ get_tracebuffer()->mask = mask; }

    word_t get_tbuf_typemask()
	{ return get_tracebuffer()->mask; }
    
    
    word_t get_tbuf_size()
	{ return (TRACEBUFFER_SIZE / sizeof (tracerecord_t)) - 1; }
    
    word_t get_tbuf_current()
	{ return get_tracebuffer()->current / sizeof (tracerecord_t); }

    void reset_tbuf()
	{ 
	    memset (get_tracebuffer ()->tracerecords, 0,
		    TRACEBUFFER_SIZE - sizeof (tracerecord_t));
	    get_tracebuffer ()->current = 0;

	}
    
    void reset_tbuf_counters()
	{ 
	    tracebuffer_t * tracebuffer = get_tracebuffer ();
	    
	    for (word_t i = 0; i < 8; i++)
		tracebuffer->counters[i] = 0;

	}
    
    void dump_tbuf_counters()
	{
	    
	    tracebuffer_t * tracebuffer = get_tracebuffer ();
	    
	    for (word_t i = 0; i < 8; i++)
		printf ("Counter %d = %10d\n", i, tracebuffer->counters[i]);
	    
	}


    word_t find_tbuf_start(word_t end, word_t count, word_t size)
	{ 
    
	    word_t start, num;
	    
	    for (start = end, num = 0; num < size && count; start--, num++)
	    {
		if (start > size) start = size;
		
		tracerecord_t *rec = get_tracebuffer()->tracerecords + start;
		if (!pass(rec))
		{
		    //if (rec->tsc && !tsc_pass(rec))
		    //break;
		    //else
		    continue;
		}		
		count--;
	    }
	    return start;
	}
    
    word_t find_tbuf_end(word_t start, word_t count, word_t size)
	{ 
	    word_t end, num;
		
	    for (end = start, num = 0; num < size && count; end++, num++)
	    {
		if (end > size) end = 0;
		
		if (!pass(get_tracebuffer()->tracerecords + end))
		    continue;
		
		count--;
	    }
	    return end;
	}
    
    bool is_tbuf_valid()
	{
	    tracebuffer_t * tracebuffer = get_tracebuffer ();

	    if (! tracebuffer->is_valid ())
	    {
		printf("Bad tracebuffer signature at %p [%p]\n",
		       (word_t) (&tracebuffer->magic), tracebuffer->magic);
		return false;
	    }  
	    
	    if (tracebuffer->current == 0)
	    {
		printf ("No records\n");
		return false;
	    }
	    
	    return true;

	}
    

    
    void dump_tbuf(word_t start, word_t count, word_t size, bool header = true)
	{
	    word_t num, index;
	    tracerecord_t * rec;
	    tracebuffer_t * tracebuffer = get_tracebuffer ();
	    bool printed = false;
	    space_t * space = get_current_space ();

	    struct {
		word_t tsc;
		u64_t pmc0;
		u64_t pmc1;
	    } old[CONFIG_SMP_MAX_CPUS], sum = { 0, 0, 0 };

	    for (word_t cpu = 0; cpu < CONFIG_SMP_MAX_CPUS; cpu++)
		old[cpu].tsc = old[cpu].pmc0 = old[cpu].pmc1 = 0;

	    if (header)
		printf ("\nRecord P Type     TP %ws   TSC " IF_PERFMON (" PMC0  PMC1 ") "  Event\n", 
			"Thread");
	    
	    bool current_reached = false;
	    for (num = 1, index = start; count--; index++)
	    {
		if (index >= size) index = 0;
		rec = tracebuffer->tracerecords + index;

		if (!pass(rec))
		    continue;
		
		word_t cpu = rec->cpu;
		
		if (((++num % 4000) == 0) && get_choice ("Continue", "y/n", 'y') == 'n')
		    break;
		
		if (header && !current_reached && (index >= get_tbuf_current() + 1))
		{
		    current_reached = true;
		    printf ("------------------- Current ---------------"
			    IF_PERFMON ("--------------------") "\n");  
		}
		
		if (!old[cpu].tsc)
		{
		    old[cpu].tsc = rec->tsc;
		    IF_PERFMON (old[cpu].pmc0 = rec->pmc0);
		    IF_PERFMON (old[cpu].pmc1 = rec->pmc1);
		}

		u64_t c_delta = pmc_delta(rec->tsc, old[cpu].tsc);

		if (!header && !c_delta)
		    continue;
			
		printed = true;
	
		tcb_t * tcb;
		threadid_t tid;
		
		if (rec->is_kernel_event ())
		{
		    tcb = addr_to_tcb((addr_t) rec->thread);
		    tid = tcb->get_global_id();
		    
		    printf ("%6d %01d %04x %c %4d %wt ", index, rec->cpu, rec->get_type 
			    (), rec->is_kernel_event () ? 'k' : 'u', rec->id, tcb);
		}
		else
		{
		    tid = threadid (rec->thread);
		    tcb = space->get_tcb (tid);
		    printf ("%6d %01d %04x %c %4d %wt ", index, rec->cpu, rec->get_type 
			    (), rec->is_kernel_event () ? 'k' : 'u', rec->id, tid.get_raw ());

		}


#if defined(CONFIG_TBUF_PERFMON)
		// User and kernel instructions
		word_t pmcdelta0 = pmc_delta(rec->pmc0, (word_t) old[cpu].pmc0);
		word_t pmcdelta1 = pmc_delta(rec->pmc1, (word_t) old[cpu].pmc1);
		
		pmc_print(c_delta);
		pmc_print(pmcdelta0);
		pmc_print(pmcdelta1);
		
		sum.pmc0 += pmcdelta0;
		sum.pmc1 += pmcdelta1;
		
		old[cpu].pmc0 = rec->pmc0;
		old[cpu].pmc1 = rec->pmc1;

#else
		pmc_print(c_delta);
#endif
		
		sum.tsc  += (rec->tsc - old[cpu].tsc);

		old[cpu].tsc = rec->tsc;

		static char tb_str[256];
		word_t idx = 0;
		char *src = (char*) rec->str, *dst = tb_str;
		bool mapped = true;
		
		if (rec->is_kernel_event ())
		{
		    space = get_kernel_space();
		}
		else
		{
		    // For user strings we attempt to look up the string in
		    // the space of the thread.  We don't really bother too
		    // much if this does not work.

		    // Check if we seem to have a valid space and string pointer

		    if (tcb->get_global_id () != tid ||
			space->is_user_area ((addr_t) tcb->get_space ()) ||
			! space->is_user_area ((addr_t) rec->str))
		    {
	 		printf ("%p (%p, %p, %p, %p)\n", rec->str,
				rec->arg[0], rec->arg[1],
				rec->arg[2], rec->arg[3],
				rec->arg[4], rec->arg[5],
				rec->arg[6], rec->arg[7],
				rec->arg[8]);
			continue;
		    }
		    space = tcb->get_space ();

		}
		addr_t p = (addr_t) src;
		char c;

		while ((mapped = readmem (space, p, &c)) && (c != 0) && idx++ < (sizeof (tb_str) - 1))
		{
		    *dst++ = c;
		    p = addr_offset (p, 1);
		    if (idx % TB_WRAP == 0)
		    {
			bool fid = (*(dst-1) == '%');
			if (fid) dst-=1;
			*dst++ = '\n'; *dst++ = '\t';
			*dst++ = '\t'; *dst++ = '\t';
			*dst++ = '\t'; *dst++ = '\t';
			*dst++ = '\t'; *dst++ = ' ';
			*dst++ = ' ' ; *dst++ = ' ';
			idx+=10;
			if (fid) *dst++ = '%';
		    }
			
		    // Turn '%s' into '%p' (i.e., avoid printing arbitrary
		    // user strings).
		    if (!rec->is_kernel_event() &&  *dst == 's' &&
			( *(dst-1) == '%' || 	
			  ( *(dst-2) == '%' &&		      
			    ((*(dst-1) >= '0' && *(dst-1) <= '9') 
			     || *(dst-1) == 'w' || *(dst-1) == 'l' || *(dst-1) == '.'))))
			*dst = 'p';
		    
		    if (!mapped) *dst++ = 0;
		    
		}	    
		
		tb_str[idx] = 0;
		
		printf (tb_str, rec->arg[0], 
			rec->arg[1], rec->arg[2], 
			rec->arg[3], rec->arg[4], 
			rec->arg[5], rec->arg[6], 
			rec->arg[7], rec->arg[8]);	

		if (!mapped)
		    printf("[###]");

		// Append a newline and zero if needed and possible
		idx = strlen(tb_str);
		if( (idx < 1) || (tb_str[idx-1] != '\n' && tb_str[idx-1] != '\r') )
		    printf("\n"); 

	    }
    
	    if (header)
	    {
		printf ("-------------------------------------------"
			IF_PERFMON ("--------------------") "\n");  
		printf ("Mask %08x                 ", tracebuffer->mask);
		
		pmc_print(sum.tsc);
		IF_PERFMON (pmc_print(sum.pmc0));
		IF_PERFMON (pmc_print(sum.pmc1));
		
		printf(" %d entries", num-1);
	    }
	    
	    if (header || printed)
		printf("\n");

	}


};
 

tbuf_handler_t tbuf_handler;


void tbuf_dump (word_t count, word_t usec, word_t tp_id, word_t cpumask)
{
    word_t start, end, size;
    word_t old_tp_id[tbuf_handler_t::max_filters];
    word_t old_cpumask = tbuf_handler.get_cpumask();
    word_t old_typemask = tbuf_handler.get_typemask();
    u64_t old_tsc = tbuf_handler.get_tsc();
    word_t old_tbuf_typemask = tbuf_handler.get_tbuf_typemask();
    
    tbuf_handler.set_cpumask(cpumask);
    tbuf_handler.set_tbuf_typemask(~0ULL);
    tbuf_handler.set_typemask(~0ULL);
    
    for (word_t i=0; i < tbuf_handler_t::max_filters; i++)
    {
	old_tp_id[i] = tbuf_handler.get_id(i);
	tbuf_handler.set_id(i, 0);
    }
    tbuf_handler.set_id(0, tp_id);
    
    size  = tbuf_handler.get_tbuf_size();
    end   = tbuf_handler.get_tbuf_current();
    
    if (usec)
    {
	u64_t tsc = x86_rdtsc() - ((u64_t) usec * (u64_t) (get_timer()->get_proc_freq() / 1000));
	count = size;
	tbuf_handler.set_tsc(tsc);
    }
    else if (count == 0)
	count = size;
    
    start = tbuf_handler.find_tbuf_start(end, count, size);
    count = (end >= start) ? end - start : end + size - start;
    tbuf_handler.dump_tbuf(start, count, size, false);

    if (tp_id)
    {
	for (word_t i=0; i < tbuf_handler_t::max_filters; i++)
	    tbuf_handler.set_id(i, old_tp_id[i]);
    }
    tbuf_handler.set_cpumask(old_cpumask);
    tbuf_handler.set_typemask(old_typemask);
    tbuf_handler.set_tsc(old_tsc);
    tbuf_handler.set_tbuf_typemask(old_tbuf_typemask);
 
}



/*
 * Submenu for tracebuffer related commands.
 */

DECLARE_CMD (cmd_tracebuffer, root, 'y', "tracebuffer",
	     "Dump/manipulate tracebuffer");

CMD (cmd_tracebuffer, cg)
{
    return tracebuf.interact (cg, "tracebuffer");
}


/*
 * Reset tracebuffer.
 */

DECLARE_CMD (cmd_tb_reset, tracebuf, 'r', "reset", "Reset buffer");

CMD (cmd_tb_reset, cg)
{	
    tbuf_handler.reset_tbuf();
    return CMD_NOQUIT;
}


/*
 * Reset counters.
 */

DECLARE_CMD (cmd_tb_reset_ctr, tracebuf, 'R', "resetctr", "Reset counters");

CMD (cmd_tb_reset_ctr, cg)
{
    tbuf_handler.reset_tbuf_counters();
    return CMD_NOQUIT;
}


/*
 * Dump counters.
 */

DECLARE_CMD (cmd_tb_dump_ctr, tracebuf, 'c', "counters", "Dump counters");

CMD (cmd_tb_dump_ctr, cg)
{
    tbuf_handler.dump_tbuf_counters();
    return CMD_NOQUIT;
}


/*
 * Apply filter for tracebuffer events.
 */

word_t get_typemask()
{
    word_t mask = 0;
    
    switch (get_choice ("Keep which events",
			"All/Kernel/No tracept/no tpDetails/User/Mask", 'a'))
    {
    case 'a':
	mask = 0xffffffff;
	break;
    case 'k':
	mask = 0xffff0000;
	break;
    case 'n':
	mask = 0xfffcffff;
	break;
    case 'd':
	mask = 0x00010001;
	break;
    case 'u':
	mask = 0x0000ffff;
	break;
    case 'm':
	mask = get_hex ("Mask", 0xffffffff);
	break;
    }
    
    return mask;
    
}

DECLARE_CMD (cmd_tb_type_filter, tracebuf, 'f', "filter", "Record filter");

CMD (cmd_tb_type_filter, cg)
{	
    tbuf_handler.set_tbuf_typemask(get_typemask());
    return CMD_NOQUIT;
}


/*
 * Dump current tracebuffer.
 */

DECLARE_CMD (cmd_tb_dump, tracebuf, 'd', "dump", "Dump tracebuffer");

CMD (cmd_tb_dump, cg)
{ 
    word_t start, end, size, count;
   
    if (!tbuf_handler.is_tbuf_valid())
	return CMD_NOQUIT;
    
    size  = tbuf_handler.get_tbuf_size();
    count = 32;
    start = 0;
    end   = tbuf_handler.get_tbuf_current();
    
    switch (get_choice ("Dump tracebuffer", "All/Region/Top/Bottom", 'b'))
    {
    case 'a': 
	count = size;
	break;
    case 'r': 
	start = get_dec ("From record",  0);
	// Fall through
    case 't': 
	count = get_dec ("Record count", count);
	end = tbuf_handler.find_tbuf_end(start, count, size);
	if (count > size)  count = size; 
	break;
    case 'b':
    default: 
	count = get_dec ("Record count", count);
	if (count > size) count = size;
	start = tbuf_handler.find_tbuf_start(end, count, size);
	break;
    } 

    count = (end >= start) ? end - start : end + size - start;
    tbuf_handler.dump_tbuf(start, count, size);
	
    return CMD_NOQUIT;
}


/*
 * Dump current tracebuffer (default values).
 */

DECLARE_CMD (cmd_tb_dump_def, root, 'Y', "tracebuffer dump",
	     "Dump tracebuffer (default values");


CMD (cmd_tb_dump_def, cg)
{
    word_t start, end, size, count;
   
    if (!tbuf_handler.is_tbuf_valid())
	return CMD_NOQUIT;
    
    
    size  = tbuf_handler.get_tbuf_size();
    count = 64;
    end   = tbuf_handler.get_tbuf_current();
    start = tbuf_handler.find_tbuf_start(end, count, size);
    count = (end >= start) ? end - start : end + size - start;
    tbuf_handler.dump_tbuf(start, count, size);

    return CMD_NOQUIT;
}

/*
 * Submenu for tracebuffer display filters.
 */

#if defined(CONFIG_SMP)

DECLARE_CMD (cmd_tb_cpu, tracebuf, 'C', "cpufilter", "CPU display filter");
CMD(cmd_tb_cpu, cg) 
{
    tbuf_handler.set_cpumask(get_hex("Processor Filter", ~0UL, "all"));
    return CMD_NOQUIT;
}

#endif	    

/*
 * Apply filter for tracebuffer events.
 */

DECLARE_CMD (cmd_tb_events, tracebuf, 'F', "filter", "Record display filter");

CMD (cmd_tb_events, cg)
{	
    tbuf_handler.set_typemask(get_typemask());
    return CMD_NOQUIT;
}


DECLARE_CMD (cmd_tb_evt, tracebuf, 't', "tpfilter", "TP display filter");
CMD(cmd_tb_evt, cg) 
{
    for (word_t i=0; i < tbuf_handler_t::max_filters; i++)
	tbuf_handler.set_id(i, 0);
    
    for (word_t i=0; i < tbuf_handler_t::max_filters; i++)
    {
	for (;;)
	{
	    tbuf_handler.set_id(i, 0);
	    word_t id = get_dec ("Select TP", 0, "list");
	    if (id == 0)
	    {
		list_tp_choices ();
		continue;
	    }
	    else if (id == ABORT_MAGIC)
		return CMD_NOQUIT;
	    else if (id <= tp_list.size ())
		tbuf_handler.set_id(i, id);
	    else if (id >= TB_USERID_START)
		tbuf_handler.set_id(i, id);
	    break;
	}
	if (get_choice ("More events", "y/n", 'n') == 'n')
	    break;

    }
    return CMD_NOQUIT;
}


DECLARE_CMD (cmd_tb_tcb, tracebuf, 'T', "tcbfilter", "TCB display filter");
CMD(cmd_tb_tcb, cg) 
{
    for (word_t i=0; i < tbuf_handler_t::max_filters; i++)
	tbuf_handler.set_tcb(i, NULL);
    
    for (word_t i=0; i < tbuf_handler_t::max_filters; i++)
    {
	tbuf_handler.set_tcb(i, get_thread ("tcb/tid/name"));
	if (get_choice ("More threads", "y/n", 'n') == 'n')
	    break;

    }
    return CMD_NOQUIT;
	
}

DECLARE_CMD (cmd_tb_showfilters, tracebuf, 's', "showfilters", "Show filters");
CMD(cmd_tb_showfilters, cg) 
{
    tbuf_handler.dump_filters();
    return CMD_NOQUIT;
	
}

DECLARE_CMD (cmd_tb_zero, tracebuf, 'z', "zerofilter", "Invalidate filters");
CMD(cmd_tb_zero, cg) 
{
    tbuf_handler.invalidate_filters();
    return CMD_NOQUIT;
}



#endif /* CONFIG_TRACEBUFFER */
