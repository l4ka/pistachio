/*********************************************************************
 *                
 * Copyright (C) 2002, 2003, 2005,  Karlsruhe University
 *                
 * File path:     kdb/arch/ia64/breakpoints.cc
 * Description:   IA-64 data and instruction breakpoints
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
 * $Id: breakpoints.cc,v 1.8 2005/06/06 16:01:07 skoglund Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include <kdb/kdb.h>
#include <kdb/input.h>

#include INC_ARCH(pal.h)
#include INC_ARCH(breakpoint.h)
#include INC_GLUE(context.h)
#include INC_API(tcb.h)


void set_psr_bits (threadid_t thread, bool db);

static void init_numregs (void);
static cmd_ret_t set_breakpoint (word_t type);
static cmd_ret_t clr_breakpoint (word_t type);

static word_t num_iregs = 0;
static word_t num_dregs = 0;

static bool global_bp = false;


DECLARE_CMD_GROUP (breakpoints);


/**
 * IA-64 breakpoint management.
 */
DECLARE_CMD (cmd_breakpoints, root, 'b', "breakpoint",
	     "breakpoint management");

CMD (cmd_breakpoints, cg)
{
    return breakpoints.interact (cg, "breakpoint");
}


/**
 * Set data breakpoint.
 */
DECLARE_CMD (cmd_dbr_set, breakpoints, 'd', "dset", "set data breakpoint");

CMD (cmd_dbr_set, cg)
{
    return set_breakpoint ('d');
}

/**
 * Set instruction breakpoint.
 */
DECLARE_CMD (cmd_ibr_set, breakpoints, 'i', "iset",
	     "set instruction breakpoint");

CMD (cmd_ibr_set, cg)
{
    return set_breakpoint ('i');
}


static cmd_ret_t set_breakpoint (word_t type)
{
    ia64_exception_context_t * frame = 
	(ia64_exception_context_t *) kdb.kdb_param;

    if (num_iregs == 0)
	init_numregs ();

    word_t regs = type == 'i' ? num_iregs : num_dregs;
    word_t reg;

    // Get breakpoint register number
    do {
	printf ("Number pair (%d-%d): ", 0, regs-1);
	if ((reg = get_hex (NULL, regs, "")) == ABORT_MAGIC)
	    return CMD_NOQUIT;
    } while (reg >= regs);

    // Get breakpoint trigger address
    addr_t addr = (addr_t) get_hex ("Address", 0);
    if ((word_t) addr == ABORT_MAGIC)
	return CMD_NOQUIT;

    // Get privilege level
    breakpoint_t::priv_e priv = breakpoint_t::both;
    switch (get_choice ("Privilege", "User/Kernel/Both", 'b'))
    {
    case 'u': priv = breakpoint_t::user;   break;
    case 'k': priv = breakpoint_t::kernel; break;
    case 'b': priv = breakpoint_t::both;   break;
    }

    // Enable breakpoints.
    tcb_t * tcb = kdb.kdb_current;
    if (get_choice ("Context", "All threads/Single thread", 'a') == 'a')
    {
	tcb->resources.enable_global_breakpoint (tcb);
	global_bp = true;
    }
    else
    {
	tcb = get_thread ("Thread");
	ia64_exception_context_t * user_frame =
	    (ia64_exception_context_t *) tcb->get_stack_top () - 1;
	ia64_switch_context_t * kernel_frame =
	    (ia64_switch_context_t *) tcb->stack;

	if (global_bp)
	    tcb->resources.disable_global_breakpoint (tcb);
	global_bp = false;

	user_frame->ipsr.db = kernel_frame->psr.db = 1;

    }

    if (tcb == kdb.kdb_current)
	frame->ipsr.db = 1;

    // Set breakpoint registers
    if (type == 'i')
    {
	instr_breakpoint_t ibr (addr, ~0UL, priv, true);
	ibr.put (reg);
    }
    else
    {
	char acc = get_choice ("Access", "Read/Write/Both", 'b');
	data_breakpoint_t dbr (addr, ~0UL, priv,
			       acc == 'w' || acc == 'b',
			       acc == 'r' || acc == 'b');
	dbr.put (reg);
    }

    return CMD_NOQUIT;
}


