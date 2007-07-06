/* architecture specific 'helper' functions */

#include <l4/types.h>
#include <l4io.h>
#include <arch.h>

#include "../l4test.h"
#include "../assert.h"


#if 0
/* generic stuff */
#include "../../generic/l4test.h"
#include "../../generic/menu.h"
#include "../../generic/assert.h"
#endif


#if 0
void 
setup_exreg( L4_Word_t *ip, L4_Word_t *sp, void (*func)(void) )
{
    printf("setup_exreg"); for (;;);
}
#endif

/* setup an exreg 
 *
 * *sp == NULL: allocate a new stack
 * *sp != NULL: top of sp from an old exreg used, just refresh it!
 */
void 
setup_exreg( L4_Word_t *ip, L4_Word_t *sp, void (*func)(void) )
{
	L4_Word_t *stack;
	int max;
        static unsigned int l4test_section = 0x00100000;
        static unsigned int l4stack_section = 0x30100000;

	/* allocate a stack, or perhaps not */
	max = STACK_PAGES * PAGE_SIZE / sizeof( L4_Word_t );
	if( *sp == NULL )
	{
		stack = (L4_Word_t*) get_pages( STACK_PAGES, 1 );
		assert( stack != NULL );

		stack = &stack[max-1];
	}
	else
		stack = (L4_Word_t*) *sp;

	/* put the real IP on the stack */
	*ip = stack[0] = ((L4_Word_t) func );
        l4test_section += 0x00100000;

	/* set the return params */
	*sp = (L4_Word_t) (((unsigned int)stack ) );
        l4stack_section += 0x00100000;

        //printf("setup_exreg: returning ip = %x sp = %x\n", *ip, *sp);
}

/* XXX Just return the entry point */
void *
code_addr( void *addr )
{
	return addr;
}

void
get_startup_values (void (*func)(void), L4_Word_t * ip, L4_Word_t * sp)
{
    // Calculate intial SP      
    L4_Word_t stack = (L4_Word_t) get_pages (STACK_PAGES, 1);
    stack += STACK_PAGES * PAGE_SIZE;

    *ip = (L4_Word_t) func;
    *sp = stack;       
}

