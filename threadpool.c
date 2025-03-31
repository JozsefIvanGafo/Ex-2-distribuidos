#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "threadpool.h"

static void *threadpool_thread(void *threadpool);

threadpool_t *threadpool_create(int thread_count) {
    if (thread_count <= 0 || thread_count > MAX_THREADS)
        return NULL;

    threadpool_t *pool = calloc(1, sizeof(threadpool_t));
    if (pool == NULL)
        return NULL;

    pool->thread_count = thread_count;
    pool->queue_size = MAX_QUEUE;

    pool->threads = calloc(thread_count, sizeof(pthread_t));
    pool->queue = calloc(MAX_QUEUE, sizeof(threadpool_task_t));

    if (pool->threads == NULL || pool->queue == NULL) {
        free(pool->threads);
        free(pool->queue);
        free(pool);
        return NULL;
    }

    if (pthread_mutex_init(&(pool->lock), NULL) != 0 ||
        pthread_cond_init(&(pool->notify), NULL) != 0) {
        free(pool->threads);
        free(pool->queue);
        free(pool);
        return NULL;
    }

    for (int i = 0; i < thread_count; i++) {
        if (pthread_create(&(pool->threads[i]), NULL, threadpool_thread, pool) != 0) {
            threadpool_destroy(pool);
            return NULL;
        }
        pool->started = pool->started + 1; // Fixed boolean increment warning
    }

    return pool;
}

void threadpool_destroy(threadpool_t *pool) {
    if (pool == NULL)
        return;

    pthread_mutex_lock(&(pool->lock));
    pool->shutdown = true;
    pthread_cond_broadcast(&(pool->notify));
    pthread_mutex_unlock(&(pool->lock));

    for (int i = 0; i < pool->thread_count; i++) {
        pthread_join(pool->threads[i], NULL);
    }

    pthread_mutex_destroy(&(pool->lock));
    pthread_cond_destroy(&(pool->notify));
    free(pool->threads);
    free(pool->queue);
    free(pool);
}

int threadpool_add(threadpool_t *pool, void (*function)(void*), void *argument) {
    if (pool == NULL || function == NULL)
        return -1;

    pthread_mutex_lock(&(pool->lock));

    if (pool->count >= pool->queue_size) {
        pthread_mutex_unlock(&(pool->lock));
        return -1;
    }

    pool->queue[pool->tail].function = function;
    pool->queue[pool->tail].argument = argument;
    pool->tail = (pool->tail + 1) % pool->queue_size;
    pool->count++;

    pthread_cond_signal(&(pool->notify));
    pthread_mutex_unlock(&(pool->lock));

    return 0;
}

static void *threadpool_thread(void *threadpool) {
    threadpool_t *pool = (threadpool_t *)threadpool;
    threadpool_task_t task;

    while (true) {
        pthread_mutex_lock(&(pool->lock));

        while (pool->count == 0 && !pool->shutdown) {
            pthread_cond_wait(&(pool->notify), &(pool->lock));
        }

        if (pool->shutdown) {
            pthread_mutex_unlock(&(pool->lock));
            break;
        }

        task.function = pool->queue[pool->head].function;
        task.argument = pool->queue[pool->head].argument;
        pool->head = (pool->head + 1) % pool->queue_size;
        pool->count--;

        pthread_mutex_unlock(&(pool->lock));
        task.function(task.argument);
    }

    return NULL;
}