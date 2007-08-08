/*********************************************************************
 *                
 * Copyright (C) 2002-2007,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/ctors.cc
 * Description:   Prioritized constructors support
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
 * $Id$
 *                
 ********************************************************************/

#include <debug.h>	/* TRACEF */


// helper type
typedef void (*func_ptr) (void);

// zero-terminated constructor tables for each group
extern func_ptr __ctors_CPU__[];
extern func_ptr __ctors_NODE__[];
extern func_ptr __ctors_GLOBAL__[];

/// calls all constructors for CPU-local global objects
void call_cpu_ctors()
{
    for (unsigned int i = 0; __ctors_CPU__[i] != 0; i++)
	__ctors_CPU__[i] ();
}

/// calls all constructors for node-local global objects
void call_node_ctors()
{
    for (unsigned int i = 0; __ctors_NODE__[i] != 0; i++)
	__ctors_NODE__[i] ();
}

/// calls truly global constructors
void call_global_ctors()
{
    for (unsigned int i = 0; __ctors_GLOBAL__[i] != 0; i++)
	__ctors_GLOBAL__[i] ();
}

