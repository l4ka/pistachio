/*********************************************************************
 *                
 * Copyright (C) 2004, 2007-2008,  Karlsruhe University
 *                
 * File path:     kdb/api/v4/thread.cc
 * Description:   Kdebug stuff for V4 threads
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
 * $Id: thread.cc,v 1.5 2005/06/03 15:54:04 joshua Exp $
 *                
 ********************************************************************/
#include <kdb/tid_format.h>
#include INC_API(thread.h)
#include INC_API(tcb.h)


/* From generic/print.cc */

int print_hex (const word_t val,
	       int width,
	       int precision,
	       bool adjleft = false,
	       bool nullpad = false);
int print_string (const char * s,
		  const int width = 0,
		  const int precision = 0);
int print_hex_sep (const word_t val,
		   const int bits,
		   const char *sep);
int print_dec (const word_t val,
	       const int width = 0,
	       const char pad = ' ');
    

int print_tid (word_t val, word_t width, word_t precision, bool adjleft)
{
    tcb_t * tcb;
    threadid_t tid;
    space_t * dummy = NULL;

#if 0
    print_string ("<");
    print_dec (width);
    print_string (":");
    print_dec (precision);
    print_string (">");
#endif

    // If val is within TCB area, treat it as a tcb address.  If not,
    // treat it as a thread ID.

    if (dummy->is_tcb_area ((addr_t) val) || 
	addr_to_tcb((addr_t) val) == get_idle_tcb() || 
	addr_to_tcb((addr_t) val) == get_kdebug_tcb())
    {
	tcb = addr_to_tcb ((addr_t) val);
	tid = tcb->get_global_id ();
    }
    else
    {
	tid.set_raw (val);
	tcb = dummy->get_tcb (tid);
    }

    if (kdb_tid_format.X.human)
    {
	// Convert special thread IDs to human readable form
	if (tcb == get_idle_tcb ())
	    return print_string ("IDLETHRD", width, precision);

	if (tcb == get_kdebug_tcb())
	    return print_string ("KDBTHRD", width, precision);

	if (tid.is_nilthread ())
	    return print_string ("NIL_THRD", width, precision);

	if (tid.is_anythread())
	    return print_string ("ANY_THRD", width, precision);

	if (tid.is_interrupt ())
	{
	    print_string ("IRQ_");
	    return 4 + print_dec (tid.get_irqno(), width - 4, '0');
	}

	word_t base_id = tid.get_threadno () -
	    get_kip()->thread_info.get_user_base ();
	if (base_id < 3)
	{
	    const char *names[3] = { "SIGMA0", "SIGMA1", "ROOTTASK" };
	    return print_string (names[base_id], width, precision);
	}
    }

    // We're dealing with something which is not a special thread ID.

    word_t n = 0;
    bool f_both = TID_FORMAT_VALUE_BOTH == kdb_tid_format.X.value;
    bool f_gid = TID_FORMAT_VALUE_GID == kdb_tid_format.X.value;
    bool f_tcb = TID_FORMAT_VALUE_TCB == kdb_tid_format.X.value;
    bool f_ver = TID_FORMAT_VERSION_OFF != kdb_tid_format.X.version;

    if (f_gid || f_both)
    {
	// Getting a consistent output width with separators is pretty
	// much hopeless depending on the position of the separator,
	// additional hex characters become necessary

	if (TID_FORMAT_VERSION_INLINE == kdb_tid_format.X.version)
	{
	    // Do not separate version from threadno
	    if (kdb_tid_format.X.sep != 0)
		// Insert a separator into threadno
		n = print_hex_sep (tid.get_raw (),
				   kdb_tid_format.X.sep +
				   L4_GLOBAL_VERSION_BITS, ".");
	    else
		// No separator at all
		n = print_hex (tid.get_raw (), 0, sizeof (word_t) * 2);
	}
	else
	{
	    if (kdb_tid_format.X.sep != 0)
		// Insert a separator into threadno
		n = print_hex_sep (tid.get_threadno (),
				   kdb_tid_format.X.sep, ".");
	    else
		// Print threadno without separator
		n = print_hex (tid.get_threadno (),
			       f_both || f_ver ? 0 : width,
			       0, adjleft);

	    if (f_ver)
	    {
		// Add a separator between threadno and version
		n += print_string ("v");
//		print_dec (width); print_string (">");
		width -= width > n ? n : 0;
		n += print_hex (tid.get_version (),
				f_both ? 0 : width, 0, true);
	    }
	}
    }

    if (f_both)
	n += print_string ("/");

    if (f_tcb || f_both)
	// Print plain TCB address
	n += print_hex ((word_t) tcb, 0, sizeof (word_t) * 2);

    return n;
}
