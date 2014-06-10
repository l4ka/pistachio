/*	$OpenBSD: search.h,v 1.3 1997/09/21 10:45:49 niklas Exp $	*/
/*	$NetBSD: search.h,v 1.9 1995/08/08 21:14:45 jtc Exp $	*/

/*
 * Written by J.T. Conklin <jtc@netbsd.org>
 * Public domain.
 */

#ifndef _SEARCH_H_
#define _SEARCH_H_
#include <sys/cdefs.h>
#include <machine/ansi.h>

#ifdef	_BSD_SIZE_T_
typedef	_BSD_SIZE_T_	size_t;
#undef	_BSD_SIZE_T_
#endif

typedef struct entry {
	char *key;
	char *data;
} ENTRY;

typedef enum {
	FIND, ENTER
} ACTION;

typedef enum {
	preorder,
	postorder,
	endorder,
	leaf
} VISIT;

//http://gitweb.dragonflybsd.org/dragonfly.git/blob_plain/c8dd1ae615f4c3a6042727e69e5c05434f781224:/include/search.h
typedef	struct node {
	char         *key;
	struct node  *llink, *rlink;
} node_t;

struct que_elem {
	struct que_elem *next;
	struct que_elem *prev;
};

//__BEGIN_DECLS
#ifdef __cplusplus
extern "C" {
#endif
// void	*bsearch ((const void *, const void *, size_t, size_t,
//			      int (*)(const void *, const void *)));
 //int	 hcreate ((unsigned int));
 //void	 hdestroy ((void));
 //ENTRY	*hsearch ((ENTRY, ACTION));

// void	*lfind ((const void *, const void *, size_t *, size_t,
			    //  int (*)(const void *, const void *)));
 //void	*lsearch ((const void *, const void *, size_t *, size_t,
			     // int (*)(const void *, const void *)));
 void	 insque (void *, void *);
 //void	 remque ((void *));

 void	*tdelete (const void *, void **,
			      int (*)(const void *, const void *));
 void	*tfind (const void *, void * const *,
			      int (*)(const void *, const void *));
void *
tsearch(const void *vkey, void **vrootp,
    int (*compar)(const void *, const void *));

 void      twalk (const void *, void (*)(const void *, VISIT, int));
//__END_DECLS
#ifdef __cplusplus
}
#endif

#endif
