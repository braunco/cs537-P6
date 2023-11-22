#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "proxyserver.h"

#include "safequeue.h"

safequeue_t request_queue;



/*
 * Constants
 */
#define RESPONSE_BUFSIZE 10000
#define MAX_LISTENERS 65112

/*
 * Global configuration variables.
 * Their values are set up in main() using the
 * command line arguments (already implemented for you).
 */
int num_listener;
int *listener_ports;
int num_workers;
char *fileserver_ipaddr;
int fileserver_port;
int max_queue_size;


void serve_request(int client_fd, struct http_request *http_request);
void free_http_request(struct http_request *req);


void *worker_thread_function(void *arg) {
    while (1) {
        
        request_info_t *req_info = get_work_blocking(&request_queue);

        if (req_info != NULL) {
            int delay = req_info->request->delay ? atoi(req_info->request->delay) : 0;
            printf("delay: %i\n", delay);
            if (delay > 0) {
                sleep(delay);
            }

            printf("\t\tabout to serve request\n");
            serve_request(req_info->client_fd, req_info->request);

            //free_http_request(req_info->request);
            //////////free(req_info);
        }
    }
    
    return NULL;
}



int get_request_priority(const char *path) {
    // Extract the directory name from the path and convert it to an integer
    int priority = 0;
    sscanf(path, "/%d/", &priority);
    return priority;
}


void send_error_response(int client_fd, status_code_t err_code, char *err_msg) {
    printf("Status code: %d\n", err_code);
    printf("Entering here\n");
    http_start_response(client_fd, err_code);
    http_send_header(client_fd, "Content-Type", "text/html");
    http_end_headers(client_fd);
    char *buf = malloc(strlen(err_msg) + 2);
    sprintf(buf, "%s\n", err_msg); // Added a line to see where print
    http_send_string(client_fd, buf);
    /////////free(buf);
    return;
}

/*
 * forward the client request to the fileserver and
 * forward the fileserver response to the client
 */
void serve_request(int client_fd, struct http_request *http_request) {
    //printf("Entered serve_request\n");

    // create a fileserver socket
    int fileserver_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (fileserver_fd == -1) {
        fprintf(stderr, "Failed to create a new socket: error %d: %s\n", errno, strerror(errno));
        exit(errno);
    }
    //printf("Did fileserver socket\n");

    // create the full fileserver address
    struct sockaddr_in fileserver_address;
    fileserver_address.sin_addr.s_addr = inet_addr(fileserver_ipaddr);
    fileserver_address.sin_family = AF_INET;
    fileserver_address.sin_port = htons(fileserver_port);
    //printf("Did fileserver address\n");

    // connect to the fileserver
    int connection_status = connect(fileserver_fd, (struct sockaddr *)&fileserver_address, sizeof(fileserver_address));
    //printf("Connected to fileserver\n");

    if (connection_status < 0) {
        // failed to connect to the fileserver
        printf("Failed to connect to the file server\n");
        send_error_response(client_fd, BAD_GATEWAY, "Bad Gateway");
        return;
    }

    // successfully connected to the file server
    //char *buffer = (char *)malloc(RESPONSE_BUFSIZE * sizeof(char));
    //printf("Did fileserver socket\n");

    char request_buffer[RESPONSE_BUFSIZE];
    sprintf(request_buffer, "%s %s HTTP/1.0\r\n\r\n", http_request->method, http_request->path);
    http_send_data(fileserver_fd, request_buffer, strlen(request_buffer));

    char response_buffer[RESPONSE_BUFSIZE];
    int bytes_read;
    while ((bytes_read = recv(fileserver_fd, response_buffer, RESPONSE_BUFSIZE - 1, 0)) > 0) {
        if(http_send_data(client_fd, response_buffer, bytes_read) < 0) {
            perror("COULDNT SEND DATA TO CLIENT");
        }
    }

    //printf("\tRESPONSE: %s\n", response_buffer);

    // Close the connection to the fileserver
    shutdown(fileserver_fd, SHUT_WR);
    close(fileserver_fd);
    
    //close connection to client
    shutdown(client_fd, SHUT_WR);
    close(client_fd);

    /*
    // forward the client request to the fileserver
    int bytes_read = read(client_fd, buffer, RESPONSE_BUFSIZE);
    printf("Passed bytes_read\n");
    int ret = http_send_data(fileserver_fd, buffer, bytes_read);
    printf("Passed ret\n");
    if (ret < 0) {
        printf("Failed to send request to the file server\n");
        send_error_response(client_fd, BAD_GATEWAY, "Bad Gateway");

    } else {
        printf("Entered deep else statement\n");
        // forward the fileserver response to the client
        while (1) {
            int bytes_read = recv(fileserver_fd, buffer, RESPONSE_BUFSIZE - 1, 0);
            if (bytes_read <= 0) // fileserver_fd has been closed, break
                break;
            ret = http_send_data(client_fd, buffer, bytes_read);
            if (ret < 0) { // write failed, client_fd has been closed
                break;
            }
        }
    }

    // close the connection to the fileserver
    shutdown(fileserver_fd, SHUT_WR);
    close(fileserver_fd);

    // Free resources and exit
    free(buffer);
    */
}

