/*********************************************************************
 *
 * Copyright (C) 2002-2004,  Karlsruhe University
 *
 * File path:     kdb/api/v4/tcb.cc
 * Description:   tcb dumping
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
 * $Id: tcb.cc,v 1.45 2005/06/03 15:54:04 joshua Exp $
 *
 ********************************************************************/
#include <debug.h>
#include <kdb/kdb.h>
#include <kdb/cmd.h>
#include <kdb/input.h>
#include INC_API(tcb.h)
#include INC_API(schedule.h)
#include INC_API(cpu.h)

#if defined(CONFIG_IS_64BIT)
#define __PADSTRING__ "        "
#else
#define __PADSTRING__ ""
#endif

u16_t dbg_get_current_cpu()
{
    return get_current_cpu();
}

word_t dbg_get_current_tcb()
{
    return (word_t) get_current_tcb();
}

#if defined(CONFIG_TBUF_PERFMON_ENERGY)
DECLARE_TRACEPOINT(ENERGY_TIMER);
#endif

bool kdebug_check_interrupt()
{

#if defined(CONFIG_TBUF_PERFMON_ENERGY)
    scheduler_t *scheduler = get_current_scheduler();

    static u64_t UNIT("cpulocal") last_second_tick = 0;
    
	if (scheduler->get_current_time() > (last_second_tick + 50000000))
	{
	    TRACEPOINT(ENERGY_TIMER, "Energy TIMER @ %d",
		       (word_t) (scheduler->get_current_time() / 1000));
	    
	    last_second_tick = scheduler->get_current_time();
	    
	    if (get_current_cpu() == 0)
		for (cpuid_t cpu = 0; cpu < cpu_t::count; cpu++)
		{
		    // Energy timer in the last 1.1 seconds
		    tbuf_dump(2, 0, __tracepoint_ENERGY_TIMER.id, (1 << cpu));
		    // Lx syscalls in the last 10 milliseconds
		    //tbuf_dump(10, , 110, (1 << cpu));
		}
	    

	}
#endif

#if defined(CONFIG_KDB_INPUT_HLT)
    if (get_current_tcb() == get_kdebug_tcb())
	return true;
#endif

    kdebug_check_breakin();
    return false;

}


DECLARE_CMD(cmd_show_tcb, root, 't', "showtcb",  "show thread control block");
DECLARE_CMD(cmd_show_tcbext, root, 'T', "showtcbext", "shows thread control block (extended)");

static inline msg_tag_t SECTION(SEC_KDEBUG) get_msgtag(tcb_t* tcb)
{
    msg_tag_t tag = tcb->get_mr(0);
    return tag;
}

void SECTION(SEC_KDEBUG) dump_tcb(tcb_t * tcb, bool extended)
{
    sched_ktcb_t *sched_state = &tcb->sched_state;
    
    printf("=== TCB: %p === ID: %p = %p/%p",
	   tcb, tcb->get_global_id().get_raw(),
	   tcb->get_local_id().get_raw(), tcb->get_utcb());
    sched_state->dump_priority();
#if !defined(CONFIG_SMP)
    printf("=====");
#else
    printf(" CPU: %d ===", tcb->get_cpu());
#endif
    printf(" ===\n");

    printf("UIP: %p   queues: %c%c%c%c%s      ",
	   tcb->get_user_ip(),
	   tcb->queue_state.is_set(queue_state_t::ready )	? 'R' : 'r',
	   tcb->queue_state.is_set(queue_state_t::send)		? 'S' : 's',
	   tcb->queue_state.is_set(queue_state_t::wakeup)	? 'W' : 'w',
	   tcb->queue_state.is_set(queue_state_t::late_wakeup)	? 'L' : 'l',
	   __PADSTRING__);
    sched_state->dump_list1();
    printf("space: %p\n", tcb->get_space());
    printf("USP: %p   tstate: %ws  ", tcb->get_user_sp(), tcb->get_state().string());
    sched_state->dump_list2();
    printf("pdir : %p\n", tcb->pdir_cache);
    printf("KSP: %p   sndhd : %-wt  send : %wt:%-wt   pager: %t\n",
	   tcb->stack, tcb->send_head, tcb->send_list.next, tcb->send_list.prev,
	   TID(tcb->get_utcb() ? tcb->get_pager() : threadid_t::nilthread()));
    sched_state->dump(get_current_scheduler()->get_current_time());
    printf("resources: %p [", (word_t) tcb->resource_bits);
    tcb->resources.dump (tcb);
    printf("]");
    printf("   flags: %p [", (word_t) tcb->flags);
    printf("%c", (tcb->flags.is_set (tcb_t::has_xfer_timeout)) 		? 'T' : 't');
    printf("%c", (tcb->flags.is_set (tcb_t::schedule_in_progress))      ? 'S' : 's');
#if defined(CONFIG_X_CTRLXFER_MSG)
    printf("%c", (tcb->flags.is_set (tcb_t::kernel_ctrlxfer_msg))      ? 'K' : 'k');
#endif
    printf("]\n");
#if defined(CONFIG_X_CTRLXFER_MSG)
    tcb->dump_ctrlxfer_state(extended);
#endif
    printf("partner: %t, saved partner: %t, saved state: %s, scheduler: %t\n",
	   TID(tcb->get_partner()), TID(tcb->get_saved_partner ()),
	   tcb->get_saved_state ().string (), TID(tcb->sched_state.get_scheduler()));
}


