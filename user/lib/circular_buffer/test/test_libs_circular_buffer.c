#include <cb.h>
#include <stdio.h>
#include <stdlib.h>
#include <check.h>
#include "test_libs_circular_buffer.h"

#define CB_TEST_ALLOC_SIZE 0x1000
#define CB_TEST_TEST_SIZE 0x100

#define CB_TEST_CB_TEST_GET_WRAP_SIZE 200

struct cb_test_size_val {
	int size;
	void *val;
};


START_TEST(test_cb_wrap_simple)
{
	/* Allocate a CB, fill it, retrieve, then ensure that when we wrap
	 * after calling "get" we don't get spurious wrapping. 
	*/
	int i;
	int element_size = (CB_TEST_CB_TEST_GET_WRAP_SIZE / 10) + 1;
	struct cb_alloc_handle *cba;
	struct cb_get_handle *cbg;
	void *val;
	cba = cb_new(CB_TEST_CB_TEST_GET_WRAP_SIZE);
	fail_if(cba == NULL, "Couldn't allocate a CB");
	cbg = cb_attach(cb_get_cb(cba));
	fail_if(cbg == NULL, "Couldn't get a 'get' handle");
	
	for (i=0; i<9; i++) {
		val = cb_alloc(cba, element_size);
		cb_sync_alloc(cba);
	}
	for (i=0; i<9; i++) {
		val = cb_get(cbg, element_size);
		cb_sync_get(cbg);
		fail_if(val == NULL, "Couldn't grab data");
	}
	val = cb_get(cbg, element_size);
	fail_unless(val == NULL, "Got more data than was stored");
	cb_free(cba);
}
END_TEST


START_TEST(test_cb_wrap_sequential)
{
	int i;
	int element_size = (CB_TEST_CB_TEST_GET_WRAP_SIZE / 10) + 1;
	struct cb_alloc_handle *cba;
	struct cb_get_handle *cbg;
	void *val;
	/* get and retrieve in sequence. */
	cba = cb_new(CB_TEST_CB_TEST_GET_WRAP_SIZE);
	fail_if(cba == NULL, "Couldn't allocate CB");
	cbg = cb_attach(cb_get_cb(cba));
	fail_if(cbg == NULL, "Couldn't get a 'get' handle");
	for (i=0; i<9; i++) {
		val = cb_alloc(cba, element_size);
		cb_sync_alloc(cba);
		fail_if(val == NULL, "Couldn't store value");
		val = cb_get(cbg, element_size);
		cb_sync_get(cbg);
		fail_if(val == NULL, "Couldn't retrieve value");
	}
	val = cb_get(cbg, element_size);
	fail_unless(val == NULL, "Got more data than was stored");
	cb_free(cba);
}
END_TEST

START_TEST(test_cb_wrap_sequential_2)
{
	int i;
	int element_size = (CB_TEST_CB_TEST_GET_WRAP_SIZE / 10) + 1;
	struct cb_alloc_handle *cba;
	struct cb_get_handle *cbg;
	void *val;
	/* ensure we can't overwrite the first element when wrapping.*/
	cba = cb_new(CB_TEST_CB_TEST_GET_WRAP_SIZE);
	fail_if(cba == NULL, "Couldn't allocate CB");
	cbg = cb_attach(cb_get_cb(cba));
	fail_if(cbg == NULL, "Couldn't get a 'get' handle");
	for (i=0; i<9; i++) {
		val = cb_alloc(cba, element_size);
		//printf("Ate %d bytes\n", element_size);
		cb_sync_alloc(cba);
		fail_if(val == NULL, "Couldn't store value");
	}
	val = cb_alloc(cba, element_size);
	cb_sync_alloc(cba);
	fail_unless(val == NULL, "Stored more than length of buffer");
	cb_free(cba);

	return;
}
END_TEST

