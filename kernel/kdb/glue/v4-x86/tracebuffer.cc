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

#if defined(CONFIG_TRACEBUFFER)

#define TB_WRAP	50
extern void list_tp_choices (void);

#if defined(CONFIG_TBUF_PERFMON)
#define IF_PERFMON(a...) a
#else
#define IF_PERFMON(a...)
#endif

static inline int SECTION(SEC_KDEBUG) strlen(const char* p) { int i=0; while (*(p++)) i++; return i; };


DECLARE_CMD_GROUP (tracebuf);

word_t tbufcnt = 0;

class tbuf_dumper_t
{
public:
    static const word_t max_filters = 4;

private:   
    word_t id[max_filters];
    tcb_t *tcb[max_filters];
    word_t typemask;
    word_t cpumask;

    bool cpu_pass(tracerecord_t *t)
	{
	    return ((cpumask & (1UL << t->cpu)) != 0);
	}

    bool type_pass(tracerecord_t *t)
	{
	    return (typemask & ((t->ktype << 16) | t->utype));
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
	    if (tcb[0] == NULL || ((word_t) tcb[0] == (t->thread & KTCB_MASK)))
		return true;
    
	    for (word_t i=1; i < max_filters; i++)
	    {
		if ((word_t) tcb[i] == (t->thread & KTCB_MASK))
		    return true;
		if (tcb[i] == NULL)
		    return false;
	    }
	    return false;
	}


public:
    
    tbuf_dumper_t()
	{ invalidate_filters(); }
    
    void invalidate_filters()
	{
	    for (word_t i=0; i < max_filters; i++)
	    {
		id[i] = NULL;
		tcb[i] = NULL;
	    }
	    cpumask = typemask = ~0UL;

	}
    
    void dump_filters()
	{
	    printf("\tCPU:      [%x]\n", cpumask);
	    printf("\tTypemask: [%x]\n", typemask);
	    
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
    void set_typemask(word_t mask) {  typemask = mask; }
    
    void set_id(word_t idx, word_t id)
	{ 
	    ASSERT(idx < max_filters);
	    this->id[idx] = id;
	}
    
    void set_tcb(word_t idx, tcb_t *t)
	{ 
	    ASSERT(idx < max_filters);
	    tcb[idx] = t;
	}
    
    bool pass(tracerecord_t *t)
	{ return cpu_pass(t) && type_pass(t) && id_pass(t) && tcb_pass(t); }


    void dump(word_t start, word_t count, word_t size)
	{
	    word_t num, index;
	    tracerecord_t * rec;
	    tracebuffer_t * tracebuffer = get_tracebuffer ();
	    space_t *kspace = get_kernel_space();
	    
	    struct {
		word_t tsc;
		word_t pmc0;
		word_t pmc1;
	    } old[CONFIG_SMP_MAX_CPUS], sum = { 0, 0, 0 };

	    for (word_t cpu = 0; cpu < CONFIG_SMP_MAX_CPUS; cpu++)
		old[cpu].tsc = old[cpu].pmc0 = old[cpu].pmc1 = 0;

   
	    for (num = 0, index = start; count--; index++)
	    {
		if (index >= size) index = 0;
		rec = tracebuffer->tracerecords + index;

		if (!pass(rec))
		    continue;

		word_t cpu = rec->cpu;
	
		num++;

		if (! old[cpu].tsc)
		{
		    old[cpu].tsc = rec->tsc;
		    IF_PERFMON (old[cpu].pmc0 = rec->pmc0);
		    IF_PERFMON (old[cpu].pmc1 = rec->pmc1);
		}

		threadid_t tid;
		if (rec->is_kernel_event ())
		{
		    tcb_t *tcb = addr_to_tcb((addr_t) rec->thread);
	    
		    if (!kspace->is_tcb_area (tcb) && tcb != get_idle_tcb())
			tcb = kspace->get_tcb (threadid ((word_t) tcb));
	    
		    tid = tcb->get_global_id ();
		}
		else
		    tid = threadid (rec->thread);

		printf ("%6d %01d %04x %c %3d %wt ", index, rec->cpu, rec->get_type 
			(), rec->is_kernel_event () ? 'k' : 'u', rec->id, tid.get_raw ());

		word_t tscdelta = rec->tsc - old[cpu].tsc;
		
		if (tscdelta > 1000000)
		    printf("%-4uM ", tscdelta / 1000000);
		else if (tscdelta > 1000)
		    printf("%-4uK ", tscdelta / 1000);
		else
		    printf("%-5u ", tscdelta);
		
#if defined(CONFIG_TBUF_PERFMON)
		word_t pmcdelta0 = rec->pmc0-old[cpu].pmc0;
		word_t pmcdelta1 = rec->pmc1-old[cpu].pmc1;
		
		if (pmcdelta0 > 1000000)
		    printf("%-4uM ", pmcdelta0 / 1000000);
		else if (pmcdelta0 > 1000)
		    printf("%-4uK ", pmcdelta0 / 1000);
		else
		    printf("%-5u ", pmcdelta0);

		if (pmcdelta1 > 1000000)
		    printf("%-4uM   ", pmcdelta1 / 1000000);
		else if (pmcdelta1 > 1000)
		    printf("%-4uK   ", pmcdelta1 / 1000);
		else
		    printf("%-5u   ", pmcdelta1);

#endif
		
		sum.tsc  += (rec->tsc - old[cpu].tsc);
		IF_PERFMON (sum.pmc0 += (rec->pmc0 - old[cpu].pmc0));
		IF_PERFMON (sum.pmc1 += (rec->pmc1 - old[cpu].pmc1));

		old[cpu].tsc = rec->tsc;
		IF_PERFMON (old[cpu].pmc0 = rec->pmc0);
		IF_PERFMON (old[cpu].pmc1 = rec->pmc1);

		static char tb_str[256];
		word_t idx = 0;
		char *src = (char*) rec->str, *dst = tb_str;
		bool mapped = true;
		
		if (rec->is_kernel_event ())
		{
		    // Trust kernel strings to be ok
		    idx = 0;
		    while ((*src != 0) && (idx++ < (sizeof (tb_str) - 1)))
		    {
			*dst++ = *src++;
			if (idx % TB_WRAP == 0)
			{
			    bool fid = (*(dst-1) == '%');
			    if (fid) dst-=1;
			    *dst++ = '\n'; *dst++ = '\t';
			    *dst++ = '\t'; *dst++ = '\t';
			    *dst++ = '\t'; *dst++ = '\t';
			    *dst++ = '\t' ; *dst++ = ' ';
			    idx+=8;
			    if (fid) *dst++ = '%';
			}
		    }
		}
		else
		{
		    // For user strings we attempt to look up the string in
		    // the space of the thread.  We don't really bother too
		    // much if this does not work.

		    space_t * space = get_current_space ();
		    tcb_t * tcb = space->get_tcb (tid);

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
		    addr_t p = (addr_t) rec->str;
		    char c;

		    // Safely copy string into kernel
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
			    idx+=8;
			    if (fid) *dst++ = '%';
			}
			// Turn '%s' into '%p' (i.e., avoid printing arbitrary
			// user strings).
			if (( *dst == 's') &&
			    ( *(dst-1) == '%' || 	
			      ( *(dst-2) == '%' &&		      
				((*(dst-1) >= '0' && *(dst-1) <= '9') 
				 || *(dst-1) == 'w' || *(dst-1) == 'l' || *(dst-1) == '.'))))
			    *dst = 'p';
		    
		    }
		    
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
    
	    printf ("---------------------------------"
		    IF_PERFMON ("--------------------") "\n");  
	    printf ("Mask %08x  %10d" IF_PERFMON ("%10d %10d") "%d entries\n", 
		    tracebuffer->mask, sum.tsc, IF_PERFMON (sum.pmc0, sum.pmc1,)  num);
    

	}


};
    
tbuf_dumper_t tbuf_dumper;


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
    memset (get_tracebuffer ()->tracerecords, 0,
	    TRACEBUFFER_SIZE - sizeof (tracerecord_t));
    get_tracebuffer ()->current = 0;
    return CMD_NOQUIT;
}


