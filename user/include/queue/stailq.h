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
#ifndef STAILQ_H
#define STAILQ_H

#include <stddef.h>

#define STAILQ_CONCAT(head1, head2, name) \
    do { \
	    if ((head2)->stqh_first != NULL) { \
		(head1)->stqh_last->name.stqe_next = (head2)->stqh_first; \
		(head1)->stqh_last = (head2)->stqh_last; \
		(head2)->stqh_first = NULL; \
		(head2)->stqh_last = NULL; \
	} \
    } while (0)

#define STAILQ_EMPTY(head) \
    ((head)->stqh_first == NULL)

#define STAILQ_ENTRY(type) \
    struct { \
	    struct type *stqe_next; \
    }

#define STAILQ_FIRST(head) \
    ((head)->stqh_first)

#define STAILQ_FOREACH(var, head, name) \
    for ((var) = (head)->stqh_first; (var); (var) = (var)->name.stqe_next)

#define STAILQ_FOREACH_SAFE(var, head, name, tmp) \
    for ((var) = (head)->stqh_first; (var) && ((tmp) = (var)->name.stqe_next), (var); (var) = (tmp))

#define STAILQ_HEAD(headname, type) \
    struct headname { \
	    struct type *stqh_first; \
	    struct type *stqh_last; \
    }

#define STAILQ_HEAD_INITIALIZER(head) \
    { .stqh_first = NULL, .stqh_last = NULL }

#define STAILQ_INIT(head) \
    do { \
	    (head)->stqh_first = NULL; \
	    (head)->stqh_last = NULL; \
    } while (0)

#define STAILQ_INSERT_AFTER(head, listelm, elm, name) \
    do { \
	    (elm)->name.stqe_next = (listelm)->name.stqe_next; \
	    (listelm)->name.stqe_next = (elm); \
	    if ((elm)->name.stqe_next == NULL) { \
		    (head)->stqh_last = (elm); \
	    } \
    } while (0)


#define STAILQ_INSERT_HEAD(head, elm, name) \
    do { \
	    (elm)->name.stqe_next = (head)->stqh_first; \
	    if ((elm)->name.stqe_next == NULL) { \
		    (head)->stqh_last = (elm); \
	    } \
	    (head)->stqh_first = (elm); \
    } while (0)

#define STAILQ_INSERT_TAIL(head, elm, name) \
    do { \
	    (elm)->name.stqe_next = NULL; \
	    if ((head)->stqh_last != NULL) { \
		    (head)->stqh_last->name.stqe_next = (elm); \
	    } \
	    (head)->stqh_last = (elm); \
    } while (0)

#define STAILQ_LAST(head) \
    ((head)->stqh_last)

#define STAILQ_NEXT(elm, name) \
    ((elm)->name.stqe_next)

#define STAILQ_REMOVE(head, elm, name) \
    do { \
	    if ((elm)->name.stqe_next != NULL) { \
		    (elm)->name.stqe_next->name.stqe_prev = (elm)->name.stqe_prev; \
	    } else { \
		    (head)->stqh_last = (elm)->name.stqe_prev; \
	    } \
	    if ((elm)->name.stqe_prev != NULL) { \
		    (elm)->name.stqe_prev->name.stqe_next = (elm)->name.stqe_next; \
	    } else { \
		    (head)->stqh_first = (elm)->name.stqe_next; \
	    } \
    } while (0)

#define STAILQ_REMOVE_HEAD(head, name) \
    do { \
	    (head)->stqh_first = (head)->stq_first->name.stqe_next; \
	    if ((head)->stqh_first == NULL) { \
		    (head)->stqh_last = NULL; \
	    } \
    } while (0)

#endif
