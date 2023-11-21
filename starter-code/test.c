#include "safequeue.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdbool.h>
#include <unistd.h>

#define MAX_SIZE 5

void test_basic_functionality() {
    printf("Test 1: Basic Functionality\n");
    safequeue_t q;
    create_queue(&q, MAX_SIZE);
    create_queue(&q, MAX_SIZE);

    // Enqueue some items
    for (int i = 0; i < 5; i++) {
        char* test = malloc(sizeof(char) * 10);
        sprintf(test, "%d", i);
        add_work(&q, test, i);  // Include priority
        printf("Enqueued: %s with priority %d\n", test, i);
    }

    // Dequeue all items
    while (!safequeue_is_empty(&q)) {
        char* data = get_work_blocking(&q);
        printf("Dequeued: %s\n", data);
    }

    // for(int i=4; i>=0; i--) {
    //     add_work(&q, i+10, i);
    //     printf("Enqueued: %d with priority %d\n", i+10, i);
    // }

    // while (!safequeue_is_empty(&q)) {
    //     int data = get_work_nonblocking(&q);
    //     printf("Dequeued: %d\n", data);
    // }

    destroy_queue(&q);
}

// void test_queue_full() {
//     printf("\nTest 2: Queue Full\n");
//     safequeue_t q;
//     create_queue(&q, MAX_SIZE);

//     // Enqueue items until the queue is full
//     for (int i = 0; i < MAX_SIZE; i++) {
//         add_work(&q, i, i);  // Include priority
//         printf("Enqueued: %d with priority %d\n", i, i);
//     }

//     // Dequeue one item to make space
//     if (!safequeue_is_empty(&q)) {
//         int data = get_work_nonblocking(&q);
//         printf("(Normally will wait here - Dequeueing to make space): %d\n", data);
//     }

//     // Try to enqueue another item
//     int extra_data = 5;
//     add_work(&q, extra_data, 5);
//     printf("Enqueued: %d with priority %d\n", extra_data, 5);

//     // try to add another one... should deadlock
//     int another_one = 8;
//     add_work(&q, another_one, 8);
//     printf("SHOULD NOT GET HERE\n");

//     // Dequeue remaining items
//     while (!safequeue_is_empty(&q)) {
//         int data = get_work_nonblocking(&q);
//         printf("Dequeued: %d\n", data);
//     }

//     destroy_queue(&q);
// }

// void test_queue_empty() {
//     printf("\nTest 3: Queue Empty\n");
//     safequeue_t q;
//     create_queue(&q, MAX_SIZE);

//     // Attempt to dequeue from an empty queue (nonblocking)
//     printf("Attempting to dequeue from empty queue...\n");
//     int data = get_work_nonblocking(&q);
//     printf("Dequeued: %d\n", data);

//     // attempt to deqeue (blocking -> should deadlock)
//     int next = get_work_blocking(&q);
//     printf("SHOULD NOT GET HERE: %d\n", next);

//     // // Enqueue an item to allow the dequeue operation to complete
//     // int *extra_data = malloc(sizeof(int));
//     // *extra_data = 1;
//     // safequeue_enqueue(&q, extra_data, 1);  // Include priority
//     // printf("Enqueued: %d with priority %d (to allow dequeue from empty queue)\n", *extra_data, 99);

//     // // Dequeue the item
//     // if (!safequeue_is_empty(&q)) {
//     //     int *data = safequeue_dequeue(&q);
//     //     printf("Dequeued: %d\n", *data);
//     //     free(data);
//     // }

//     destroy_queue(&q);
// }

// void test_priority_queue() {
//     printf("\nTest 4: Priority Queue\n");
//     safequeue_t q;
//     create_queue(&q, MAX_SIZE);

//     // Enqueue items with different priorities
//     int priorities[] = {3, 1, 4, 2, 5};
//     for (int i = 0; i < 5; i++) {
//         int data = i;
//         add_work(&q, data, priorities[i]);
//         printf("Enqueued: %d with priority %d\n", data, priorities[i]);
//     }

//     // Dequeue items and check if they are in priority order
//     printf("Dequeuing in priority order:\n");
//     while (!safequeue_is_empty(&q)) {
//         int data = get_work_blocking(&q);
//         printf("Dequeued: %d\n", data);
//     }

//     destroy_queue(&q);
// }


// safequeue_t q;
// void* listen_thread1(void* arg) {
//     //add to max
//     for(int i=0; i<MAX_SIZE; i++) {
//         add_work(&q, i+10, i);
//     }
//     printf("Done adding 5\n");
//     add_work(&q, 15, 5);
//     printf("Add work worked.\n");

//     return NULL;
// }

// void* listen_thread2(void* arg) {
//     sleep(20);
//     printf("adding another\n");
//     add_work(&q, 100, 100);
//     printf("added another\n");


//     return NULL;
// }

// void* worker_thread1(void* arg) {
//     sleep(10);
//     printf("Done sleeping\n");
//     int val = get_work_blocking(&q);
//     if(val != 14) {
//         printf("Shoulda got 14, got %d instead\n", val);
//     }
//     while(!safequeue_is_empty(&q)) {
//         val = get_work_blocking(&q);
//         printf("got %d from queue\n", val);
//     }

//     //now should be empty, try to get from queue and should block
//     printf("waiting for another...\n");
//     val = get_work_blocking(&q);
//     printf("got another: %d\n", val);

//     return NULL;
// }

// void* worker_thread2(void* arg) {

//     return NULL;
// }

// void test_threads() {
//     create_queue(&q, MAX_SIZE);

//     pthread_t listen1, listen2, worker1, worker2;

//     //create 2 'listener' threads
//     if(pthread_create(&listen1, NULL, listen_thread1, NULL) < 0) {
//         printf("BRUH1\n");
//     }
//     if(pthread_create(&listen2, NULL, listen_thread2, NULL) < 0) {
//         printf("BRUH2\n");
//     }

//     //create 2 'worker' threads
//     if(pthread_create(&worker1, NULL, worker_thread1, NULL) < 0) {
//         printf("BRUH3\n");
//     }
//     if(pthread_create(&worker2, NULL, worker_thread2, NULL) < 0) {
//         printf("BRUH4\n");
//     }

//     pthread_join(listen1, NULL);
//     pthread_join(listen2, NULL);
//     pthread_join(worker1, NULL);
//     pthread_join(worker2, NULL);

//     printf("Done\n");
// }

// bool listen1go = true;
// bool listen2go = false;
// bool worker1go = false;
// bool worker2go = false;


int main() {
    test_basic_functionality();
    //test_queue_full();
    //test_queue_empty();
    //test_priority_queue();
    //test_threads();
    return 0;
}