#include "test_libs_mutex.h"
#include <mutex/mutex.h>
#include <l4/thread.h>
#include <check.h>

START_TEST(test_mutex_basic)
{
	/* Simple create mutex, release */
	struct mutex m;
	mutex_t mtx = &m;

	mutex_init(mtx);
	fail_if(mtx->holder != 0, "Mutex has been initialised in locked state");
	fail_if(mtx->needed != 0, "Mutex has not been initialised properly");
	fail_if(mtx->count != 0, "Mutex has not been initialised properly");

	mutex_lock(mtx);
	fail_if(mtx->holder == 0, "Mutex has not been locked");
	fail_if(mtx->holder != L4_Myself().raw, "This thread doesn't hold the mutex");

	mutex_unlock(mtx);
	fail_if(mtx->holder != 0, "Mutex has not been unlocked properly");
	fail_if(mtx->needed != 0, "Mutex has not been unlocked properly");
	fail_if(mtx->count != 0, "Mutex has not been unlocked properly");
}
END_TEST

START_TEST(test_mutex_count_lock)
{
	struct mutex m;
	mutex_t mtx = &m;
	L4_Word_t me = L4_Myself().raw;
	
	mutex_init(mtx);

	mutex_count_lock(mtx);
	fail_if(mtx->holder != me, "Mutex is not held by me");
	fail_if(mtx->count != 1, "Mutex count is wrong");

	mutex_count_lock(mtx);
	fail_if(mtx->count != 2, "Mutex count is wrong");
	
	mutex_count_lock(mtx);
	fail_if(mtx->count != 3, "Mutex count is wrong");

	mutex_count_unlock(mtx);
	fail_if(mtx->holder != me, "Mutex is not held by me");
	fail_if(mtx->count != 2, "Mutex count is wrong");
	
	mutex_count_unlock(mtx);
	fail_if(mtx->holder != me, "Mutex is not held by me");
	fail_if(mtx->count != 1, "Mutex count is wrong");
	
	mutex_count_unlock(mtx);
	fail_if(mtx->holder != 0, "Mutex has not been unlocked properly");
	fail_if(mtx->count != 0, "Mutex count is wrong");
}
END_TEST

Suite *
make_test_libs_mutex_suite(void)
{
	Suite *suite;
	TCase *tc;

	suite = suite_create("mutex tests");
	tc = tcase_create("Core"); 
	tcase_add_test(tc, test_mutex_basic);
	tcase_add_test(tc, test_mutex_count_lock);
	suite_add_tcase(suite, tc);
	return suite;
}
