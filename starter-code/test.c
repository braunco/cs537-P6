#include "safequeue.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 10
#define OPERATIONS_PER_THREAD 100

// Thread function for enqueueing
void *enqueue_thread(void *arg) {
    safequeue_t *q = (safequeue_t *)arg;
    for (int i = 0; i < OPERATIONS_PER_THREAD; ++i) {
        int *data = malloc(sizeof(int));
        *data = i; // Assign some data
        safequeue_enqueue(q, data);
    }
    return NULL;
}

// Thread function for dequeueing
void *dequeue_thread(void *arg) {
    safequeue_t *q = (safequeue_t *)arg;
    for (int i = 0; i < OPERATIONS_PER_THREAD; ++i) {
        int *data = safequeue_dequeue(q);
        if (data) {
            // Process data
            free(data);
        }
    }
    return NULL;
}

int main() {
    safequeue_t q;
    safequeue_init(&q, NUM_THREADS * OPERATIONS_PER_THREAD);

    pthread_t threads[NUM_THREADS];

    // Create threads for enqueueing
    for (int i = 0; i < NUM_THREADS / 2; ++i) {
        pthread_create(&threads[i], NULL, enqueue_thread, &q);
    }

    // Create threads for dequeueing
    for (int i = NUM_THREADS / 2; i < NUM_THREADS; ++i) {
        pthread_create(&threads[i], NULL, dequeue_thread, &q);
    }

    // Join threads
    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_join(threads[i], NULL);
    }

    safequeue_destroy(&q);
    return 0;
}
