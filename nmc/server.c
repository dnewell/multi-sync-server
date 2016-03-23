#include "as3.h"

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
//static pthread_barrier_t barrier;

void* start(void* arg)
{
    struct argument* args = (struct argument*)arg;
    struct connection_queue* queue = args->queue;
    sem_t* semaphore = args->semaphore;
    pthread_t tid = pthread_self();
    while(1)
    {
        int connfd, numBytes, num_rec, num_send;
        int lock;
        char to_rec[INT_LEN], to_send[INT_LEN];

        //Semaphore is used to signal when the main thread adds a connection to the queue
        //Threads are blocked until released
        sem_wait(semaphore);
      
        //Mutex is used to ensure that the queue is not manipulated in an unexpected way
        //For example a queue and dequeue happening around the same time
        pthread_mutex_lock(&mtx);
       
        //This should never really happen since semaphore should assure that item is in queue
        if (queue_empty(queue))
        {
            pthread_mutex_unlock(&mtx);
            continue;
        }
        connfd = dequeue(queue);

        pthread_mutex_unlock(&mtx);
        
        //pthread_barrier_wait(&barrier);

        printf("Thread ID: %u, Handling connfd: %d\n", (unsigned int)tid, connfd);

        memset(to_rec, '0', sizeof(to_rec));
        memset(to_send, '0', sizeof(to_send));
        numBytes = recv(connfd, &to_rec, sizeof(to_rec)-1, 0);
        
        if (numBytes == -1)
        {
            perror("read");
            exit(EXIT_FAILURE);
        }
        
        to_rec[numBytes] = '\0';

        if (sscanf(to_rec, "%d", &num_rec) == EOF)
        {
            perror("sscanf");
        }

        num_send = num_rec * 10;
        
        sprintf(to_send, "%d", num_send);

        numBytes = send(connfd, &to_send, sizeof(to_send), 0);
        
        if (numBytes == -1)
        {
            perror("write");
            exit(EXIT_FAILURE);
        }
        close(connfd);
    }
}


int main(int argc, char* argv[])
{
    if (argc != 4)
    {
        printf("\nUsage: %s port num_threads array_size\n", argv[0]);
        return 1;
    }
 
    sem_t semaphore;
    pthread_attr_t attr;
    int num_con = atoi(argv[3]);
    int num_threads = atoi(argv[2]);
    pthread_t threads[num_threads];
    int i, listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr;
    struct argument args;

    //setup the connection queue
    struct connection_queue queue;
    queue_set_size(&queue, num_con);

    if (pthread_attr_init(&attr)) 
    {
        perror("thread init");
        exit(EXIT_FAILURE);
    }

    if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED))
    {
        perror("thread set detach");
        exit(EXIT_FAILURE);
    }

    sem_init(&semaphore, 0, 0);
    args.queue = &queue;
    args.semaphore = &semaphore;
    //Initialize thread pool
    for (i = 0; i < num_threads; i++)
    {
        if(pthread_create(&threads[i], &attr, start, &args))
        {
            perror("thread create");
            exit(EXIT_FAILURE);
        }
    }

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&serv_addr, '0', sizeof(serv_addr));
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if (bind(listenfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(listenfd, num_con) == -1)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    //pthread_barrier_init(&barrier, NULL, 5);
    while(1)
    {
        connfd = accept(listenfd, NULL, NULL);
        pthread_mutex_lock(&mtx);
        enqueue(&queue, connfd);
        pthread_mutex_unlock(&mtx);
        //Semaphore is used to signal that a connection has been added to the queue
        sem_post(&semaphore);
    }
    exit(EXIT_SUCCESS);
}
