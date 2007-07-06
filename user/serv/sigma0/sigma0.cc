/*********************************************************************
 *                
 * Copyright (C) 2001-2006,  Karlsruhe University
 *                
 * File path:     sigma0.cc
 * Description:   sigma0 implementation
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
 * $Id: sigma0.cc,v 1.47 2006/10/21 03:52:55 reichelt Exp $
 *                
 ********************************************************************/
#include <l4/kip.h>
#include <l4/ipc.h>
#include <l4/misc.h>
#include <l4/kdebug.h>
#include <l4io.h>

#include "sigma0.h"
#include "region.h"

/**
 * Verbose level for sigma0 output.
 */
int verbose = 0;


/**
 * Pointer to sigma0's kernel interface page.
 */
L4_KernelInterfacePage_t * kip;


/*
 * Some API defined thread IDs.
 */
L4_ThreadId_t kernel_id;	// Lowest possible system thread
L4_ThreadId_t sigma0_id;
L4_ThreadId_t sigma1_id;
L4_ThreadId_t rootserver_id;



extern "C" __attribute__ ((weak)) void *
memcpy (void * dst, const void * src, unsigned int len)
{
    unsigned char *d = (unsigned char *) dst;
    unsigned char *s = (unsigned char *) src;

    while (len-- > 0)
	*d++ = *s++;

    return dst;
}


void dump_pools (void);


/*
 * Encoding for label field of message tag.
 */
#define L4_REQUEST_MASK		( ~((~0UL) >> ((sizeof (L4_Word_t) * 8) - 20)))
#define L4_PAGEFAULT		(-2UL << 20)
#define L4_SIGMA0_RPC		(-6UL << 20)
#define L4_SIGMA0_EXT		(-1001UL << 20)


/*
 * Encoding for MR1 of extended sigma0 protocol.
 */
#define L4_S0EXT_VERBOSE	(1)
#define L4_s0EXT_DUMPMEM	(2)

static region_t initial_regs[32];


/**
 * Main sigma0 loop.
 */
