#include "safequeue.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define MAX_SIZE 5

void test_basic_functionality() {
    printf("Test 1: Basic Functionality\n");
    safequeue_t q;
    safequeue_init(&q, MAX_SIZE);

    // Enqueue some items
    for (int i = 0; i < 5; i++) {
        int *data = malloc(sizeof(int));
        *data = i;
        safequeue_enqueue(&q, data);
        printf("Enqueued: %d\n", *data);
    }

    // Dequeue all items
    while (!safequeue_is_empty(&q)) {
        int *data = safequeue_dequeue(&q);
        printf("Dequeued: %d\n", *data);
        free(data);
    }

    safequeue_destroy(&q);
}

void test_queue_full() {
    printf("\nTest 2: Queue Full\n");
    safequeue_t q;
    safequeue_init(&q, MAX_SIZE);

    // Enqueue items until the queue is full
    for (int i = 0; i < MAX_SIZE; i++) {
        int *data = malloc(sizeof(int));
        *data = i;
        safequeue_enqueue(&q, data);
        printf("Enqueued: %d\n", *data);
    }

    // Dequeue one item to make space
    if (!safequeue_is_empty(&q)) {
        int *data = safequeue_dequeue(&q);
        printf("(Normally will wait here - Dequeueing to make space): %d\n", *data);
        free(data);
    }

    // Try to enqueue another item
    int *extra_data = malloc(sizeof(int));
    *extra_data = 5;
    safequeue_enqueue(&q, extra_data);
    printf("Enqueued: %d\n", *extra_data);

    // Dequeue remaining items
    while (!safequeue_is_empty(&q)) {
        int *data = safequeue_dequeue(&q);
        printf("Dequeued: %d\n", *data);
        free(data);
    }

    safequeue_destroy(&q);
}

int main() {
    test_basic_functionality();
    test_queue_full();
    return 0;
}