/*
 * Reset counters.
 */

DECLARE_CMD (cmd_tb_reset_ctr, tracebuf, 'R', "resetctr", "Reset counters");

CMD (cmd_tb_reset_ctr, cg)
{
    tracebuffer_t * tracebuffer = get_tracebuffer ();

    for (word_t i = 0; i < 8; i++)
	tracebuffer->counters[i] = 0;

    return CMD_NOQUIT;
}


/*
 * Dump counters.
 */

DECLARE_CMD (cmd_tb_dump_ctr, tracebuf, 'c', "counters", "Dump counters");

CMD (cmd_tb_dump_ctr, cg)
{
    tracebuffer_t * tracebuffer = get_tracebuffer ();

    for (word_t i = 0; i < 8; i++)
	printf ("Counter %d = %10d\n", i, tracebuffer->counters[i]);

    return CMD_NOQUIT;
}


/*
 * Apply filter for tracebuffer events.
 */

word_t get_type_mask()
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
	mask = 0xffffffff & ~(TP_DEFAULT | TP_DETAIL << 16);
	break;
    case 'd':
	mask = 0xffffffff & ~(TP_DETAIL << 16);
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
    tracebuffer_t * tracebuffer = get_tracebuffer ();
    tbufcnt++;
    
    tracebuffer->mask = get_type_mask();
    return CMD_NOQUIT;
}


/*
 * Dump current tracebuffer.
 */

DECLARE_CMD (cmd_tb_dump, tracebuf, 'd', "dump", "Dump tracebuffer");