START_TEST(test_cb_basic)
{
	struct cb_alloc_handle *cba;
	struct cb_get_handle *cbg;
	struct cb_test_size_val *size_val;
	void *values[CB_TEST_ALLOC_SIZE / CB_TEST_TEST_SIZE - 1];
	int i;
	void *val;
	
	size_val = malloc(sizeof(struct cb_test_size_val) * CB_TEST_ALLOC_SIZE);

	/* Test we get valid things to start with */
	cba = cb_new(CB_TEST_ALLOC_SIZE);
	fail_if(cba == NULL, "Couldn't grab memory for buffer.");
	cbg = cb_attach(cb_get_cb(cba));
	fail_if(cbg == NULL, "Couldn't get a 'get' handle.");

	/* Allocate as many as we can successfully */
	for (i=0; i < CB_TEST_ALLOC_SIZE / CB_TEST_TEST_SIZE - 1; i++) {
		values[i] = val = cb_alloc(cba, CB_TEST_TEST_SIZE);
		fail_if(val == 0, "Couln't allocate memory in CB.");
	}
	/* Check allocate the next one fails */
	val = cb_alloc(cba, CB_TEST_TEST_SIZE);
	fail_unless(val == NULL, "Allocated more than we should have!");

	/* Try to get one before we sync.. should fail */
	val = cb_get(cbg, CB_TEST_TEST_SIZE);
	fail_unless(val == NULL, "Managed to grab CB data before sync alloc!");

	/* Now sync */
	cb_sync_alloc(cba);

	/* Getting on should work */
	val = cb_get(cbg, CB_TEST_TEST_SIZE);
	fail_if(val == NULL, "Got NULL from cb_get.");
	fail_unless(val == values[0], "Returned data doesn't match stored data");

	/* Try to allocate a new one, should fail again */
	val = cb_alloc(cba, CB_TEST_TEST_SIZE);
	fail_unless(val == NULL, "Managed to allocate more than buffer size.");

	cb_sync_get(cbg);

	/* Now that we have synced it should work */
	values[0] = val = cb_alloc(cba, CB_TEST_TEST_SIZE);
	fail_if(val == NULL, "Couldn't allocate after cb_sync_get.");

	/* Now get the rest */
	for (i=0; i < CB_TEST_ALLOC_SIZE / CB_TEST_TEST_SIZE - 2; i++) {
		val = cb_get(cbg, CB_TEST_TEST_SIZE);
		fail_unless(values[i+1] == val, "Returned data doesn't match stored data");
	}

	/* Now sync */
	cb_sync_alloc(cba);

	val = cb_get(cbg, CB_TEST_TEST_SIZE);
	fail_unless(values[0] == val, "Returned data doesn't match stored data");
	val = cb_get(cbg, CB_TEST_TEST_SIZE);
	fail_unless(val == NULL, "Managed to get more than was allocated");

	cb_sync_get(cbg);

	/* Now test size-1 so its not aligned*/
	for (i=0; i < CB_TEST_ALLOC_SIZE / CB_TEST_TEST_SIZE - 1; i++) {
		//printf("Testing %d of %d\n",i, CB_TEST_ALLOC_SIZE / CB_TEST_TEST_SIZE - 1);
		values[i] = val = cb_alloc(cba, CB_TEST_TEST_SIZE - 1);
		fail_if(val == 0, "Couldn't allocated after sync_get");
	}

	cb_sync_alloc(cba);

	for (i=0; i < CB_TEST_ALLOC_SIZE / CB_TEST_TEST_SIZE - 1; i++) {
		val = cb_get(cbg, CB_TEST_TEST_SIZE - 1);
		fail_unless(values[i] == val, "Returned data doesn't match stored data");
	}

	cb_sync_get(cbg);

	/* Randomly allocate until we are full, then free */
	i = 0;
	do {
		size_val[i].size = rand() % 370;
		size_val[i].val = cb_alloc(cba, size_val[i].size);
	} while(size_val[i++].val != NULL);

	cb_sync_alloc(cba);

	i = 0;
	do {
		val = cb_get(cbg, size_val[i].size);
		fail_unless(val == size_val[i].val, NULL);
	} while(size_val[i++].val != NULL);

	cb_sync_get(cbg);
	
	return;
}
END_TEST

Suite *
make_test_libs_circular_buffer_suite(void)
{
	Suite *suite;
	TCase *tc;

	suite = suite_create("CB tests");
	tc = tcase_create("Core");
	tcase_add_test(tc, test_cb_basic);
	tcase_add_test(tc, test_cb_wrap_simple);
	tcase_add_test(tc, test_cb_wrap_sequential);
	tcase_add_test(tc, test_cb_wrap_sequential_2);
	suite_add_tcase(suite, tc);
	return suite;
}

