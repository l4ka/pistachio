/*********************************************************************
 *                
 * Copyright (C) 2002, 2004-2005, 2007-2008,  Karlsruhe University
 *                
 * File path:     api/v4/ipcx.cc
 * Description:   Extended transfer of IPC
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
 * $Id: ipcx.cc,v 1.26 2006/10/19 22:57:38 ud3 Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include <kdb/tracepoints.h>
#include INC_API(tcb.h)
#include INC_API(ipc.h)
#include INC_API(fpage.h)
#include INC_API(schedule.h)
#include INC_GLUE(map.h)

#define CHECK_BR_IDX(idx) if (idx > IPC_NUM_BR) goto message_overflow
#define CHECK_MR_IDX(idx, total) if (idx > total) goto message_overflow


DECLARE_TRACEPOINT_DETAIL(IPC_STRING_COPY);
DECLARE_TRACEPOINT_DETAIL(IPC_STRING_ITEM);
DECLARE_TRACEPOINT_DETAIL(IPC_MAPGRANT_ITEM);
DECLARE_TRACEPOINT_DETAIL(IPC_MESSAGE_OVERFLOW);
DECLARE_TRACEPOINT_DETAIL(IPC_EXT_TRANSFER);

#if !defined(IPC_STRING_COPY)
extern "C" void * memcpy (void * dst, const void * src, word_t len);
#define IPC_STRING_COPY memcpy
#endif

word_t ipc_copy (tcb_t * src, addr_t src_addr,
		 tcb_t * dst, addr_t dst_addr, word_t len)
{
    TRACEPOINT (IPC_STRING_COPY, "IPC string copy: %t @ %p -> %t @ %p, len=0x%x\n",
		src, src_addr, dst, dst_addr, len);

    space_t * src_space = src->get_space ();
    space_t * dst_space = dst->get_space ();

    // Check limits of source and destination string.
    word_t src_len = src_space->get_copy_limit (src_addr, len);
    word_t dst_len = dst_space->get_copy_limit (dst_addr, len);

    // String might need to be clipped.
    if (src_len < len) len = src_len;
    if (dst_len < len) len = dst_len;

    // For inter-space string copy we need to use a copy area.
    if (src_space != dst_space)
	src->adjust_for_copy_area (dst, &src_addr, &dst_addr);

    src->misc.ipc_copy.copy_start_src =
	src->misc.ipc_copy.copy_fault = src_addr;
    src->misc.ipc_copy.copy_start_dst = dst_addr;

    // we may cause a nested pagefault ipc, therefore unlock the receiver tcb
    dst->unlock();
    IPC_STRING_COPY (dst_addr, src_addr, len);
    dst->lock();
    
    src->misc.ipc_copy.copy_length += len;

    return len;
}

static void copy_mr(tcb_t * dst, tcb_t * src, int index)
{
    ASSERT(index < IPC_NUM_MR);
    dst->set_mr(index, src->get_mr(index));
}

msg_tag_t extended_transfer(tcb_t * src, tcb_t * dst, msg_tag_t msgtag)
{
    msg_item_t src_item;
    acceptor_t acceptor;
    int br_idx = 1;
    word_t total_mrs = msgtag.get_untyped() + msgtag.get_typed();
    word_t total_len = 0;
    bool accept_strings;

//    ENABLE_TRACEPOINT (IPC_STRING_COPY, ~0, 0);
//    ENABLE_TRACEPOINT (IPC_STRING_ITEM, ~0, 0);
//    ENABLE_TRACEPOINT (IPC_MESSAGE_OVERFLOW, ~0, 0);
#undef TRACEF
#define TRACEF(args...)
    
    if (total_mrs > IPC_NUM_MR)
    {
	printf("message exceeds MR's (untyped=%d, typed=%d)\n", 
	       msgtag.get_untyped(), msgtag.get_typed());
	enter_kdebug("message exceeds MR's");
	goto message_overflow;
    }

    src->set_state (thread_state_t::locked_running);
    dst->set_state (thread_state_t::locked_waiting);

    src->set_partner (dst->get_global_id ());
    src->misc.ipc_copy.copy_length = 0;

    acceptor = dst->get_br(0);
    
    /* does the receiver (still) accepts strings? */
    accept_strings = acceptor.accept_strings();

    TRACEPOINT(IPC_EXT_TRANSFER, "tag=%p, untyped: %d, typed: %d, acceptor: %x\n", 
	       msgtag.raw, msgtag.get_untyped(), msgtag.get_typed(), acceptor);

    for (word_t src_idx = msgtag.get_untyped() + 1; src_idx < total_mrs; )
    {
	src_item = src->get_mr(src_idx);

	if (src_item.is_map_item() || src_item.is_grant_item())
	{
	    /* is the descriptor beyond the valid range? */
	    CHECK_MR_IDX(src_idx + 1, total_mrs);
	    
	    fpage_t snd_fpage, rcv_fpage;
	    snd_fpage.raw = src->get_mr(src_idx + 1);

	    if (snd_fpage.is_mempage ())
		rcv_fpage.raw = acceptor.get_rcv_window();
	    else if (snd_fpage.is_archpage ())
		rcv_fpage = acceptor.get_arch_specific_rcvwindow (dst);
	    else
		// Unknown fpage type
		goto message_overflow;


	    TRACEPOINT(IPC_MAPGRANT_ITEM, "%s item: snd_base=%p, fpage=%p\n",
		       src_item.is_map_item() ? "map" : "grant", 
		       src_item.get_snd_base(), snd_fpage.raw);
	    
	    /* does the receiver accept mappings */
	    if (EXPECT_FALSE( rcv_fpage.is_nil_fpage() ))
		goto message_overflow;

	    copy_mr(dst, src, src_idx++);
	    copy_mr(dst, src, src_idx++);

	    if (snd_fpage.is_mempage ())
		src->get_space()->map_fpage
		    (snd_fpage, src_item.get_snd_base(), 
		     dst->get_space(), rcv_fpage, src_item.is_grant_item());
	    else if (snd_fpage.is_archpage ())
		arch_map_fpage(src, snd_fpage, src_item.get_snd_base (),
			       dst, rcv_fpage, src_item.is_grant_item ());

	    if (EXPECT_FALSE (src_item.is_grant_item () &&
			      snd_fpage.get_size_log2 () >
			      rcv_fpage.get_size_log2 ()))
	    {
		/*
		 * We are granting an fpage that is larger than
		 * receive window.  Make sure that the whole fpage of
		 * the sender is flushed.  We don't call unmap from
		 * the map function itself to avoid deep function
		 * recursion.
		 */
		if (snd_fpage.is_mempage ())
		    src->get_space ()->unmap_fpage (snd_fpage, true, false);
		else if (snd_fpage.is_archpage ())
		    arch_unmap_fpage (src, snd_fpage, true);
	    }
	}
	else if (src_item.is_string_item())
	{
	    /* 
	     * Copy the MR at the very beginning to make sure the
	     * receiver can deal with cut message situations.
	     */
	    copy_mr (dst, src, src_idx);
	
	    if (! accept_strings)
		goto message_overflow;

	    // We might have overflow on MRs
	    CHECK_MR_IDX (src_idx + src_item.get_string_ptr_count (),
			  total_mrs);

	    msg_item_t dst_item;
	    word_t src_addr, src_len, src_ptridx;
	    word_t dst_addr, dst_len, dst_ptridx;

	    src_addr = src->get_mr (src_idx + 1);
	    src_len  = src_item.get_string_length ();
	    src_ptridx = 1;

	    // Check for overflow on BRs.  br_idx is always guaranteed
	    // to be in range.
	    dst_item = dst->get_br (br_idx);
	    CHECK_BR_IDX (br_idx + dst_item.get_string_ptr_count ());

	    dst_addr = dst->get_br (br_idx + 1);
	    dst_len  = dst_item.get_string_length ();
	    dst_ptridx = 1;

	    TRACEPOINT (IPC_STRING_ITEM, 
			"IPC string item:  src_item=%p  dst_item=%p"
			"  src: substrings=%d (idx=%d)  len=%p %s"
			"  dst: substrings=%d (idx=%d)  len=%p %s",
			src_item.raw, dst_item.raw,
			src_item.get_string_ptr_count (),
			src_ptridx, src_len,
			src_item.is_string_compound () ?	
			"compound" : "",
			dst_item.get_string_ptr_count (),
			dst_ptridx, dst_len,
			dst_item.is_string_compound () ?
			"compound" : "");

	    // Sanity checking
	    if (! dst_item.is_string_item ())
		goto message_overflow;

	    // Check if there are more receive buffers after this one
	    accept_strings = dst_item.more_strings ();

	    bool end_of_send_string = false;

	    while (! end_of_send_string)
	    {
		TRACEPOINT (IPC_STRING_ITEM, 
			    "  src: addr=%p len=%p (idx=%d)"
			    "  dst: addr=%p len=%p (idx=%d)",
			    src_addr, src_len, src_ptridx,
			    dst_addr, dst_len, dst_ptridx);

		copy_mr (dst, src, src_idx + src_ptridx);

		word_t copy_length = dst_len < src_len ? dst_len : src_len;
		word_t cpy_len = ipc_copy (src, (addr_t) src_addr,
					   dst, (addr_t) dst_addr,
					   copy_length);

		total_len += cpy_len;

		// Copy operation might have been clipped.
		if (cpy_len < copy_length)
		    goto message_overflow;

		src_len  -= cpy_len;
		dst_len  -= cpy_len;
		src_addr += cpy_len;
		dst_addr += cpy_len;

		if (src_len == 0)
		{
		    // Current part of send string exhausted.  Check
		    // if there are more substrings or compund strings
		    // in the current send string.

		    if (src_ptridx < src_item.get_string_ptr_count ())
		    {
			// More substrings to send
			src_ptridx++;
			src_addr = src->get_mr (src_idx + src_ptridx);
			src_len  = src_item.get_string_length ();
		    }
		    else if (src_item.is_string_compound ())
		    {
			// Send string is compund
			src_idx += src_ptridx + 1;
			src_ptridx = 1;
			CHECK_MR_IDX (src_idx + 1, total_mrs);

			src_item = src->get_mr (src_idx);
			src_addr = src->get_mr (src_idx + src_ptridx);
			src_len  = src_item.get_string_length ();

			CHECK_MR_IDX (src_idx +
				      src_item.get_string_ptr_count (),
				      total_mrs);

			TRACEPOINT (IPC_STRING_ITEM,
				    "  src: substrings=%d (idx=%d) len=%p %s\n",
					    src_item.get_string_ptr_count (),
					    src_ptridx, src_len,
					    src_item.is_string_compound () ?
					    "compound" : "");
		    }
		    else
		    {
			// End of send string
			end_of_send_string = true;
			src_idx += src_ptridx + 1;
		    }
		}

		if (end_of_send_string)
		{
		    // No more data in the current send string.  Skip
		    // past the current receive buffer.

		    TRACEF ("Skip receive buffer\n");
		    bool compound;
		    do {
			compound = dst_item.is_string_compound ();
			TRACEF ("compund=%d  more=%d\n",
				dst_item.is_string_compound (),
				dst_item.more_strings ());

			// Calculate position of next string item
			br_idx += dst_item.get_string_ptr_count () + 1;

			if (br_idx >= IPC_NUM_BR)
			{
			    // BR register overflow
			    TRACEF ("Last receive buffer\n");
			    accept_strings = false;
			    break;
			}

			dst_item.raw = dst->get_br (br_idx);
		    } while (compound);
		}
		else
		{
		    // More data in send string.  Check if we have
		    // room in the receive buffers.

		    if (dst_len == 0)
		    {
			if (dst_ptridx < dst_item.get_string_ptr_count ())
			{
			    // More substrings in receive buffer
			    dst_ptridx++;
			    dst_addr = dst->get_br (br_idx + dst_ptridx);
			    dst_len  = dst_item.get_string_length ();
			}
			else if (dst_item.is_string_compound ())
			{
			    // Receive buffer is compund
			    br_idx += dst_ptridx + 1;
			    dst_ptridx = 1;
			    CHECK_BR_IDX (br_idx + 1);

			    dst_item = dst->get_br (br_idx);
			    dst_addr = dst->get_br (br_idx + dst_ptridx);
			    dst_len  = dst_item.get_string_length ();

			    CHECK_BR_IDX (br_idx +
					  dst_item.get_string_ptr_count ());

			    TRACEPOINT (
				IPC_STRING_ITEM,
				"  dst: substrings=%d (idx=%d)"
					"  len=%p %s\n",
					dst_item.get_string_ptr_count (),
					dst_ptridx, src_len,
					dst_item.is_string_compound () ?
					"compound" : "");
			}
			else
			{
			    // No more receive buffers
			    TRACEF ("No more receive buffers\n");
			    goto message_overflow;
			}
		    }
		}

	    } // while (! end_of_send_string)
	}
       	else 
	{
	    TRACEF("unknown item type\n");
	    goto message_overflow;
	}
    }

    // Release copy area
    src->release_copy_area ();

    // Cancel any pending XFER timeouts.
    get_current_scheduler ()->cancel_timeout (src);
    src->flags -= tcb_t::has_xfer_timeout;
    return msgtag;

message_overflow:

    // Release copy area
    src->release_copy_area ();

    // Cancel any pending XFER timeouts.
    get_current_scheduler ()->cancel_timeout (src);

    TRACEPOINT (IPC_MESSAGE_OVERFLOW, "IPC message overflow (%t->%t), len=0x%x\n",
		src, dst, total_len);

    // Report message overflow error
    dst->set_error_code (IPC_RCV_ERROR (ERR_IPC_MSG_OVERFLOW (total_len)));
    src->set_error_code (IPC_SND_ERROR (ERR_IPC_MSG_OVERFLOW (total_len)));
    msgtag.set_error();
    return msgtag;
}