void free_http_request(struct http_request *req) {
    if (req) {
        // Free individual fields of req
        free(req->method);
        free(req->path);
        free(req->delay);
        free(req);
    }
}


int server_fds[MAX_LISTENERS]; // 65535 - 1024 + 1 (for inclusivity)
/*
 * opens a TCP stream socket on all interfaces with port number PORTNO. Saves
 * the fd number of the server socket in *socket_number. For each accepted
 * connection, calls request_handler with the accepted fd number.
 */

void handle_getjob_request(int client_fd) {
    //pthread_mutex_lock(&request_queue.lock);
    if (safequeue_is_empty(&request_queue)) {
        //pthread_mutex_unlock(&request_queue.lock);
        //http_start_response(client_fd, QUEUE_EMPTY);
        send_error_response(client_fd, QUEUE_EMPTY, "Queue is empty");
        //printf("reached this point\n");
        return;
    } else {
        //pthread_mutex_lock(&request_queue.lock);
        request_info_t *dequeued_request = get_work_blocking(&request_queue);
        //pthread_mutex_unlock(&request_queue.lock);
        http_start_response(client_fd, OK);
        http_send_header(client_fd, "Content-Type", "text/plain");
        http_end_headers(client_fd);
        http_send_string(client_fd, dequeued_request->request->path);
        /////////free_http_request(dequeued_request->request);
        /////////free(dequeued_request);
        //printf("Fail 7\n");
    }
}


void handle_normal_request(int client_fd, struct http_request *http_request) {
    //printf("Entered handle_normal_request\n");
    request_info_t *req_info = malloc(sizeof(request_info_t));
    if (req_info == NULL) {
        //printf("Entered first h_n_r if statement\n");
        send_error_response(client_fd, SERVER_ERROR, "Server Error");
        return;
    }

    req_info->request = http_request;
    req_info->client_fd = client_fd;
    int priority = get_request_priority(http_request->path);

    //int current_queue_size = safequeue_size(&request_queue);
    
    if(add_work(&request_queue, req_info, priority) < 0) {
        printf("priority queue full. path: %s\n", req_info->request->path);
        send_error_response(client_fd, QUEUE_FULL, "Priority queue is full.");
        printf("\tGOT PAST SEND ERROR RESP\n");
        //free_http_request(http_request);
        ////////////free(req_info);
    }

    // if (current_queue_size < max_queue_size) {
    //     //printf("Entered second h_n_r if statement\n");
    //     add_work(&request_queue, req_info, priority);
    // } else {
    //     //printf("Entered first h_n_r else statement\n");
    //     send_error_response(client_fd, QUEUE_FULL, "Priority queue is full");
    //     free_http_request(http_request);
    //     free(req_info);
    // }
    
    
    
}