CMD (cmd_tb_dump, cg)
{ 
    word_t start, size, count, chunk;
   
    tracebuffer_t * tracebuffer = get_tracebuffer ();

    if (! tracebuffer->is_valid ())
    {
        printf("Bad tracebuffer signature at %p [%p]\n",
	       (word_t) (&tracebuffer->magic), tracebuffer->magic);
        return CMD_NOQUIT;
    }  

    if (tracebuffer->current == 0)
    {
	printf ("No records\n");
	return CMD_NOQUIT;
    }

    count = (TRACEBUFFER_SIZE / sizeof (tracerecord_t)) - 1;
    start = tracebuffer->current / sizeof (tracerecord_t);
    if (tracebuffer->tracerecords[count - 1].tsc == 0)
    {
	count = start;
	start = 0;
    }
    chunk = (count < 32) ? count : 32;
    size = count;

    switch (get_choice ("Dump tracebuffer", "All/Region/Top/Bottom", 'b'))
    {
    case 'a': 
	break;
    case 'r': 
	start = get_dec ("From record", 0);
	if (start >= count)
	    start = size - 1;
	// Fallthrough
    case 't': 
	count = get_dec ("Record count", chunk);
	if (count > size)
	    count = size;
	break;
    case 'b':
    default: 
	count = get_dec ("Record count", chunk);
	if (count > size)
	    count = size;
	start = (count <= start) ? start - count : start + size - count;
	break;
    } 

    printf ("\nRecord P  Type  TP   %ws Cyclecount "
	    IF_PERFMON (" Perfctr0  Perfctr1 ")
	    " Event\n", "Thread");
    
    tbuf_dumper.dump(start, count, size);
	
    return CMD_NOQUIT;
}


/*
 * Dump current tracebuffer (default values).
 */

DECLARE_CMD (cmd_tb_dump_def, root, 'Y', "tracebuffer dump",
	     "Dump tracebuffer (default values");


CMD (cmd_tb_dump_def, cg)
{
    word_t start, size, count;
   
    tracebuffer_t * tracebuffer = get_tracebuffer ();

    if (! tracebuffer->is_valid ())
    {
        printf("Bad tracebuffer signature at %p [%p]\n",
	       (word_t) (&tracebuffer->magic), tracebuffer->magic);
        return CMD_NOQUIT;
    }  

    if (tracebuffer->current == 0)
    {
	printf ("No records\n");
	return CMD_NOQUIT;
    }
    
    count = 64;
    start = tracebuffer->current / sizeof (tracerecord_t) - count;
    size = (TRACEBUFFER_SIZE / sizeof (tracerecord_t)) - 1;
    
    tbuf_dumper.dump(start, count, size);

    return CMD_NOQUIT;
}

/*
 * Submenu for tracebuffer display filters.
 */


DECLARE_CMD (cmd_tb_cpu, tracebuf, 'C', "cpufilter", "CPU display filter");
CMD(cmd_tb_cpu, cg) 
{
#if defined(CONFIG_SMP)
    tbuf_dumper.set_cpumask(get_hex("Processor Filter", ~0UL, "all"));
#endif	    
    return CMD_NOQUIT;
}

/*
 * Apply filter for tracebuffer events.
 */

DECLARE_CMD (cmd_tb_events, tracebuf, 'F', "filter", "Record display filter");

CMD (cmd_tb_events, cg)
{	
    tbufcnt++;

    tbuf_dumper.set_typemask(get_type_mask());

    return CMD_NOQUIT;
}


DECLARE_CMD (cmd_tb_evt, tracebuf, 't', "tpfilter", "TP display filter");
CMD(cmd_tb_evt, cg) 
{
    for (word_t i=0; i < tbuf_dumper_t::max_filters; i++)
	tbuf_dumper.set_id(i, 0);
    
    for (word_t i=0; i < tbuf_dumper_t::max_filters; i++)
    {
	for (;;)
	{
	    tbuf_dumper.set_id(i, 0);
	    word_t id = get_dec ("Select TP", 0, "list");
	    if (id == 0)
	    {
		list_tp_choices ();
		continue;
	    }
	    else if (id == ABORT_MAGIC)
		return CMD_NOQUIT;
	    else if (id <= tp_list.size ())
		tbuf_dumper.set_id(i, id);
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
    for (word_t i=0; i < tbuf_dumper_t::max_filters; i++)
	tbuf_dumper.set_tcb(i, NULL);
    
    for (word_t i=0; i < tbuf_dumper_t::max_filters; i++)
    {
	tbuf_dumper.set_tcb(i, get_thread ("tcb/tid/name"));
	if (get_choice ("More threads", "y/n", 'n') == 'n')
	    break;

    }
    return CMD_NOQUIT;
	
}

DECLARE_CMD (cmd_tb_showfilters, tracebuf, 's', "showfilters", "Show filters");
CMD(cmd_tb_showfilters, cg) 
{
    tracebuffer_t * tracebuffer = get_tracebuffer ();
    printf("Record  filters:\n");
    printf("\tTypemask: [%x]\n", tracebuffer->mask);
    printf("Display filters:\n");
    tbuf_dumper.dump_filters();
    return CMD_NOQUIT;
	
}

DECLARE_CMD (cmd_tb_zero, tracebuf, 'z', "zerofilter", "Invalidate filters");
CMD(cmd_tb_zero, cg) 
{
    tracebuffer_t * tracebuffer = get_tracebuffer ();
    tracebuffer->mask = 0xffffffff;
    
    tbuf_dumper.invalidate_filters();
    return CMD_NOQUIT;
}



#endif /* CONFIG_TRACEBUFFER */
