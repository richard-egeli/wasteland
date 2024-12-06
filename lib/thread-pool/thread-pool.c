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

        // Try to take this task
        Task* task = &pool->tasks[tail];

        // Advance tail BEFORE execution to prevent other threads from taking it
        atomic_store(&pool->tail, (tail + 1) % MAX_THREAD_POOL_TASKS);

        if (task->function) {
            task->function(task->argument);
            atomic_store(&task->completed, true);
        }
    }
    return NULL;
}

Task* thread_pool_push(ThreadPool* pool, void (*function)(void*), void* args) {
    int head      = atomic_load(&pool->head);
    int tail      = atomic_load(&pool->tail);
    int next_head = (head + 1) % MAX_THREAD_POOL_TASKS;

    if (next_head == tail) {
        return NULL;
    }

    // Set up task before advancing head
    atomic_store(&pool->tasks[head].completed, false);  // Ensure it starts as false
    pool->tasks[head].function = function;
    pool->tasks[head].argument = args;

    atomic_store(&pool->head, next_head);
    return &pool->tasks[head];
}

bool thread_pool_complete(ThreadPool* pool, Task* task) {
    return atomic_load(&task->completed);
}

void thread_pool_wait(ThreadPool* pool, Task* task) {
    while (!atomic_load(&task->completed)) {
        atomic_thread_fence(memory_order_acquire);
        sched_yield();
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