void SECTION (SEC_KDEBUG) dump_utcb (tcb_t * tcb)
{
    printf ("\nuser handle:       %p  "
	    "cop flags:      %02x%s  "
	    "preempt flags:     %02x [%c%c%c]\n"
	    "exception handler: %t  "
	    "virtual sender: %t  "
	    "intended receiver: %t\n",
	    tcb->get_user_handle (), tcb->get_cop_flags (),
	    sizeof (word_t) == 8 ? "              " : "      ",
	    tcb->get_preempt_flags ().raw,
	    tcb->get_preempt_flags ().is_pending()  ? 'I' : '~',
	    tcb->get_preempt_flags ().is_delayed()  ? 'd' : '~',
	    tcb->get_preempt_flags ().is_signaled() ? 's' : '~',
	    TID (tcb->get_exception_handler ()),
	    TID (tcb->get_virtual_sender ()),
	    TID (tcb->get_intended_receiver ()));

    printf ("xfer timeouts:     snd (");
    time_t xfer = tcb->get_xfer_timeout_snd ();
    printf (xfer.is_never () ? "never" : "%s: %12dus",
	    xfer.is_period () ? "rel" : "abs",
	    xfer.get_microseconds ());
    printf (")\n                   rcv (");
    xfer = tcb->get_xfer_timeout_rcv ();
    printf (xfer.is_never () ? "never" : "%s: %12dus",
	    xfer.is_period () ? "rel" : "abs",
	    xfer.get_microseconds ());
    printf (")\n");
}


/**
 * Dumps a message and buffer registers of a thread in human readable form
 * @param tcb	pointer to thread control block
 */
static void SECTION(SEC_KDEBUG) dump_message_registers(tcb_t * tcb)
{
    for (int i = 0; i < IPC_NUM_MR; i++)
    {
	if (!(i % 8)) printf("\nmr(%02d):", i);
	printf(" %p", tcb->get_mr(i));
    }

    printf("\nMessage Tag: %d untyped, %d typed, label = %x, flags = %c%c%c%c\n",
           get_msgtag(tcb).get_untyped(), get_msgtag(tcb).get_typed(),
           get_msgtag(tcb).x.label,
           get_msgtag(tcb).is_error() ? 'E' : '-',
           get_msgtag(tcb).is_xcpu() ? 'X' : '-',
           get_msgtag(tcb).is_redirected() ? 'r' : '-',
           get_msgtag(tcb).is_propagated() ? 'p' : '-'
        );

    for (word_t i = 0; i < get_msgtag(tcb).get_typed();)
    {
	int offset = get_msgtag(tcb).get_untyped() + 1;
	msg_item_t item;

	item = tcb->get_mr(offset + i);
	if (item.is_map_item() || item.is_grant_item())
	{
	    fpage_t fpage ((fpage_t) {{ raw: tcb->get_mr(offset + i + 1)}} );
	    printf("%s item: snd base=%p, fpage=%p (addr=%p, sz=%x), %c%c%c\n",
                   item.is_map_item() ? "map" : "grant",
                   item.get_snd_base(),
                   fpage.raw, fpage.get_base(), fpage.get_size(),
                   fpage.mem.x.write	? 'W' : 'w',
                   fpage.mem.x.read	? 'R' : 'r',
                   fpage.mem.x.execute	? 'X' : 'x');
	    i+=2;
	}
	else if (item.is_string_item())
	{
	    printf("string item: len=%x, num=%d, cont=%d, cache=%d\n  ( ",
                   item.get_string_length(), item.get_string_ptr_count(),
                   item.is_string_compound(), item.get_string_cache_hints());
	    i++;

	    for (word_t j = 0; j < item.get_string_ptr_count(); j++, i++)
                printf("%p ", tcb->get_mr(offset + i));
	    printf(")\n");
	}
#if defined(CONFIG_X_CTRLXFER_MSG)
	else if (item.is_ctrlxfer_item())
	{
            
            if (tcb->flags.is_set(tcb_t::kernel_ctrlxfer_msg))
            {
                ctrlxfer_mask_t mask = tcb->get_fault_ctrlxfer_items(item.get_ctrlxfer_id());
                word_t id = item.get_ctrlxfer_id();

                printf( "ctrlxfer kernel msg fault %d mask %x\n", item.get_ctrlxfer_id(), (word_t) mask);

                id = lsb(mask);	
                
                do {
                    printf("\t id %d %s mask %x %x\n ", id, ctrlxfer_item_t::get_idname(id), 
                           ctrlxfer_item_t::fault_item((ctrlxfer_item_t::id_e) id).get_ctrlxfer_mask(), (word_t) mask);
                    mask -= id;
                    id = lsb(mask);	
                } while (mask);
                
                i+=1;
                
            }
            else
            {
                word_t mask = item.get_ctrlxfer_mask();
                word_t id = item.get_ctrlxfer_id();
                word_t num = 1, reg = 0;
                
                printf("ctrlxfer item: mask=%x, id=%d",  mask, id);
                
                while (mask && num < IPC_NUM_MR)
                {
                    if ((num-1) % 4 == 0) printf("\n\t");
                    while ((mask & 1) == 0) { mask >>= 1; reg++; } 
                    printf("%s: %p ", ctrlxfer_item_t::get_hwregname(id, reg),  tcb->get_mr(offset + i + num));
                    mask >>= 1; reg++; num++;
                }
                i += num;
            }
            printf("\n");
        }
#endif
	else
	{
	    printf("unknown item type (%p)\n", item.raw);
	    i++;
	}
    }
}