extern "C" void sigma0_main (void)
{
    L4_Word_t api_version, api_flags, kernelid;
    L4_MsgTag_t tag;

    dprintf (0, "s0: This is Sigma0\n");

    // Get kernel interface page.
    kip = (L4_KernelInterfacePage_t *)
	L4_KernelInterface (&api_version, &api_flags, &kernelid);

    dprintf (0, "s0: KIP @ %p (0x%lx, 0x%lx, 0x%lx)\n", 
	     kip, api_version, api_flags, kernelid);

    // Calculate API defined thread IDs.
    kernel_id = L4_GlobalId (L4_ThreadIdSystemBase (kip), 1);
    sigma0_id = L4_GlobalId (L4_ThreadIdUserBase (kip), 1);
    sigma1_id = L4_GlobalId (L4_ThreadIdUserBase (kip) + 1, 1);
    rootserver_id = L4_GlobalId (L4_ThreadIdUserBase (kip) + 2, 1);

    // Add some initial region_t structures to pool
    region_list.add ((L4_Word_t) initial_regs, sizeof (initial_regs));

    init_mempool ();
#if defined(L4_ARCH_IA32) || defined(L4_ARCH_AMD64)
    init_iopool();
#endif

    L4_ThreadId_t tid;
    L4_Accept (L4_UntypedWordsAcceptor);
    L4_Reset_WordSizeMask();

    tag = L4_Wait (&tid);

    for (;;)
    {
	L4_Msg_t msg;
	L4_MapItem_t map;
	L4_Fpage_t fpage;

	L4_Word_t word_size_mask = L4_WordSizeMask();
	L4_Reset_WordSizeMask();

	// Make sure we've received a valid IPC request
	while (L4_IpcFailed (tag))
	    tag = L4_Wait (&tid);

	if (L4_UntypedWords (tag) != 2 || L4_TypedWords (tag) != 0)
	{
	    dprintf (0, "s0: malformed request from %p (tag=%p)\n", 
		     (void *) tid.raw, (void *) tag.raw);
	    tag = L4_Wait (&tid);
	    continue;
	}
	
	L4_Store (tag, &msg);

	dprintf (1, "s0: got msg from %p, (0x%lx, %p, %p)\n", 
		 (void *) tid.raw, (long) L4_Label (tag),
		 (void *) L4_Get (&msg, 0), (void *) L4_Get (&msg, 1));

	/*
	 * Dispatch IPC according to protocol.
	 */

	switch (tag.raw & L4_REQUEST_MASK)
	{
	case L4_PAGEFAULT:
	{
	    if (! allocate_page (tid, L4_Get (&msg, 0), min_pgsize, map, true))
		dprintf (0, "s0: unhandled pagefault from %p @ %p, ip: %p\n",
			 (void *) tid.raw, (void *) L4_Get (&msg, 0),
			 (void *) L4_Get (&msg, 1));
	    break;
	}
#if defined(L4_ARCH_IA32) || defined(L4_ARCH_AMD64)
	case L4_IO_PAGEFAULT:
	{
	    L4_Fpage_t iofp = { raw : L4_Get(&msg, 0) };
	    if (! allocate_iopage (tid, iofp, map))
		dprintf (0, "s0: unhandled IO-pagefault from %p @ "
			 "%lx[%lx], ip: %p\n",
			 (void *) tid.raw, L4_IoFpagePort(iofp),
			 L4_IoFpageSize(iofp),
			 (void *) L4_Get (&msg, 1));
	    break;
	}
#endif

	case L4_SIGMA0_RPC:
	{
	    fpage.raw = L4_Get (&msg, 0);
#if defined(L4_ARCH_IA32) || defined(L4_ARCH_AMD64)
	    if (L4_IsIoFpage (fpage))
	    {
		if ((fpage.raw >> 16) == (word_size_mask >> 16))
		{
		    dprintf (0, "s0: cannot allocate arbitrary IO fpage\n");
		}
		else
		{
		    // Allocate from specific location.
		    if (! allocate_iopage (tid, fpage, map))
			dprintf (0, "s0: unable to allocate IO fpage at port "
				 "%x of size %p to %p\n",
				 (int) L4_IoFpagePort(fpage),
				 (void *) L4_IoFpageSize (fpage),
				 (void *) tid.raw);
		}
		break;
	    }
#endif
	    L4_Word_t addr = L4_Address (fpage);
	    L4_Word_t attributes = L4_Get (&msg, 1);

	    if (is_kernel_thread (tid))
	    {
		if (L4_Size (fpage) == 0)
		    // Request recommended kernel memory.
		    L4_KDB_Enter ("s0: recommended kernel mem");
		else
		    // Request kernel memory.
		    L4_KDB_Enter ("s0: request kernel memory");

		tag = L4_Wait (&tid);
		continue;
	    }
	    else
	    {
		if ((fpage.raw >> 10) == (word_size_mask >> 10))
		{
		    // Allocate from arbitrary location.
		    if (! allocate_page (tid, L4_SizeLog2 (fpage), map))
			dprintf (0, "s0: unable to allocate page of size %p"
				 " to %p\n", (void *) L4_Size (fpage),
				 (void *) tid.raw);
		}
		else
		{
		    // Allocate from specific location.
		    if (! allocate_page (tid, addr, L4_SizeLog2 (fpage),
			    	    	 map))
			dprintf (0, "s0: unable to allocate page %p of "
				 "size %p to %p\n", (void *) addr,
				 (void *) L4_Size (fpage),
				 (void *) tid.raw);
		}

		if (attributes != 0)
		{
		    // XXX: Setting memory attributes in sigma0
		    // possibly needs to be revised.

		    // Set memory attributes before mapping.
		    if (! L4_Set_PageAttribute (L4_SndFpage (map), attributes))
		    {
			dprintf (1, "s0: memory control failed (%ld) setting "
				 "page %p with attributes %p",
				 L4_ErrorCode(), 
				 (void *) L4_Address (L4_SndFpage (map)),
				 (void *) attributes);

			// We do not deallocate the memory.
			map = L4_MapItem (L4_Nilpage, 0);
		    }
		}
	    } 
	    break;
	}

	case L4_SIGMA0_EXT:
	{
	    // Only allow kernel threads to use extended sigma0 protocol.
	    if (! is_kernel_thread (tid))
	    {
		tag = L4_Wait (&tid);
		continue;
	    }

	    bool reply = false;
	    switch (L4_Get (&msg, 0))
	    {
	    case L4_S0EXT_VERBOSE:
		verbose = L4_Get (&msg, 1);
		break;
	    case L4_s0EXT_DUMPMEM:
		dump_pools ();
		if (L4_Get (&msg, 1) != 0)
		    reply = true;
		break;
	    }

	    if (! reply)
		tag = L4_Wait (&tid);
	    else
	    {
		L4_Set_MsgTag (L4_Niltag);
		tag = L4_ReplyWait (tid, &tid);
	    }
	    continue;
	}

	default:
	    dprintf (0, "s0: unknown sigma0 request from %p, (%p, %p, %p)\n",
		     (void *) tid.raw, (void *) tag.raw,
		     (void *) L4_Get (&msg, 0), (void *) L4_Get(&msg, 1));
	    map = L4_MapItem (L4_Nilpage, 0);

	    tag = L4_Wait (&tid);
	    continue;
	}

	L4_Put (&msg, 0, 0, (L4_Word_t *) 0, 2, &map);
	L4_Load (&msg);
	tag = L4_ReplyWait (tid, &tid);

	// If reply phase fails, receive phase is redone at top of loop.
    }
}


/**
 * Dump all sigma0 pools.
 */
void dump_pools (void)
{
    printf ("s0: Free region structures: %d\n:s0\n",
	    (int) region_list.contents ());
    dump_mempools ();
}
