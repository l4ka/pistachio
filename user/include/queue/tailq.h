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
#ifndef TAILQ_H
#define TAILQ_H

#include <stddef.h>

#define TAILQ_CONCAT(head1, head2, name) \
    do { \
	    if ((head2)->tqh_first != NULL) { \
		(head1)->tqh_last->name.tqe_next = (head2)->tqh_first; \
		(head2)->tqh_first->name.tqe_prev = (head1)->tqh_last; \
		(head1)->tqh_last = (head2)->tqh_last; \
		(head2)->tqh_first = NULL; \
		(head2)->tqh_last = NULL; \
	} \
    } while (0)

#define TAILQ_EMPTY(head) \
    ((head)->tqh_first == NULL)

#define TAILQ_ENTRY(type) \
    struct { \
	    struct type *tqe_next; \
	    struct type *tqe_prev; \
    }

#define TAILQ_FIRST(head) \
    ((head)->tqh_first)

#define TAILQ_FOREACH(var, head, name) \
    for ((var) = (head)->tqh_first; (var); (var) = (var)->name.tqe_next)

#define TAILQ_FOREACH_SAFE(var, head, name, tmp) \
    for ((var) = (head)->tqh_first; (var) && ((tmp) = (var)->name.tqe_next), (var); (var) = (tmp))

#define TAILQ_FOREACH_REVERSE(var, head) \
    for ((var) = (head)->tqh_last; (var); (var) = (var)->name.tqe_prev)

#define TAILQ_FOREACH_REVERSE_SAFE(var, head, name, tmp) \
    for ((var) = (head)->tqh_last; (var) && ((tmp) = (var)->name.tqe_prev), (var); (var) = (tmp))

#define TAILQ_HEAD(headname, type) \
    struct headname { \
	    struct type *tqh_first; \
	    struct type *tqh_last; \
    }

#define TAILQ_HEAD_INITIALIZER(head) \
    { .tqh_first = NULL, .tqh_last = NULL }

#define TAILQ_INIT(head) \
    do { \
	    (head)->tqh_first = NULL; \
	    (head)->tqh_last = NULL; \
    } while (0)

#define TAILQ_INSERT_AFTER(head, listelm, elm, name) \
    do { \
	    (elm)->name.tqe_next = (listelm)->name.tqe_next; \
	    (elm)->name.tqe_prev = (listelm); \
	    (listelm)->name.tqe_next = (elm); \
	    if ((elm)->name.tqe_next != NULL) { \
		    (elm)->name.tqe_next->name.tqe_prev = (elm); \
	    } else { \
		    (head)->tqh_last = (elm); \
	    } \
    } while (0)

#define TAILQ_INSERT_BEFORE(head, listelm, elm, name) \
    do { \
	(elm)->name.tqe_next = (listelm); \
	(elm)->name.tqe_prev = (listelm)->name.tqe_prev; \
	(listelm)->name.tqe_prev = (elm); \
	if ((elm)->name.tqe_prev != NULL) { \
		(elm)->name.tqe_prev->name.tqe_next = (elm); \
	} else { \
		(head)->tqh_first = (elm); \
	} \
    } while (0)

#define TAILQ_INSERT_HEAD(head, elm, name) \
    do { \
	    if (__builtin_types_compatible_p(typeof((head)->tqh_first),typeof(elm))) { \
		    (head) = __builtin_choose_expr(0, NULL, "I'm a sad panda"); \
	    } \
	    (elm)->name.tqe_next = (head)->tqh_first; \
	    (elm)->name.tqe_prev = NULL; \
	    if ((head)->tqh_first != NULL) { \
		    (head)->tqh_first->name.tqe_prev = (elm); \
	    } \
	    if ((elm)->name.tqe_next == NULL) { \
		    (head)->tqh_last = (elm); \
	    } \
	    (head)->tqh_first = (elm); \
    } while (0)

#define TAILQ_INSERT_TAIL(head, elm, name) \
    do { \
	    (elm)->name.tqe_next = NULL; \
	    (elm)->name.tqe_prev = (head)->tqh_last; \
	    if ((head)->tqh_last != NULL) { \
		    (head)->tqh_last->name.tqe_next = (elm); \
	    } \
	    if ((elm)->name.tqe_prev == NULL) { \
		    (head)->tqh_first = (elm); \
	    } \
	    (head)->tqh_last = (elm); \
    } while (0)

#define TAILQ_LAST(head) \
    ((head)->tqh_last)

#define TAILQ_NEXT(elm, name) \
    ((elm)->name.tqe_next)

#define TAILQ_PREV(elm, name) \
    ((elm)->name.tqe_prev)

#define TAILQ_REMOVE(head, elm, name) \
    do { \
	    if ((elm)->name.tqe_next != NULL) { \
		    (elm)->name.tqe_next->name.tqe_prev = (elm)->name.tqe_prev; \
	    } else { \
		    (head)->tqh_last = (elm)->name.tqe_prev; \
	    } \
	    if ((elm)->name.tqe_prev != NULL) { \
		    (elm)->name.tqe_prev->name.tqe_next = (elm)->name.tqe_next; \
	    } else { \
		    (head)->tqh_first = (elm)->name.tqe_next; \
	    } \
    } while (0)

#endif
