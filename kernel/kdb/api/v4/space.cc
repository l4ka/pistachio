/*********************************************************************
 *
 * Copyright (C) 2004,  National ICT Australia
 *
 * File path:     kdb/api/v4/space.cc
 * Description:   address space lists
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
 * $Id $
 *
 ********************************************************************/
#include <debug.h>
#include <kdb/kdb.h>
#include <kdb/cmd.h>
#include <kdb/input.h>
#include INC_API(space.h)
#include INC_API(tcb.h)

DECLARE_CMD(cmd_list_spaces, root, 'S', "listspaces",  "list all address spaces");

CMD(cmd_list_spaces, cg)
{
    spaces_list_lock.lock();
    space_t * walk = global_spaces_list;

    do {
	tcb_t * tcb_walk = walk->get_thread_list();

	printf("%p:", walk);

	if (tcb_walk) {
	    do {
#if !defined(CONFIG_SMP)
		printf(tcb_walk->queue_state.is_set(queue_state_t::ready) ? " %.wt" : " (%.wt)", tcb_walk);
#else
		printf(tcb_walk->queue_state.is_set(queue_state_t::ready) ? 
			       " %t:%d" : " (%t:%d)", tcb_walk, tcb_walk->get_cpu());
#endif
		tcb_walk = tcb_walk->thread_list.next;
	    } while ((tcb_walk != walk->get_thread_list()));
	}

	printf("\n");
	walk = walk->get_spaces_list().next;
    } while (walk != global_spaces_list);

    spaces_list_lock.unlock();
    return CMD_NOQUIT;
}

