/*********************************************************************
 *                
 * Copyright (C) 2002-2004, 2006-2007,  Karlsruhe University
 *                
 * File path:     kdb/api/v4/input.cc
 * Description:   Version 4 specific input functions
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
 * $Id: input.cc,v 1.14 2006/10/02 16:04:27 reichelt Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include <kdb/input.h>
#include <kdb/kdb.h>
#include <kdb/console.h>
#include INC_API(space.h)
#include INC_API(tcb.h)


/**
 * Prompt for an address space using PROMPT and return the space
 * pointer.  Input value can be a TCB address or a physical or virtual
 * space pointer.

 * @param prompt	prompt
 *
 * @return pointer to space
 */
space_t SECTION(SEC_KDEBUG) * get_space (const char * prompt)
{
    space_t * dummy = kdb.kdb_current->get_space ();
    space_t * space;
    addr_t val;

    if (!dummy)
	dummy = get_kernel_space ();

    val = (addr_t) get_hex (prompt, (word_t) dummy,
			    "current");

    tcb_t * tidtcb = dummy->get_tcb (threadid ((word_t) val));

    if (dummy->is_tcb_area (val))
    {
	// Pointer into the TCB area
	tcb_t * tcb = addr_to_tcb (val);
	space = tcb->get_space ();
    }
    else if (dummy->is_tcb_area(tidtcb) && 
	     tidtcb->myself_global == threadid ((word_t) val))
    {
	// A valid thread ID
	space = tidtcb->get_space ();
    }
    else if (dummy->is_user_area (val))
    {
	// Pointer in lower memory area.  Probably a physical address.
	val = phys_to_virt (val);
	space = (space_t *) val;
    }
    else
    {
	// Hopefuly a valid space pointer
	space = (space_t *) val;
    }

    return space;
}



static const char * thread_names[] = {
    "nil_thrd", "irq_", "idlethrd", "sigma0", "sigma1", "roottask", 0
};

static inline char lowercase (char c)
{
    return c >= 'A' && c <= 'Z' ? c + 'a' - 'A' : c;
}

static inline int thread_match (const char * str)
{
    for (word_t i = 0; thread_names[i] != 0; i++)
    {
	for (word_t j = 0; thread_names[i][j] == str[j]; j++)
	    if (str[j] == 0)
		return i+1;
    }

    return 0;
}


/**
 * Prompt for a thread using PROMPT and return the tcb pointer.  Input
 * value can be a TCB address or a thread id.
 *
 * @param prompt	prompt
 *
 * @return pointer to tcb
 */
tcb_t SECTION (SEC_KDEBUG) * get_thread (const char * prompt)
{
    space_t * dummy = kdb.kdb_current->get_space ();
    const word_t nsize = sizeof (word_t) * 2;

    printf ("%s [current]: ", prompt ? prompt : "Thread");

    word_t val = 0;
    word_t num = 0;
    word_t len = 0;
    word_t version_char = 0;
    bool break_loop = false;
    char c, r;

    while (! break_loop && 
	   (len < nsize || version_char != 0))
    {
	switch (r = c = getc ())
	{
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
	    num *= 16;
	    num += c - '0';
	    putc (r);
	    len++;
	    break;

	case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
	    c += 'a' - 'A';
	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
	    num *= 16;
	    num += c - 'a' + 10;
	    putc (r);
	    len++;
	    break;

	case 'x': case 'X':
	    // Allow "0x" prefix
	    if (len == 1 && num == 0)
	    {
		putc (r);
		len--;
	    }
	    break;

	case 'v': case 'V':
	    if (len == 0)
		break;
	    putc (r);
	    val = num << L4_GLOBAL_VERSION_BITS;
	    num = 0;
	    version_char = len++;
	    break;

	case 'S': case 'N': case 'R': case 'I':
	case 's': case 'n': case 'r': case 'i':
	    if (len == 0)
	    {
		// Trying to type in name of thread.
		char buf[9];
		int i = 0;

		putc (r);
		buf[i++] = lowercase (r);
		buf[i] = 0;

		while (! thread_match (buf) && r != KEY_RETURN)
		{
		    switch (r = getc ())
		    {
		    case '\b':
			printf ("\b \b");
			buf[i--] = 0;
			break;
		    case '\e':
			printf ("\n");
			return (tcb_t *) NULL;
		    case KEY_RETURN:
			break;
		    default:
			if (i == 8)
			    break;
			putc (r);
			buf[i++] = lowercase (r);
			buf[i] = 0;
			break;
		    }
		}

		// Check which thread name the user gave

		word_t ubase = get_kip ()->thread_info.get_user_base ();
		break_loop = true;

		switch (thread_match (buf))
		{
		case 1: // Nilthrad
		    val = threadid_t::nilthread ().get_raw ();
		    break;
	
		case 2: // IRQ thread
		    val = threadid_t::irqthread	(get_dec ()).get_raw ();
		    break;

		case 3: // Idle thread
		    val = (word_t) get_idle_tcb ();
		    break;

		case 4: // Sigma0
		    val = threadid_t::threadid (ubase, 1).get_raw ();
		    break;

		case 5: // Sigma1
		    val = threadid_t::threadid (ubase + 1, 1).get_raw ();
		    break;

		case 6: // Roottask
		    val = threadid_t::threadid (ubase + 2, 1).get_raw ();
		    break;

		default: // (invalid)
		    while (i-- > 0)
			printf ("\b \b");
		    break_loop = false;
		    break;
		}

		break;
	    }
	    // Not typing string.  Fallthrough.

	case '\b':
	    // Backspace
	    if (len > 0)
	    {
		printf ("\b \b");
		num /= 16;
		len--;
		if (len == version_char)
		{
		    version_char = 0;
		    num = val >> L4_GLOBAL_VERSION_BITS;
		}
	    }
	    break;

	case KEY_RETURN:
	    if (len == 0)
	    {
		// Use default value
		printf ("current");
		val = (word_t) kdb.kdb_current;
	    }
	    else
	    {
		len = 0;
		if (version_char != 0)
		    val |= num & ((1UL << L4_GLOBAL_VERSION_BITS) - 1);
		else
		    val = num;
	    }
	    break_loop = true;
	    break;
	}
    }

    if (len == nsize)
	val = num;

    printf ("\n");

    if (dummy->is_tcb_area ((addr_t) val) || 
	(addr_t) val == (addr_t) get_idle_tcb())
	return addr_to_tcb ((addr_t) val); 
    else
	return dummy->get_tcb (threadid (val));
}
