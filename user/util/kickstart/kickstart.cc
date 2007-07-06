/*********************************************************************
 *                
 * Copyright (C) 2003-2004,  Karlsruhe University
 *                
 * File path:     kickstart.cc
 * Description:   
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
 * $Id: kickstart.cc,v 1.15 2004/04/14 18:42:47 skoglund Exp $
 *                
 ********************************************************************/
/* startup stuff */

#include <config.h>
#include <l4io.h>

#include "kickstart.h"

#define	_MKSTR(sym)	_MKSTR2(sym)
#define	_MKSTR2(sym)	#sym


/**
 * Main kickstart loader function.  Parses through all loader formats
 * to find a valid one.
 */
extern "C" void loader (void)
{
    loader_format_t * fmt = NULL;

    printf("KickStart 0."_MKSTR(REVISION)"\n");

    // Try to find a valid loader format.
    for (L4_Word_t n = 0; loader_formats[n].probe; n++)
    {
	if (loader_formats[n].probe ())
	{
	    fmt = &loader_formats[n];
	    break;
	}
    }

    if (fmt == NULL)
    {
	printf ("No valid loader format found.");
	return;
    }

    printf ("Detected %s\n", fmt->name);
    L4_Word_t entry = fmt->init ();

    // Flush caches (some archs don't like code in their D-cache)
    flush_cache();

    printf("Launching kernel ...\n");

    // Start the kernel at its entry point
    launch_kernel (entry);
    
    // We're not supposed to return from the kernel. Signal if we do
    FAIL();
}