/**
 * Disable data breakpoint.
 */
DECLARE_CMD (cmd_dbr_clr, breakpoints, 'D', "dclr",
	     "disable data breakpoint");

CMD (cmd_dbr_clr, cg)
{
    return clr_breakpoint ('d');
}

/**
 * Disable instruction breakpoint.
 */
DECLARE_CMD (cmd_ibr_clr, breakpoints, 'I', "iclr",
	     "disable instruction breakpoint");

CMD (cmd_ibr_clr, cg)
{
    return clr_breakpoint ('i');
}

static cmd_ret_t clr_breakpoint (word_t type)
{
    ia64_exception_context_t * frame = 
	(ia64_exception_context_t *) kdb.kdb_param;
    ia64_exception_context_t * user_frame =
	(ia64_exception_context_t *) kdb.kdb_current->get_stack_top () - 1;

    if (num_iregs == 0)
	init_numregs ();

    word_t regs = type == 'i' ? num_iregs : num_dregs;
    word_t reg;

    // Get breakpoint register number
    do {
	printf ("Number pair (%d-%d): ", 0, regs-1);
	if ((reg = get_hex (NULL, regs, "")) == ABORT_MAGIC)
	    return CMD_NOQUIT;
    } while (reg >= regs);

    // Disable breakpoint registers
    if (type == 'i')
    {
	instr_breakpoint_t ibr = get_ibr (reg);
	ibr.disable ();
	ibr.put (reg);
    }
    else
    {
	data_breakpoint_t dbr = get_dbr (reg);
	dbr.disable ();
	dbr.put (reg);
    }

    if (global_bp)
	kdb.kdb_current->resources.disable_global_breakpoint (kdb.kdb_current);
    global_bp = false;

    // Disable breakpoints
    frame->ipsr.db = user_frame->ipsr.db = 0;

    return CMD_NOQUIT;
}


/**
 * List all breakpoints
 */
DECLARE_CMD (cmd_bp_list, breakpoints, 'l', "list", "list breakpoints");

CMD (cmd_bp_list, cg)
{
    if (num_iregs == 0)
	init_numregs ();

    printf ("Instruction breakpoints:\n");
    for (word_t i = 0; i < num_iregs; i++)
    {
	instr_breakpoint_t ibr = get_ibr (i);
	printf ("  ibr[%d]%s = addr: %p,  mask: %p,  priv: %s  %s\n",
		i, i >= 10 ? "" : " ",
		ibr.get_address (), ibr.get_mask (),
		ibr.get_priv () == breakpoint_t::kernel ? "kernel, " :
		ibr.get_priv () == breakpoint_t::user ? "user,   " :
		ibr.get_priv () == breakpoint_t::both ? "both,   " :
		"unknown,",
		ibr.is_active () ? "active" : "inactive");
    }

    printf ("Data breakpoints:\n");
    for (word_t i = 0; i < num_dregs; i++)
    {
	data_breakpoint_t dbr = get_dbr (i);
	printf ("  dbr[%d]%s = addr: %p,  mask: %p,  priv: %s  %s\n",
		i, i >= 10 ? "" : " ",
		dbr.get_address (), dbr.get_mask (),
		dbr.get_priv () == breakpoint_t::kernel ? "kernel, " :
		dbr.get_priv () == breakpoint_t::user ? "user,   " :
		dbr.get_priv () == breakpoint_t::both ? "both,   " :
		"unknown,",
		dbr.is_active () ?
		dbr.is_read_match () ? dbr.is_write_match () ?
		"read/write" : "read" : "write" : "inactive");
    }

    return CMD_NOQUIT;
}


static void init_numregs (void)
{
    pal_status_e status;

    if ((status = pal_debug_info (&num_iregs, &num_dregs)) != PAL_OK)
    {
	printf ("Error: PAL_DEBUG_INFO => %d\n", status);
    }
}
