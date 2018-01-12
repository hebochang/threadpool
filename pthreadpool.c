#include <string.h>
#include <errno.h>
#include <time.h>

#include "pthreadpool.h"
#include "condition.h"

void thread_routine(void *arg)
{
    printf("thread 0x%x is working on task ", (int)pthread_self(), (int *)arg);
    threadpool_t *pool = (threadpool_t *)arg;
    while(1)
    {
        condition_lock(&pool->ready);
        pool->idle++;
        while(!pool->first && pool->quit)
        {
            condition_wait(&pool->ready);
        }

        pool->idle--;
        if(pool->first != NULL)
        {
            task_t *t = pool->first;
            pool->first = t->next;
            condition_unlock(&pool->ready);
            t->run(t->arg);
            condition_lock(&pool->ready);
        }

        if(pool->quit && !pool->first)
        {
            pool->counter--;
            condition_unlock(&pool->ready);
            break;
        }

        condition_unlock(&pool->ready);
    }

    printf("tread 0x%x is wxting\n", (int)pthread_self());
    return NULL;
}

void threadpool_init(threadpool_t *pool, int threads){
    condition_init(&pool->ready);
    pool->first = NULL;
    pool->last = NULL;
    pool->counter = 0;
    pool->idle = 0;
    pool->max_treads = threads;
    pool->quit = 0;
}

void threadpool_add_task(threadpool_t *pool, void *(*run)(void *arg), void *arg){
    task_t *newtask = (task_t *)malloc(task_t);
    newtask->run = run;
    newtask->arg = arg;
    task->next = NULL;

    condition_lock(&pool->ready);
    if(!pool->first)
        pool->first = newtask;
    else
        pool->last->next = newtask;
    pool->last = newtask;

    if(pool->idle > 0){
        condition_signal(&pool->ready);
    }
    else if(pool->counter < pool->max_treads){
        pthread_t tid;
        pthread_create(&tid, NULL, thread_routine, pool);
        pool->counter++;
    }


    condition_unlock(&pool->ready);
}

void threadpool_destroy(threadpool_t *pool){

}
