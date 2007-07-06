/*********************************************************************
 *                
 * Copyright (C) 2002,  Karlsruhe University
 *                
 * File path:     ctors.h
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
 * $Id: ctors.h,v 1.3 2004/09/17 13:01:46 ud3 Exp $
 *                
 ********************************************************************/
#ifndef __CTORS_H__
#define __CTORS_H__

/**
 * adds initialization priority to a static object
 * @param prio_class	major initialization group
 * @param prio		group-local priority, higher numbers
 *			indicate later initialization
 *
 * forces error message if @c prio is in next group
 *
 * example:	foo_t foo CTORPRIO(CTORPRIO_CPU, 100);
 * 
 */
#define CTORPRIO(prio_class, prio) \
  __attribute__((init_priority(prio >= 10000 ? 0 : 65535-(prio_class+prio))))


// Major priority groups for static constructors
#define CTORPRIO_CPU	30000
#define CTORPRIO_NODE	20000
#define CTORPRIO_GLOBAL	10000


// prototypes
void call_cpu_ctors();
void call_node_ctors();
void call_global_ctors();

#endif /* !__CTORS_H__ */
