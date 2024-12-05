#include "thread-pool.h"

#include <assert.h>
#include <pthread/sched.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void* worker_thread(void* arg) {
    ThreadPool* pool = (ThreadPool*)arg;

    while (!atomic_load(&pool->stop)) {
        int tail = atomic_load(&pool->tail);
        int head = atomic_load(&pool->head);

        if (head == tail) {
            sched_yield();
            continue;
        }

        int desired = (tail + 1) % MAX_THREAD_POOL_TASKS;
        if (atomic_compare_exchange_strong(&pool->tail, &tail, desired)) {
            Task* task = &pool->tasks[tail];

            if (task->function) {
                task->function(task->argument);
                atomic_store(&task->completed, true);
            }
        }
    }

    return NULL;
}

Task* thread_pool_push(ThreadPool* pool, void (*function)(void*), void* args) {
    int head = atomic_load(&pool->head);
    int tail = atomic_load(&pool->tail);

    // Check if queue is full
    if ((head + 1) % MAX_THREAD_POOL_TASKS == tail) {
        return NULL;
    }

    // Try to push task using compare-and-swap
    if (atomic_compare_exchange_strong(&pool->head, &head, (head + 1) % MAX_THREAD_POOL_TASKS)) {
        Task task;
        atomic_init(&task.completed, false);
        task.function     = function;
        task.argument     = args;
        pool->tasks[head] = task;
        return &pool->tasks[head];
    }

    return NULL;
}

bool thread_pool_complete(ThreadPool* pool, Task* task) {
    return atomic_load(&task->completed);
}

void thread_pool_wait_all(ThreadPool* pool) {
    int tail = atomic_load(&pool->tail);
    int head = atomic_load(&pool->head);

    // Iterate over all tasks in the queue
    for (int i = tail; i != head; i = (i + 1) % MAX_THREAD_POOL_TASKS) {
        Task* task = &pool->tasks[i];
        // Wait for the task to complete
        while (!atomic_load(&task->completed)) {
            sched_yield();  // Yield to other threads
        }
    }
}

void thread_pool_task_wait(ThreadPool* pool, Task* result) {
    while (!atomic_load(&result->completed)) {
        sched_yield();  // Yield to other threads
    }
}

void thread_pool_destroy(ThreadPool* pool) {
    if (!pool) return;

    // Signal threads to stop
    atomic_store(&pool->stop, true);

    // Wait for all threads to finish
    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(pool->threads[i], NULL);
    }

    free(pool);
}

ThreadPool* thread_pool_create() {
    ThreadPool* pool = malloc(sizeof(ThreadPool));
    assert(pool != NULL && "Thread pool cannot be NULL");

    memset(pool, 0, sizeof(ThreadPool));
    atomic_init(&pool->head, 0);
    atomic_init(&pool->tail, 0);
    atomic_init(&pool->stop, false);

    for (int i = 0; i < THREAD_COUNT; i++) {
        if (pthread_create(&pool->threads[i], NULL, worker_thread, pool) != 0) {
            thread_pool_destroy(pool);
            return NULL;
        }
    }

    return pool;
}
