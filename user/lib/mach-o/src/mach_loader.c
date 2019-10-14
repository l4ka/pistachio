/*
 * @NICTA_MODIFICATIONS_START@
 * 
 * This source code is licensed under Apple Public Source License Version 2.0.
 * Portions copyright Apple Computer, Inc.
 * Portions copyright National ICT Australia.
 *
 * All rights reserved.
 *
 * This code was modified 2006-06-20.
 *
 * @NICTA_MODIFICATIONS_END@
 */
/*
 * Copyright (c) 2001-2005 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 2.0 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 *
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 * mach_loader.c
 *
 * A loader for Mach-O files
 *
 * XXX 64-bit?  This can only load 32-bit stuff for now.  It should not be
 * XXX hard to do, but it just isn't required for now.
 *
 * XXX this only does the right thing if the file arch and  the host arch
 * XXX are the same, because it does not do any byteswapping.
 *
 * -gl 
 *
 * Geoffrey Lee < glee at cse.unsw.edu.au >
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <l4/kdebug.h>

/*
 * This our directory!
 */
#include <mach-o/mach-o.h>

int
macho_checkfile(
	void	*f
)
{
	struct mach_header	*mh;

	mh = (struct mach_header *)f;

	if (mh->magic != MH_MAGIC && mh->magic != MH_CIGAM)
		return EMACHOBADFILE;

	return EMACHOSUCCESS;
}

int
macho_getmachhdr(
	struct mach_header	*mh,
	cpu_type_t		*cputype,
	cpu_subtype_t		*cpusubtype,
	unsigned long		*filetype,
	unsigned long		*ncmds,
	unsigned long		*sizeofcmds,
	unsigned long		*flags
)
{

	if (cputype)
		*cputype = mh->cputype;
	if (cpusubtype)
		*cpusubtype = mh->cpusubtype;
	if (filetype)
		*filetype = mh->filetype;
	if (ncmds)
		*ncmds = mh->ncmds;
	if (sizeofcmds)
		*sizeofcmds = mh->sizeofcmds;
	if (flags)
		*flags = mh->flags;

	return EMACHOSUCCESS;
}

struct load_command *
macho_getloadcmds(
	struct mach_header	*mh
)
{
	struct load_command	*cmd;

	cmd = (struct load_command *)(mh + 1);

	return (cmd);
}

