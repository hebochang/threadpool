#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>

#include "pthreadpool.h"
#include "condition.h"

void thread_routine(void *arg)
{
    struct timespec abstime;
    int timeout;
    printf("thread 0x%x is starting\n", (int)pthread_self());
    threadpool_t *pool = (threadpool_t *)arg;
    while(1)
    {
        condition_lock(&pool->ready);
        pool->idle++;
        while(!pool->first && !pool->quit)
        {
            printf("thread 0x%x is waiting\n", (int)pthread_self());
            //condition_wait(&pool->ready);
            clock_gettime(CLOCK_REALTIME, &abstime);
            abstime.tv_nsec += 2;
            int status = condition_timedwait(&pool->ready, &abstime);
            if(status == ETIMEDOUT){
                printf("thread 0x%x is wait timed out\n", (int)pthread_self());
                timeout = 1;
                break;
            }
        }

        pool->idle--;

        //one kind
        if(pool->first)
        {
            task_t *t = pool->first;
            pool->first = t->next;
            //run task
            condition_unlock(&pool->ready);
            t->run(t->arg);
            condition_lock(&pool->ready);
        }

        //tow kind
        //no task and quit
        if(pool->quit && !pool->first)
        {
            pool->counter--;

            if(!pool->counter){
                condition_signal(&pool->ready);
            }

            condition_unlock(&pool->ready);
            break;
        }

        if(timeout && !pool->first){
            pool->counter--;
            condition_unlock(&pool->ready);

            break;
        }

        condition_unlock(&pool->ready);
    }

    printf("tread 0x%x is exting\n", (int)pthread_self());
    return;
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
    task_t *newtask = (task_t *)malloc(sizeof(task_t));
    newtask->run = run;
    newtask->arg = arg;
    newtask->next = NULL;

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
        pthread_create(&tid, NULL, (void *)thread_routine, pool);
        pool->counter++;
    }


    condition_unlock(&pool->ready);
}

void threadpool_destroy(threadpool_t *pool){
    if(pool->quit)
    {
        return;
    }

    condition_lock(&pool->ready);

    pool->quit = 1;
    if(pool->counter > 0){
        if(pool->idle > 0)
            condition_broadcast(&pool->ready);

        //wait working task
        while(pool->counter > 0)
        {
            condition_wait(&pool->ready);
        }
    }

    condition_unlock(&pool->ready);
    condition_destroy(&pool->ready);
}
