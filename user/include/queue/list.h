/*
 * Australian Public Licence B (OZPLB)
 * 
 * Version 1-0
 * 
 * Copyright (c) 2004 National ICT Australia
 * 
 * All rights reserved. 
 * 
 * Developed by: Embedded, Real-time and Operating Systems Program (ERTOS)
 *               National ICT Australia
 *               http://www.ertos.nicta.com.au
 * 
 * Permission is granted by National ICT Australia, free of charge, to
 * any person obtaining a copy of this software and any associated
 * documentation files (the "Software") to deal with the Software without
 * restriction, including (without limitation) the rights to use, copy,
 * modify, adapt, merge, publish, distribute, communicate to the public,
 * sublicense, and/or sell, lend or rent out copies of the Software, and
 * to permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimers.
 * 
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimers in the documentation and/or other materials provided
 *       with the distribution.
 * 
 *     * Neither the name of National ICT Australia, nor the names of its
 *       contributors, may be used to endorse or promote products derived
 *       from this Software without specific prior written permission.
 * 
 * EXCEPT AS EXPRESSLY STATED IN THIS LICENCE AND TO THE FULL EXTENT
 * PERMITTED BY APPLICABLE LAW, THE SOFTWARE IS PROVIDED "AS-IS", AND
 * NATIONAL ICT AUSTRALIA AND ITS CONTRIBUTORS MAKE NO REPRESENTATIONS,
 * WARRANTIES OR CONDITIONS OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO ANY REPRESENTATIONS, WARRANTIES OR CONDITIONS
 * REGARDING THE CONTENTS OR ACCURACY OF THE SOFTWARE, OR OF TITLE,
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NONINFRINGEMENT,
 * THE ABSENCE OF LATENT OR OTHER DEFECTS, OR THE PRESENCE OR ABSENCE OF
 * ERRORS, WHETHER OR NOT DISCOVERABLE.
 * 
 * TO THE FULL EXTENT PERMITTED BY APPLICABLE LAW, IN NO EVENT SHALL
 * NATIONAL ICT AUSTRALIA OR ITS CONTRIBUTORS BE LIABLE ON ANY LEGAL
 * THEORY (INCLUDING, WITHOUT LIMITATION, IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHERWISE) FOR ANY CLAIM, LOSS, DAMAGES OR OTHER
 * LIABILITY, INCLUDING (WITHOUT LIMITATION) LOSS OF PRODUCTION OR
 * OPERATION TIME, LOSS, DAMAGE OR CORRUPTION OF DATA OR RECORDS; OR LOSS
 * OF ANTICIPATED SAVINGS, OPPORTUNITY, REVENUE, PROFIT OR GOODWILL, OR
 * OTHER ECONOMIC LOSS; OR ANY SPECIAL, INCIDENTAL, INDIRECT,
 * CONSEQUENTIAL, PUNITIVE OR EXEMPLARY DAMAGES, ARISING OUT OF OR IN
 * CONNECTION WITH THIS LICENCE, THE SOFTWARE OR THE USE OF OR OTHER
 * DEALINGS WITH THE SOFTWARE, EVEN IF NATIONAL ICT AUSTRALIA OR ITS
 * CONTRIBUTORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH CLAIM, LOSS,
 * DAMAGES OR OTHER LIABILITY.
 * 
 * If applicable legislation implies representations, warranties, or
 * conditions, or imposes obligations or liability on National ICT
 * Australia or one of its contributors in respect of the Software that
 * cannot be wholly or partly excluded, restricted or modified, the
 * liability of National ICT Australia or the contributor is limited, to
 * the full extent permitted by the applicable legislation, at its
 * option, to:
 * a.  in the case of goods, any one or more of the following:
 * i.  the replacement of the goods or the supply of equivalent goods;
 * ii.  the repair of the goods;
 * iii. the payment of the cost of replacing the goods or of acquiring
 *  equivalent goods;
 * iv.  the payment of the cost of having the goods repaired; or
 * b.  in the case of services:
 * i.  the supplying of the services again; or
 * ii.  the payment of the cost of having the services supplied again.
 * 
 * The construction, validity and performance of this licence is governed
 * by the laws in force in New South Wales, Australia.
 */
/*
 * Author: Alex Webster
 */
#ifndef LIST_H
#define LIST_H

#include <stddef.h>

#define LIST_EMPTY(head) \
    ((head)->lh_first == NULL)

#define LIST_ENTRY(type) \
    struct { \
	    struct type *le_next; \
	    struct type *le_prev; \
    }

#define LIST_FIRST(head) \
    ((head)->lh_first)

#define LIST_FOREACH(var, head, name) \
    for ((var) = (head)->lh_first; (var); (var) = (var)->name.le_next)

#define LIST_FOREACH_SAFE(var, head, name, tmp) \
    for ((var) = (head)->lh_first; (var) && ((tmp) = (var)->name.le_next), (var); (var) = (tmp))

#define LIST_HEAD(headname, type) \
    struct headname { \
	    struct type *lh_first; \
    }

#define LIST_HEAD_INITIALIZER(head) \
    { .lh_first = NULL }

#define LIST_INIT(head) \
    do { \
	    (head)->lh_first = NULL; \
    } while (0)

#define LIST_INSERT_AFTER(head, listelm, elm, name) \
    do { \
	    (elm)->name.le_next = (listelm)->name.le_next; \
	    (elm)->name.le_prev = (listelm); \
	    (listelm)->name.le_next = (elm); \
	    if ((elm)->name.le_next != NULL) { \
		    (elm)->name.le_next->name.le_prev = (elm); \
	    } \
    } while (0)

#define LIST_INSERT_BEFORE(head, listelm, elm, name) \
    do { \
	(elm)->name.le_next = (listelm); \
	(elm)->name.le_prev = (listelm)->name.le_prev; \
	(listelm)->name.le_prev = (elm); \
	if ((elm)->name.le_prev != NULL) { \
		(elm)->name.le_prev->name.le_next = (elm); \
	} \
    } while (0)

#define LIST_INSERT_HEAD(head, elm, name) \
    do { \
	    (elm)->name.le_next = (head)->lh_first; \
	    (elm)->name.le_prev = NULL; \
	    if ((head)->lh_first != NULL) { \
		    (head)->lh_first->name.le_prev = (elm); \
	    } \
	    (head)->lh_first = (elm); \
    } while (0)

#define LIST_NEXT(elm, name) \
    ((elm)->name.le_next)

#define LIST_REMOVE(head, elm, name) \
    do { \
	    if ((elm)->name.le_next != NULL) { \
		    (elm)->name.le_next->name.le_prev = (elm)->name.le_prev; \
	    } \
	    if ((elm)->name.le_prev != NULL) { \
		    (elm)->name.le_prev->name.le_next = (elm)->name.le_next; \
	    } else { \
		    (head)->lh_first = (elm)->name.le_next; \
	    } \
    } while (0)

#endif
