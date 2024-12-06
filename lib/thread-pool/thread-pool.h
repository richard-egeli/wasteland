#ifndef LIB_THREAD_POOL_THREAD_POOL
#define LIB_THREAD_POOL_THREAD_POOL

#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>

#define MAX_THREAD_POOL_TASKS 256
#define THREAD_COUNT          8

typedef struct Task {
    void (*function)(void*);
    void* argument;
    atomic_bool completed;
} Task;

typedef struct ThreadPool {
    Task tasks[MAX_THREAD_POOL_TASKS];
    pthread_t threads[THREAD_COUNT];
    atomic_int head;
    atomic_int tail;
    atomic_bool stop;
} ThreadPool;

Task* thread_pool_push(ThreadPool* pool, void (*function)(void*), void* arg);

Task* thread_pool_pop(ThreadPool* pool);

void thread_pool_wait(ThreadPool* pool, Task* task);

bool thread_pool_complete(ThreadPool* pool, Task* task);

void thread_pool_destroy(ThreadPool* pool);

ThreadPool* thread_pool_create();

#endif  // LIB_THREAD_POOL_THREAD_POOL
