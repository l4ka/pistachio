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

bool kdebug_check_interrupt()
{
#if defined(CONFIG_KDB_INPUT_HLT)
    if (get_current_tcb() == get_kdebug_tcb())
	return true;
#endif

# if defined(CONFIG_KDB_BREAKIN)
    kdebug_check_breakin();
#endif
    return false;

}


DECLARE_CMD(cmd_show_tcb, root, 't', "showtcb",  "show thread control block");
DECLARE_CMD(cmd_show_tcbext, root, 'T', "showtcbext", "shows thread control block (extended)");

static inline msg_tag_t SECTION(SEC_KDEBUG) get_msgtag(tcb_t* tcb)
{
    msg_tag_t tag;
    tag.raw = tcb->get_mr(0);
    return tag;
}

void SECTION(SEC_KDEBUG) dump_tcb(tcb_t * tcb)
{
    printf("=== TCB: %p === ID: %p = %p/%p === PRIO: 0x%2x ===",
	   tcb, tcb->get_global_id().get_raw(),
	   tcb->get_local_id().get_raw(), tcb->get_utcb(),
	   get_current_scheduler()->get_priority(tcb));
#if !defined(CONFIG_SMP)
    printf("=====\n");
#else
    printf(" CPU: %d ===\n", tcb->get_cpu());
#endif
    printf("UIP: %p   queues: %c%c%c%c%s      wait : %wt:%-wt   space: %p\n",
	   tcb->get_user_ip(),
	   tcb->queue_state.is_set(queue_state_t::ready )	? 'R' : 'r',
	   tcb->queue_state.is_set(queue_state_t::send)		? 'S' : 's',
	   tcb->queue_state.is_set(queue_state_t::wakeup)	? 'W' : 'w',
	   tcb->queue_state.is_set(queue_state_t::late_wakeup)	? 'L' : 'l',
	   __PADSTRING__,
	   tcb->wait_list.next, tcb->wait_list.prev,
	   tcb->get_space());
    printf("USP: %p   tstate: %ws  ready: %wt:%-wt   pdir : %p\n",
	   tcb->get_user_sp(), tcb->get_state().string(),
	   tcb->ready_list.next, tcb->ready_list.prev,
	   tcb->pdir_cache);
    printf("KSP: %p   sndhd : %-wt  send : %wt:%-wt   pager: %t\n",
	   tcb->stack, tcb->send_head, tcb->send_list.next, tcb->send_list.prev,
	   TID(tcb->get_utcb() ? tcb->get_pager() : threadid_t::nilthread()));

    printf("total quant:    %wdus, ts length  :       %wdus, curr ts: %wdus\n",
	   (word_t)tcb->total_quantum, (word_t)tcb->timeslice_length,
	   (word_t)tcb->current_timeslice);
    printf("abs timeout:    %wdus, rel timeout:       %wdus\n",
	   (word_t)tcb->absolute_timeout,
	   tcb->absolute_timeout == 0 ? 0 :
	   (word_t)(tcb->absolute_timeout -
		    get_current_scheduler()->get_current_time()));
    printf("sens prio: %d, delay: max=%dus, curr=%dus\n",
	   tcb->sensitive_prio, tcb->max_delay, tcb->current_max_delay);
    printf("resources: %p [", (word_t) tcb->resource_bits);
    tcb->resources.dump (tcb);
    printf("]");
    printf("   flags: %p [", (word_t) tcb->flags);
    printf("%c", (tcb->flags.is_set (tcb_t::has_xfer_timeout)) 		? 'T' : 't');
    printf("]\n");

    printf("partner: %t, saved partner: %t, saved state: %s, scheduler: %t\n",
	   TID(tcb->get_partner()), TID(tcb->get_saved_partner ()),
	   tcb->get_saved_state ().string (), TID(tcb->scheduler));
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
    space_t * space = NULL; // dummy space_t
#warning misuse of uninitialized space_t
    word_t val = get_hex("tcb/tid", (word_t)space->get_tcb(kdb.kdb_param), "current");

    if (val == ABORT_MAGIC)
	return NULL;

    if (!space->is_tcb_area((addr_t)val) &&
	(val != (word_t)get_idle_tcb()))
    {
	threadid_t tid;
	tid.set_raw(val);
	val = (word_t)space->get_tcb(tid);
    }
    return (tcb_t*) addr_to_tcb ((addr_t) val);

}

CMD(cmd_show_tcb, cg)
{
    tcb_t * tcb = get_thread ("tcb/tid/name");
    if (tcb)
	dump_tcb(tcb);
    return CMD_NOQUIT;
}



CMD(cmd_show_tcbext, cg)
{
    tcb_t * tcb = get_thread ("tcb/tid/name");
    if (tcb)
    {
	dump_tcb(tcb);
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
