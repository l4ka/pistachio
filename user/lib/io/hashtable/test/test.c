#if 0
/* Hash test functions */
#include <assert.h>

#include "hash.h"
#include <stdio.h>

struct teststruct {
	int key;
	void *value;
};

struct teststruct test_1[] = {
	{1, (void *)123},
	{2, (void *)456},
	{1024, (void *)1024},
	{8192, (void *)8192},
	{4962, (void *)929384},
	{0, 0} /* sentinel */
};

void
simpletest (void)
{
	int counter;
	struct hashtable *table;
	
	table = hash_init (1024);
	/* Insert some numbers and read them back. */
	for (counter=0; test_1[counter].key != 0; counter++) {
		hash_insert (table, test_1[counter].key, test_1[counter].value);
	}
	for (counter=0; test_1[counter].key != 0; counter++) {
		assert (hash_lookup (table, test_1[counter].key) == test_1[counter].value);
	}
	hash_free (table);
}

void 
stresstest (void)
{
	int counter;
	struct hashtable *table;
		
	table = hash_init (1024);
	/* Insert heaps of numbers and ensure we can read them all back */
	for (counter=0; counter<102400; counter++) {
		hash_insert (table, counter, (void *)~counter);
		assert (hash_lookup (table, counter) == (void *)~counter);
	}
	hash_free (table);
}
#if 0
int main ()
{
	simpletest ();
	stresstest ();
	printf ("Tests passed.\n");
	
	return 0;
}
#endif
#endif