static void SECTION(SEC_KDEBUG) dump_buffer_registers(tcb_t * tcb)
{
    acceptor_t acc;
    fpage_t fpage;
    msg_item_t item;

    acc = tcb->get_br(0);
    fpage.raw = tcb->get_br(0);
    fpage.raw &= ~0xf; // mask out lowermost bits.

    for (word_t i = 0; i < IPC_NUM_BR; i++)
    {
	if (!(i % 8)) printf("\nbr(%02d):", i);
	printf(" %p", tcb->get_br(i));
    }

    printf("\nAcceptor: %p (%c)\n", acc.raw, acc.accept_strings() ? 'S' : 's');
    printf("  fpage :");
    if (fpage.is_nil_fpage())
	printf(" (NIL-FPAGE)\n");
    else if (fpage.is_complete_fpage())
	printf(" (COMPLETE-FPAGE)\n");
    else
	printf("  fpage=%p (addr=%p, sz=%p)\n",
	    fpage.raw, fpage.get_base(), fpage.get_size());

    if (acc.accept_strings())
    {
	word_t idx = 1;
	do
	{
	    item = tcb->get_br(idx);
	    printf("string item: len=%x, num=%d, compound=%d, "
		   "cache=%d, more_strings=%d\n  ( ",
		   item.get_string_length(), item.get_string_ptr_count(),
		   item.is_string_compound(), item.get_string_cache_hints(),
		   item.more_strings());
	    idx++;

	    for (word_t j = 0; j < item.get_string_ptr_count(); j++, idx++)
		    printf("%p ", tcb->get_br(idx));
	    printf(")\n");
	} while(item.more_strings() || item.is_string_compound());
    }
}

tcb_t SECTION(SEC_KDEBUG) * kdb_get_tcb()
{
    debug_param_t * param = (debug_param_t*)kdb.kdb_param;
    space_t *space = param->space;
    word_t val = get_hex("tcb/tid", (word_t) space, "current");

    if (val == ABORT_MAGIC)
	return NULL;

    if (!tcb_t::is_tcb((addr_t)val) &&
	(val != (word_t)get_idle_tcb()))
    {
	threadid_t tid;
	tid.set_raw(val);
	val = (word_t)tcb_t::get_tcb(tid);
    }
    return (tcb_t*) addr_to_tcb ((addr_t) val);

}

CMD(cmd_show_tcb, cg)
{
    tcb_t * tcb = get_thread ("tcb/tid/name");
    if (tcb)
	dump_tcb(tcb, false);
    return CMD_NOQUIT;
}



CMD(cmd_show_tcbext, cg)
{
    tcb_t * tcb = get_thread ("tcb/tid/name");
    if (tcb)
    {
	dump_tcb(tcb, true);
	if (tcb->get_utcb())
	{
	    dump_utcb(tcb);
	    dump_message_registers(tcb);
	    dump_buffer_registers(tcb);
	}
	else
	    printf("no valid UTCB\n");
    }
    return CMD_NOQUIT;
}
