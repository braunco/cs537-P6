#include "safequeue.h"
#include <stdlib.h>

// Initialize the queue
/*Initializes the queue, setting head and tail to NULL, size to 0,
and initializing the mutex and condition variables*/
void safequeue_init(safequeue_t *q, int max_size) {
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
    q->max_size = max_size;
    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->not_empty, NULL);
    pthread_cond_init(&q->not_full, NULL);
}

// Destroy the queue
/*Frees all nodes in the queue and destroys the mutex and condition variables*/
void safequeue_destroy(safequeue_t *q) {
    // Free all nodes
    while (q->head != NULL) {
        node_t *temp = q->head;
        q->head = q->head->next;
        free(temp);
    }

    // Destroy mutex and condition variables
    pthread_mutex_destroy(&q->lock);
    pthread_cond_destroy(&q->not_empty);
    pthread_cond_destroy(&q->not_full);
}

// Enqueue data to the queue
/*Adds a new element to the end of the queue. It waits if the queue is full*/
void safequeue_enqueue(safequeue_t *q, void *data, int priority) {
    pthread_mutex_lock(&q->lock);

    // Wait while queue is full
    while (q->size == q->max_size) {
        pthread_cond_wait(&q->not_full, &q->lock);
    }

    // Create a new node
    node_t *new_node = malloc(sizeof(node_t));
    new_node->data = data;
    new_node->priority = priority;
    new_node->next = NULL;

    // Find the correct position to insert the new node
    // Higher number indicates higher priority
    node_t **tracer = &q->head;
    while (*tracer != NULL && (*tracer)->priority <= priority) {
        tracer = &(*tracer)->next;
    }
    new_node->next = *tracer;
    *tracer = new_node;

    if (q->tail == NULL || q->tail->priority < priority) {
        q->tail = new_node;
    }

    q->size++;

    // Signal that the queue is not empty
    pthread_cond_signal(&q->not_empty);

    pthread_mutex_unlock(&q->lock);
}


// Dequeue data from the queue
/*Removes an element from the front of the queue. It waits if the queue is empty*/
void *safequeue_dequeue(safequeue_t *q) {
    pthread_mutex_lock(&q->lock);

    // Wait while queue is empty
    while (q->size == 0) {
        pthread_cond_wait(&q->not_empty, &q->lock);
    }

    // Remove the head node
    node_t *temp = q->head;
    void *data = temp->data;
    q->head = q->head->next;
    if (q->head == NULL) {
        q->tail = NULL;
    }
    free(temp);

    q->size--;

    // Signal that the queue is not full
    pthread_cond_signal(&q->not_full);

    pthread_mutex_unlock(&q->lock);

    return data;
}

// Check if the queue is empty
/*Returns 1 if the queue is empty, otherwise 0.*/
int safequeue_is_empty(safequeue_t *q) {
    pthread_mutex_lock(&q->lock);
    int empty = (q->size == 0);
    pthread_mutex_unlock(&q->lock);
    return empty;
}

// Get the size of the queue
/*Returns the current size of the queue*/
int safequeue_size(safequeue_t *q) {
    pthread_mutex_lock(&q->lock);
    int size = q->size;
    pthread_mutex_unlock(&q->lock);
    return size;
}
