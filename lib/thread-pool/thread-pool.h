#ifndef LIB_THREAD_POOL_THREAD_POOL
#define LIB_THREAD_POOL_THREAD_POOL

#include <pthread.h>
#include <stdatomic.h>
#include <stddef.h>

#define MAX_THREAD_POOL_TASKS 256
#define THREAD_COUNT          8

typedef enum TaskStatus {
    TASK_PENDING,
    TASK_COMPLETED,
    TASK_ERROR,
} TaskStatus;

typedef struct TaskResult {
    void* data;
    size_t size;
    TaskStatus status;
} TaskResult;

typedef struct Task {
    void* (*function)(void*);
    void* argument;
    TaskResult* result;
    atomic_bool completed;
} Task;

typedef struct ThreadPool {
    Task tasks[MAX_THREAD_POOL_TASKS];
    pthread_t threads[THREAD_COUNT];
    atomic_int head;
    atomic_int tail;
    atomic_bool stop;
} ThreadPool;

int thread_pool_push(ThreadPool* pool, Task* task);

int thread_pool_pop(ThreadPool* pool);

void thread_pool_wait(ThreadPool* pool, Task* task);

TaskResult* thread_pool_submit(ThreadPool* pool, void* (*function)(void*), void* argument);

void thread_pool_destroy(ThreadPool* pool);

ThreadPool* thread_pool_create();

#endif  // LIB_THREAD_POOL_THREAD_POOL
