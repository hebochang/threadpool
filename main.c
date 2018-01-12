#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "pthreadpool.h"

void *mytask(void *arg){
    printf("thread 0x%x is working on task ", (int)pthread_self(), (int *)arg);
    sleep(1);
    free(arg);
    return NULL;
}

int main()
{
    threadpool_t pool;
    threadpool_init(&pool, 3);

    int i;
    for (i=0; i<10; i++){
        int *arg = (int *)malloc(sizeof(i));
        *arg = i;
        threadpool_add_task(&pool, mytask, arg);
    }

    threadpool_destroy(&pool);

    return 0;
}
