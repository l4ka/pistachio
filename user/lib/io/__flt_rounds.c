/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */

/*
 * Copyright 2008 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

//#pragma ident	"%Z%%M%	%I%	%E% SMI"

/*
 * These functions return the prevailing rounding mode per ANSI C spec:
 *	 0:	toward zero
 *	 1:	to nearest			<<< default
 *	 2:	toward positive infinity
 *	 3:	toward negative infinity
 *	-1:	indeterminable			<<< never returned
 */

//#include "lint.h"
#include <sys/types.h>
//#include <floatingpoint.h>
//#include "libc.h"

//https://www.cs.cmu.edu/afs/cs.cmu.edu/project/amulet-4/jh6p/classes/studio/gcc-2.5.8/include/sys/ieeefp.h
#ifdef __i386__
enum fp_direction_type 		/* rounding direction */
	{
	fp_nearest	= 0,
	fp_negative	= 1,
	fp_positive	= 2,
	fp_tozero	= 3
	} ;
#endif


#if defined(__sparc)

int
__flt_rounds(void)
{
	switch (_QgetRD()) {
	case fp_tozero:
		return (0);

	case fp_positive:
		return (2);

	case fp_negative:
		return (3);
	}
	return (1);
}


//ftp://ftp.stu.edu.tw/BSD/OpenBSD/src/lib/libc/arch/arm/gen/flt_rounds.c
#elif defined(__i386) || defined(__amd64)

int
__fltrounds(void)
{
	switch (__xgetRD()) {
	case fp_tozero:
		return (0);

	case fp_positive:
		return (2);

	case fp_negative:
		return (3);
	}
	return (1);
}

#else
#error Unknown architecture
#endif
