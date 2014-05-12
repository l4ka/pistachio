#include <liballoc.h>
#include <l4/space.h>

//http://www.cse.unsw.edu.au/~cs9242/05/project/l4uman-x2.pdf
//https://lists.ira.uni-karlsruhe.de/pipermail/l4ka/2008-April/002072.html

/** This function is supposed to lock the memory data structures. It
 * could be as simple as disabling interrupts or acquiring a spinlock.
 * It's up to you to decide. 
 *
 * \return 0 if the lock was acquired successfully. Anything else is
 * failure.
 */
extern int liballoc_lock() {
	return 0;
}

/** This function unlocks what was previously locked by the liballoc_lock
 * function.  If it disabled interrupts, it enables interrupts. If it
 * had acquiried a spinlock, it releases the spinlock. etc.
 *
 * \return 0 if the lock was successfully released.
 */
extern int liballoc_unlock() {
return 0;
}


void *
la_get_pages( L4_Word_t count, int touch )
{ 


	void *ret = free_page;

	free_page += count * PAGE_SIZE; 
	
	/* should we fault the pages in? */
	if( touch != 0 )
	{
		char *addr = (char*) ret;
		L4_Word_t i;

		/* touch each page */
		for( i = 0; i < count; i++ )
		{
			la_safe_mem_touch( (void*) addr );
			for (int j=0; j<PAGE_SIZE; j++)
			    addr[j] = 0;
				
			addr += PAGE_SIZE;
			
		}
	}

	return (void*) ret;
}

L4_Word_t
la_safe_mem_touch( void *addr )
{
	volatile L4_Word_t *ptr;
	L4_Word_t copy;

	ptr = (L4_Word_t*) addr;
	copy = *ptr;
	*ptr = copy;

	return copy;
}

/** This is the hook into the local system which allocates pages. It
 * accepts an integer parameter which is the number of pages
 * required.  The page size was set up in the liballoc_init function.
 *
 * \return NULL if the pages were not allocated.
 * \return A pointer to the allocated memory.
 */
extern void* liballoc_alloc(int aPagesReq) {

return la_get_pages(aPagesReq, 0);

//return NULL;
}

/** This frees previously allocated memory. The void* parameter passed
 * to the function is the exact same value returned from a previous
 * liballoc_alloc call.
 *
 * The integer value is the number of pages to free.
 *
 * \return 0 if the memory was successfully freed.
 */
//L4_INLINE void L4_Unmap (L4_Word_t n, L4_Fpage_t * fpages)
extern int liballoc_free(void* aPrevCallVal,int aPagesToFree) {
//L4_Unmap(aPagesToFree,aPrevCallVal);
L4_Unmap(aPagesToFree);

return 0;
}

