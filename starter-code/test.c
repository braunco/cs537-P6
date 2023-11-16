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
        safequeue_enqueue(&q, data, i);  // Include priority
        printf("Enqueued: %d with priority %d\n", *data, i);
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
        safequeue_enqueue(&q, data, i);  // Include priority
        printf("Enqueued: %d with priority %d\n", *data, i);
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
    safequeue_enqueue(&q, extra_data, 5);
    printf("Enqueued: %d with priority %d\n", *extra_data, 5);

    // Dequeue remaining items
    while (!safequeue_is_empty(&q)) {
        int *data = safequeue_dequeue(&q);
        printf("Dequeued: %d\n", *data);
        free(data);
    }

    safequeue_destroy(&q);
}

void test_queue_empty() {
    printf("\nTest 3: Queue Empty\n");
    safequeue_t q;
    safequeue_init(&q, MAX_SIZE);

    // Attempt to dequeue from an empty queue
    printf("Attempting to dequeue from empty queue...\n");
    if (safequeue_is_empty(&q)) {
        printf("Queue is empty, cannot dequeue.\n");
    } else {
        int *data = safequeue_dequeue(&q);
        printf("Dequeued: %d\n", *data);
        free(data);
    }

    // Enqueue an item to allow the dequeue operation to complete
    int *extra_data = malloc(sizeof(int));
    *extra_data = 1;
    safequeue_enqueue(&q, extra_data, 1);  // Include priority
    printf("Enqueued: %d with priority %d (to allow dequeue from empty queue)\n", *extra_data, 99);

    // Dequeue the item
    if (!safequeue_is_empty(&q)) {
        int *data = safequeue_dequeue(&q);
        printf("Dequeued: %d\n", *data);
        free(data);
    }

    safequeue_destroy(&q);
}

void test_priority_queue() {
    printf("\nTest 4: Priority Queue\n");
    safequeue_t q;
    safequeue_init(&q, MAX_SIZE);

    // Enqueue items with different priorities
    int priorities[] = {3, 1, 4, 2, 5};
    for (int i = 0; i < 5; i++) {
        int *data = malloc(sizeof(int));
        *data = i;
        safequeue_enqueue(&q, data, priorities[i]);
        printf("Enqueued: %d with priority %d\n", *data, priorities[i]);
    }

    // Dequeue items and check if they are in priority order
    printf("Dequeuing in priority order:\n");
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
    test_queue_empty();
    test_priority_queue();
    return 0;
}
