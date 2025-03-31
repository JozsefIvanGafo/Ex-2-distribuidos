#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <pthread.h>
#include <stdbool.h>

#define MAX_THREADS 10
#define MAX_QUEUE 256

typedef struct {
    void (*function)(void*);
    void* argument;
} threadpool_task_t;

typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t notify;
    pthread_t *threads;
    threadpool_task_t *queue;
    int thread_count;
    int queue_size;
    int head;
    int tail;
    int count;
    bool shutdown;
    bool started;
} threadpool_t;

threadpool_t* threadpool_create(int thread_count);
void threadpool_destroy(threadpool_t *pool);
int threadpool_add(threadpool_t *pool, void (*function)(void*), void *argument);

#endif