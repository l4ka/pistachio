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
/* Simple hash based on Mungi hash functions. Maps ints to void pointers.*/
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#include "hash.h"

struct hashtable *
hash_init (unsigned int size)
{
	struct hashtable *tablestruct;
	int counter;
		
	/* Our hash function only works with power-of-2 bucket sizes for speed. */
	assert ((size & (size -1)) == 0);
	
	tablestruct = malloc (sizeof (struct hashtable)); assert (tablestruct);
	if (!tablestruct) {
		return NULL;
	}
	tablestruct->table = malloc (size * sizeof (struct hashentry *));
	if (!tablestruct->table) {
		return NULL;
	}
	for (counter=0; counter<size; counter++) {
		tablestruct->table[counter] = NULL;
	}
	assert (tablestruct->table);
	tablestruct->size = size;
	tablestruct->spares = NULL;
	
	return tablestruct;
}

/* Ref http://www.concentric.net/~Ttwang/tech/inthash.htm */
uintptr_t
hash_hash(uintptr_t key)
{
#if (UINTPTR_MAX == UINT32_MAX)
	key += ~(key << 15);
	key ^=  (key >> 10);
	key +=  (key << 3);
	key ^=  (key >> 6);
	key += ~(key << 11);
	key ^=  (key >> 16);
#elif (UINTPTR_MAX == UINT64_MAX)
	key += ~(key << 32);
	key ^= (key >> 22);
	key += ~(key << 13);
	key ^= (key >> 8);
	key += (key << 3);
	key ^= (key >> 15);
	key += ~(key << 27);
	key ^= (key >> 31);
#else
#error unsupported word size
#endif
	//printf ("new key is %d\n", key);
	return key;
}

void *
hash_lookup (struct hashtable *tablestruct, uintptr_t key)
{
	uintptr_t hash;
	struct hashentry *entry;
	
	hash = hash_hash (key) & (tablestruct->size - 1);
	for (entry = tablestruct->table[hash]; entry != NULL; entry = entry->next) {
		if (entry->key == key) {
			return entry->value;
		}
	}
	return NULL;
}

/* Add the key to the hash table. Assumes the key is not already present. */
int
hash_insert (struct hashtable *tablestruct, uintptr_t key, void *value)
{
	uintptr_t hash;
	struct hashentry *entry;
	
	hash = hash_hash (key) & (tablestruct->size - 1);
	//printf ("bucket is %d\n", hash);

	entry = malloc (sizeof (struct hashentry));
	if (!entry) {
		return -1;
	}
	entry->key = key;
	entry->value = value;
	entry->next = tablestruct->table[hash];
	
	tablestruct->table[hash] = entry;
	return 0;
}

/* Removes the key from the hash table. Does not signal an error if the key
 * was not present. */
void
hash_remove (struct hashtable *tablestruct, uintptr_t key)
{
	uintptr_t hash;
	struct hashentry *entry, *tmpentry;
	
	hash = hash_hash (key) & (tablestruct->size - 1);
	entry = tablestruct->table[hash];
	/* If this is the first entry then it needs special handling. */
	if (entry && entry->key == key) {
		tmpentry = entry->next;
		free (entry);
		tablestruct->table[hash] = tmpentry;
	} else {
		while (entry) {
			if (entry->next && entry->next->key == key) {
				tmpentry = entry->next;
				entry->next = entry->next->next;
				free (tmpentry);
				break;
			}
			entry = entry->next;
		}
	}
}

