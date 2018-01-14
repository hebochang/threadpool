#ifndef _PTHREAD_POOL_H_
#define _PTHREAD_POOL_H_

#include "condition.h"

typedef struct task
{
	void *(*run)(void *arg);
	void *arg;
	struct task *next;
}task_t;

typedef struct threadpool
{
	condition_t ready;	//任务准备就绪或者线程池销毁通知
	task_t *first;
	task_t *last;
	int counter;	//current count
	int idle;	//wait for task count
	int max_treads;	//max count
	int quit;	//销毁线程池设置为1
}threadpool_t;

void threadpool_init(threadpool_t *pool, int threads);

void threadpool_add_task(threadpool_t *pool, void *(*run)(void *arg), void *arg);

void threadpool_destroy(threadpool_t *pool);

#endif
