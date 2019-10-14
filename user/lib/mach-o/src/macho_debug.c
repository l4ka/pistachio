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
 * macho_debug.c
 *
 * Geoffrey Lee < glee at cse unsw edu au >
 */

#include <mach-o/mach-o.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int
macho_printheaders(mh)
	struct	mach_header	*mh;
{
	cpu_type_t	cputype;
	cpu_subtype_t	cpusubtype;
	unsigned long	filetype, ncmds, sizeofcmds, flags;

	macho_getmachhdr(mh, &cputype, &cpusubtype, &filetype, &ncmds,
	    &sizeofcmds, &flags);

	printf("%s: cpu type %d\n", __func__, cputype);
	printf("%s: cpu subtype %d\n", __func__, cpusubtype);
	printf("%s: filetype %lu\n", __func__, filetype);
	printf("%s: ncmds %lu\n", __func__, ncmds);
	printf("%s: sizeofcmds %lu\n", __func__, sizeofcmds);
	printf("%s: flags %lu\n", __func__, flags);

	return EMACHOSUCCESS;
}

int
macho_printloadcmds(mh)
	struct mach_header	*mh;
{
	struct load_command	*lc;
	int			i;

	lc = macho_getloadcmds(mh);

	for (i = 0; i < mh->ncmds; i++) {

		printf("%s: load command %d type %lu size %lu\n",
		    __func__,
		    i,
		    lc->cmd,
		    lc->cmdsize);

		lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}

	return EMACHOSUCCESS;
}


