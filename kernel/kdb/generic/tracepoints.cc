/*********************************************************************
 *                
 * Copyright (C) 2002-2003, 2007,  Karlsruhe University
 *                
 * File path:     kdb/generic/tracepoints.cc
 * Description:   Tracepoint related commands
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
 * $Id: tracepoints.cc,v 1.12 2003/09/24 19:05:11 skoglund Exp $
 *                
 ********************************************************************/
#include <kdb/tracepoints.h>
#include <kdb/kdb.h>
#include <kdb/cmd.h>
#include <kdb/input.h>
#include <kdb/linker_set.h>
#include INC_API(smp.h)

#if defined(CONFIG_TRACEPOINTS) || defined(CONFIG_TRACEBUFFER)

void SECTION(SEC_KDEBUG) list_tp_choices (void)
{
    word_t size = tp_list.size ();

    for (word_t i = 0; i <= size / 2; i++)
    {
	if (i == 0)
	    printf ("%3d - %28s", 0, "List choices");
	else
	    printf ("%3d - %28s", i, tp_list.get (i - 1)->name);

	if (i + (size / 2) < size)
	    printf ("%3d - %s\n", i + (size / 2) + 1,
		    tp_list.get (i + (size / 2))->name);
    }
    printf ("\n");
}

void init_tracepoints()
{
    tracepoint_t * tp;
    word_t id = 1;
    while ((tp = tp_list.next ()) != NULL)
	tp->id = id++;
}

/*
 * Linker set containing all tracepoints.
 */

DECLARE_SET (tracepoint_set);

tracepoint_list_t tp_list = { &tracepoint_set };

#endif

#if defined(CONFIG_TRACEPOINTS)

/*
 * Command group for tracepoint commands.
 */

DECLARE_CMD_GROUP (tracepoints);


/**
 * cmd_tracepoints: enable/disable/list tracepoints
 */
DECLARE_CMD (cmd_tracepoints, root, 'r', "tracepoints",
	     "enable/disable/list tracepoints");

CMD(cmd_tracepoints, cg)
{
    return tracepoints.interact (cg, "tracepoints");
}




/**
 * cmd_tp_list: list all tracepoints
 */
DECLARE_CMD (cmd_tp_list, tracepoints, 'l', "list", "list tracepoints");

CMD(cmd_tp_list, cg)
{
    tracepoint_t * tp;

    printf (" Num  %28s Enb  KDB  ", "Name");
#ifdef CONFIG_SMP
    for (int cpu = 0; cpu < CONFIG_SMP_MAX_CPUS; cpu++)
	printf(" CPU[%d]  ", cpu);
    printf("\n");
#else
    printf("Counter\n");
#endif

    tp_list.reset ();
    for (int i = 0; (tp = tp_list.next ()) != NULL; i++)
    {
	printf ("%3d   %28s  %c    %c ",
		i+1, tp->name, tp->enabled ? 'y' : 'n',
		tp->enter_kdb ? 'y' : 'n');
	for (int cpu = 0; cpu < CONFIG_SMP_MAX_CPUS; cpu++)
	    printf("%8d ", tp->counter[cpu]);
	printf("\n");
    }

    return CMD_NOQUIT;
}


/**
 * cmd_tp_enable: enable a tracepoint
 */
DECLARE_CMD (cmd_tp_enable, tracepoints, 'e', "enable", "enable tracepoint");

CMD(cmd_tp_enable, cg)
{
    for (;;)
    {
	word_t n = get_dec ("Tracepoint", 0, "list");
	if (n == 0)
	    list_tp_choices ();
	else if (n == ABORT_MAGIC)
	    return CMD_NOQUIT;
	else if (n <= tp_list.size ())
	{
	    tracepoint_t * tp = tp_list.get (n - 1);
#ifdef CONFIG_SMP
	    word_t cpu_mask = get_hex("Processor Mask", ~0UL, "all");
#else	    
	    word_t cpu_mask = ~0UL;
#endif
	    tp->enabled = cpu_mask;
	    tp->enter_kdb = get_choice ("Enter KDB", "y/n", 'y') == 'y' ?
		cpu_mask : 0;
	    tp->reset_counter ();
	    printf ("Tracepoint %s enabled\n", tp->name);
	    return CMD_NOQUIT;
	}
    }

    /* NOTREACHED */
    return CMD_NOQUIT;
}


/**
 * cmd_tp_disable: disable a tracepoint
 */
DECLARE_CMD (cmd_tp_disable, tracepoints, 'd', "disable",
	     "disable tracepoint");

CMD(cmd_tp_disable, cg)
{
    for (;;)
    {
	word_t n = get_dec ("Tracepoint", 0, "list");
	if (n == 0)
	    list_tp_choices ();
	else if (n == ABORT_MAGIC)
	    return CMD_NOQUIT;
	else if (n <= tp_list.size ())
	{
	    tracepoint_t * tp = tp_list.get (n - 1);
	    tp->enabled = tp->enter_kdb = false;
	    tp->reset_counter ();

	    printf ("Tracepoint %s disabled\n", tp->name);
	    return CMD_NOQUIT;
	}
    }

    /* NOTREACHED */
    return CMD_NOQUIT;
}


/**
 * cmd_tp_enable_all: enable all tracepoints
 */
DECLARE_CMD (cmd_tp_enable_all, tracepoints, 'E', "enableall",
	     "enable all tracepoints");

CMD(cmd_tp_enable_all, cg)
{
    tracepoint_t * tp;

    tp_list.reset ();
    while ((tp = tp_list.next ()) != NULL)
    {
	tp->enabled = ~0UL;
	tp->enter_kdb = 0;
	tp->reset_counter();
    }

    return CMD_NOQUIT;
}


/**
 * cmd_tp_disable_all: disable all tracepoints
 */
DECLARE_CMD (cmd_tp_disable_all, tracepoints, 'D', "disableall",
	     "disable all tracepoints");

CMD(cmd_tp_disable_all, cg)
{
    tracepoint_t * tp;

    tp_list.reset ();
    while ((tp = tp_list.next ()) != NULL)
	tp->enabled = tp->enter_kdb = 0;

    return CMD_NOQUIT;
}


/**
 * cmd_tp_reset: reset all tracepoint counters
 */
DECLARE_CMD (cmd_tp_reset, tracepoints, 'R', "reset", "reset counters");

CMD(cmd_tp_reset, cg)
{
    tracepoint_t * tp;

    tp_list.reset ();
    while ((tp = tp_list.next ()) != NULL)
	tp->reset_counter();
    
    return CMD_NOQUIT;
}

#endif /* CONFIG_TRACEPONTS */

