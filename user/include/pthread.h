/* Enryo/L4 implementation of pthreads */

#pragma once

#include <sys/types.h>
#include <mutex/mutex.h>

#ifdef __cplusplus
extern "C" {
#endif


int pthread_mutex_destroy(pthread_mutex_t *mutex);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_trylock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);

#ifdef __cplusplus
}
#endif

