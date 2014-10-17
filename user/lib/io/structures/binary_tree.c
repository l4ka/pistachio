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
  Author: Ben Leslie
  Created: Tue Nov  9 2004 
*/
#include <stdlib.h>
#include <string.h>
#include <binary_tree/binary_tree.h>

struct bin_tree *binary_tree_new(void)
{
	struct bin_tree *new_tree;
	new_tree = malloc(sizeof(struct bin_tree));
	if (new_tree == NULL) {
		return new_tree;
	}
	new_tree->root = NULL;
	return new_tree;
}

static struct bin_tree_node *
find_node(struct bin_tree_node *root, char *key, int *cmp_p, struct bin_tree_node **parent)
{
	int cmp = 0;
	struct bin_tree_node *second_last_root = root;
	struct bin_tree_node *last_root = root;

	while(root != NULL) {
		second_last_root = last_root;
		last_root = root;

		cmp = strcmp(key, root->key);

		if (cmp == 0) {
			break;
		} 
		if (cmp < 0) {
			root = root->left;
		} else {
			root = root->right;
		}
	}
	*cmp_p = cmp;
	if (parent) {
		*parent = second_last_root;
	}
	return last_root;
}

/* Return the largest in this tree.. just go right */
static struct bin_tree_node *
largest_node(struct bin_tree_node *root, struct bin_tree_node **parent)
{
	struct bin_tree_node *last_root = NULL;
	while(root->right != NULL) {
		last_root = root;
		root = root->right;
	}
	if (parent) {
		*parent = last_root;
	}
	return root;
}

int
binary_tree_insert(struct bin_tree *tree, char *key, void *data)
{
	struct bin_tree_node *closest;
	struct bin_tree_node *new_node;
	int cmp;

	new_node = malloc(sizeof(struct bin_tree_node));
	if (new_node == NULL) return -1;
	
	new_node->key = key;
	new_node->data = data;
	new_node->left = NULL;
	new_node->right = NULL;

	if (tree->root == NULL) {
		tree->root = new_node;
	} else {
		closest = find_node(tree->root, key, &cmp, NULL);
		if (cmp == 0) {
			free(new_node);
			return -2; /* Key clash */
		}
		if (cmp < 0) {
			closest->left = new_node;
		} else {
			closest->right = new_node;
		}
	}
	return 0;
}

void *
binary_tree_lookup(struct bin_tree *tree, char *key)
{
	int cmp;
	struct bin_tree_node *closest;

	if (tree->root == NULL)
		return 0;
	closest = find_node(tree->root, key, &cmp, NULL);
	if (cmp == 0) {
		return closest->data;
	}
	return 0;
}

int
binary_tree_remove(struct bin_tree *tree, char *key)
{
	int cmp;
	struct bin_tree_node *dead_node, *parent, *replacement_node;

	if (tree->root == NULL)
		return 0;
	dead_node = find_node(tree->root, key, &cmp, &parent);
	if (cmp == 0) {
		/* Need to delete  */
		if (dead_node->left != NULL && dead_node->right != NULL) {
			/* Two children... promotion becomes difficult! */
			/* We'll take the largest on the left, could also take the smallest 
			   on the right. Ideally we'd take whichever was the longest path,
			   but I don't want to calculate that.. We should really use something
			   better than a binary tree. */
			replacement_node = largest_node(dead_node->left, &parent);
			if (parent == NULL) { 
				/* This happens if the largest on the left, is the first
				   node of the left */
				parent = dead_node;
			}
			/* Inplace replace our dead-node with this new node */
			dead_node->key = replacement_node->key;
			dead_node->data = replacement_node->data;

			/* Now we actually delete the repalced node, which must have at
			   most 1 child, which is easy to delete */
			dead_node = replacement_node;
		}

		/* Ok, by here we only have at most one child */
		if (dead_node->left != NULL)  {
			replacement_node = dead_node->left;
		} else if (dead_node->right != NULL) {
			replacement_node = dead_node->right;
		} else {
			/* Leaf node... easy! */
			replacement_node = NULL;
		}
		/* Now slot in the replacement node, and kill the dead_node */
		if (tree->root == dead_node) {
			tree->root = replacement_node;
		} else {
			if (parent->left == dead_node)
				parent->left = replacement_node;
			else
				parent->right = replacement_node;
		}
		free(dead_node);
		return 0;
	} else {
		return -1;
	}
}