void *serve_forever(void *arg) {
    int proxy_port = *((int *)arg);
    free(arg);                      // Free allocated memory

    // Create a socket to listen
    int local_server_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (local_server_fd == -1) {
        perror("Failed to create a new socket");
        exit(errno);
    }

    // manipulate options for the socket
    int socket_option = 1;
    if (setsockopt(local_server_fd, SOL_SOCKET, SO_REUSEADDR, &socket_option,
                   sizeof(socket_option)) == -1) {
        perror("Failed to set socket options");
        exit(errno);
    }

    struct sockaddr_in proxy_address;
    memset(&proxy_address, 0, sizeof(proxy_address));
    proxy_address.sin_family = AF_INET;
    proxy_address.sin_addr.s_addr = INADDR_ANY;
    proxy_address.sin_port = htons(proxy_port);

    
    if (bind(local_server_fd, (struct sockaddr *)&proxy_address, sizeof(proxy_address)) == -1) {
        perror("Failed to bind on socket");
        exit(errno);
    }
    
    if (listen(local_server_fd, 1024) == -1) {
        perror("Failed to listen on socket");
        exit(errno);
    }
    if (local_server_fd < 0) exit(EXIT_FAILURE); // Exit if socket setup fails
    
    printf("Listening on port %d...\n", proxy_port);

    
    struct sockaddr_in client_address;
    size_t client_address_length = sizeof(client_address);
    int client_fd;

    while (1) {
        client_fd = accept(local_server_fd, (struct sockaddr *)&client_address, (socklen_t *)&client_address_length);
        if (client_fd < 0) {
            perror("Error accepting socket");
            continue;
        }

        printf("Accepted connection from %s on port %d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

        
        struct http_request *http_request = http_request_parse(client_fd); // This line fucks with the serve_request b/c it reads from the buffer
        if (http_request != NULL) {
            if (strcmp(http_request->path, GETJOBCMD) == 0) {
                //printf("Entered second if statement\n");
                handle_getjob_request(client_fd);
                //serve_request(client_fd, http_request);
                //printf("reaches here\n");
            } else {
                //printf("Entered first else statement\n");
                handle_normal_request(client_fd, http_request);
                //serve_request(client_fd, http_request);
            }
        } else {
            printf("Entered second else statement\n");
            send_error_response(client_fd, BAD_REQUEST, "Bad Request");
        }
        

        //shutdown(client_fd, SHUT_WR);
        //close(client_fd);   
    }


    close(local_server_fd);
    return NULL;
}



/*
 * Default settings for in the global configuration variables
 */
void default_settings() {
    num_listener = 1;
    listener_ports = (int *)malloc(num_listener * sizeof(int));
    listener_ports[0] = 8000;

    num_workers = 1;

    fileserver_ipaddr = "127.0.0.1";
    fileserver_port = 3333;

    max_queue_size = 100;
}

void print_settings() {
    printf("\t---- Setting ----\n");
    printf("\t%d listeners [", num_listener);
    for (int i = 0; i < num_listener; i++)
        printf(" %d", listener_ports[i]);
    printf(" ]\n");
    printf("\t%d workers\n", num_workers);
    printf("\tfileserver ipaddr %s port %d\n", fileserver_ipaddr, fileserver_port);
    printf("\tmax queue size  %d\n", max_queue_size);
    printf("\t  ----\t----\t\n");
}

void signal_callback_handler(int signum) {
    printf("Caught signal %d: %s\n", signum, strsignal(signum));
    for (int i = 0; i < num_listener; i++) {
        if(server_fds[i] != 0) {
            if (close(server_fds[i]) < 0) perror("Failed to close server_fd (ignoring)\n");
        }
    }
    free(listener_ports);

    destroy_queue(&request_queue);

    exit(0);
}

char *USAGE =
    "Usage: ./proxyserver [-l 1 8000] [-n 1] [-i 127.0.0.1 -p 3333] [-q 100]\n";

void exit_with_usage() {
    fprintf(stderr, "%s", USAGE);
    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
    signal(SIGINT, signal_callback_handler);
    

    // Default settings
    default_settings();

    // Parsing command-line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp("-l", argv[i]) == 0) {
            num_listener = atoi(argv[++i]);
            free(listener_ports);
            listener_ports = (int *)malloc(num_listener * sizeof(int));
            for (int j = 0; j < num_listener; j++) {
                listener_ports[j] = atoi(argv[++i]);
            }
        } else if (strcmp("-w", argv[i]) == 0) {
            num_workers = atoi(argv[++i]);
        } else if (strcmp("-q", argv[i]) == 0) {
            max_queue_size = atoi(argv[++i]);
        } else if (strcmp("-i", argv[i]) == 0) {
            fileserver_ipaddr = argv[++i];
        } else if (strcmp("-p", argv[i]) == 0) {
            fileserver_port = atoi(argv[++i]);
        } else {
            fprintf(stderr, "Unrecognized option: %s\n", argv[i]);
            exit_with_usage();
        }
    }

    create_queue(&request_queue, max_queue_size); // Initialize the safequeue

    print_settings();

    // Create and start listening threads
    // Create and start listening threads
    pthread_t *threads = malloc(num_listener * sizeof(pthread_t));
    for (int i = 0; i < num_listener; ++i) {
        int *port = malloc(sizeof(int));  // Allocate memory to pass the port number
        *port = listener_ports[i];
        if (pthread_create(&threads[i], NULL, serve_forever, port) != 0) {
            perror("Failed to create thread");
            // Handle thread creation failure
        }
    }


    
    // Create worker threads
    pthread_t worker_threads[num_workers];
    for (int i = 0; i < num_workers; ++i) {
        if (pthread_create(&worker_threads[i], NULL, worker_thread_function, NULL) != 0) {
            perror("Failed to create worker thread");
            // Handle error
        }
    }
    
    //one for now
    // pthread_t worker_thread;
    // if(pthread_create(&worker_thread, NULL, worker_thread_function, NULL) < 0) {
    //     perror("failed to create worker thread");
    // }

    // Join threads
    for (int i = 0; i < num_listener; ++i) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("Failed to join thread");
            // Handle thread join failure
        }
    }

    
    // Join worker threads
    for (int i = 0; i < num_workers; ++i) {
        pthread_join(worker_threads[i], NULL);
    }

    //pthread_join(worker_thread, NULL);

    destroy_queue(&request_queue);
    
    free(threads); // Free the threads array

    return EXIT_SUCCESS;
}