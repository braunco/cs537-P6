#ifndef SAFEQUEUE_H
#define SAFEQUEUE_H

#include <pthread.h>            // For mutexes and condition variables

// Define the node structure for the queue
typedef struct node {
    char* filepath;             // Pointer to the data
    int priority;               // Priority of the request
    struct node *next;          // Pointer to the next node
} node_t;

// Define the queue structure
typedef struct {
    node_t *head;               // Pointer to the head of the queue
    node_t *tail;               // Pointer to the tail of the queue
    int size;                   // Number of elements in the queue
    pthread_mutex_t lock;       // Mutex for thread-safe access
    pthread_cond_t not_empty;   // Condition variable to signal not empty
    pthread_cond_t not_full;    // Condition variable to signal not full
    int max_size;               // Maximum size of the queue (for bounded queues)
} safequeue_t;

// Function prototypes
void create_queue(safequeue_t *q, int max_size);
void destroy_queue(safequeue_t *q);
void add_work(safequeue_t *q, char* request, int priority);
char* get_work_blocking(safequeue_t *q);
char* get_work_nonblocking(safequeue_t *q);
int safequeue_is_empty(safequeue_t *q);
int safequeue_size(safequeue_t *q);

#endif // SAFEQUEUE_H
