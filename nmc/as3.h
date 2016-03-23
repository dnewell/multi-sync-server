#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <semaphore.h>

#include "queue2.c"
#define INT_LEN 1024

struct argument {
    struct connection_queue* queue;
    sem_t* semaphore;
};
