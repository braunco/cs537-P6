#include "safequeue.h"
#include <stdlib.h>
#include <stdio.h>


// Initialize the queue
/*Initializes the queue, setting head and tail to NULL, size to 0,
and initializing the mutex and condition variables*/
void create_queue(safequeue_t *q, int max_size) {
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
void destroy_queue(safequeue_t *q) {
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
void add_work(safequeue_t *q, char* filepath, int priority) {
    pthread_mutex_lock(&q->lock); //was causing deadlock 

    // Wait while queue is full
    while (q->size == q->max_size) {
        //printf("Entered while\n");
        //printf("Entered while\n");
        pthread_cond_wait(&q->not_full, &q->lock);
    }

    // Create a new node
    node_t *new_node = malloc(sizeof(node_t));
    new_node->filepath = filepath;
    new_node->priority = priority;
    new_node->next = NULL;
    //printf("Created a new node\n");

    // Find the correct position for the new node based on priority
    node_t **tracer = &q->head;
    while (*tracer != NULL && (*tracer)->priority >= priority) {
        //printf("Entered second while\n");
        //printf("Entered second while\n");
        tracer = &(*tracer)->next;
    }
    new_node->next = *tracer;
    *tracer = new_node;

    // Update tail if necessary
    if (q->tail == NULL || q->tail->next == new_node) {
        q->tail = new_node;
    }
    //printf("updated tail\n");

    q->size++;
    //printf("updated size\n");

    // Signal that the queue is not empty
    pthread_cond_signal(&q->not_empty);
    //printf("Sent signal\n");

    pthread_mutex_unlock(&q->lock);
    pthread_mutex_unlock(&q->lock);
}



// Dequeue data from the queue
/*Removes an element from the front of the queue. It waits if the queue is empty*/
char* get_work_blocking(safequeue_t *q) {
    pthread_mutex_lock(&q->lock);

    // Wait while queue is empty
    while (q->size == 0) {
        pthread_cond_wait(&q->not_empty, &q->lock);
    }

    // Remove the head node
    node_t *temp = q->head;
    char* filepath = temp->filepath;
    q->head = q->head->next;
    if (q->head == NULL) {
        q->tail = NULL;
    }
    free(temp);

    q->size--;

    // Signal that the queue is not full
    pthread_cond_signal(&q->not_full);

    pthread_mutex_unlock(&q->lock);

    return filepath;
}

//now returns -1 on fail.
char* get_work_nonblocking(safequeue_t *q) {
    pthread_mutex_lock(&q->lock);
    // Check if queue is empty and return immediately
    if (q->size == 0) {
        pthread_mutex_unlock(&q->lock);
        return NULL;
    }

    // Dequeue logic (similar to get_work_blocking)
    node_t *temp = q->head;
    char* filepath = temp->filepath;
    q->head = q->head->next;
    if (q->head == NULL) {
        q->tail = NULL;
    }

    q->size--;

    // Signal that the queue is not full
    pthread_cond_signal(&q->not_full);


    // Signal that the queue is not full
    pthread_cond_signal(&q->not_full);

    pthread_mutex_unlock(&q->lock);

    free(temp);  // Free the dequeued node
    return filepath; // Return the data of the dequeued node
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
   // printf("Entered size\n");
    pthread_mutex_lock(&q->lock);
    //printf("After size lock\n");
    int size = q->size;
    //printf("size: %d\n", size);
    pthread_mutex_unlock(&q->lock);
   // printf("After size unlock\n");
    return size;
}
