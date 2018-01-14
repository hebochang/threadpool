#ifndef _CONDITION_H_
#define _CONDITION_H_

#include <pthread.h>

//一把互斥锁+条件变量
typedef struct condition_t
{
	pthread_mutex_t pmutex;	//互斥锁
	pthread_cond_t pcond;	//条件变量
}condition_t;

int condition_init(condition_t *cond);
int condition_lock(condition_t *cond);
int condition_unlock(condition_t *cond);
int condition_wait(condition_t *cond);
int condition_timedwait(condition_t *cond, const struct timespec *abstime);
int condition_signal(condition_t *cond);
int condition_broadcast(condition_t *cond);
int condition_destroy(condition_t *cond);
#endif
