/*===============================================================
=            Server.c - a simple multithreaded echo-esque server,
=              which uses mutexes and semaphores to implement a
=              blocking work queue with no busy-wait spinlocking
= 
=            NOTE: Some code was adapted from the examples given on course site
=
=             By : David Newell
=             For: CS 3305, Winter 2016, Assignment 3
=
===============================================================*/
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>

/*----------  Server Constants  ----------*/
#define MAX_MSG_SIZE 1024

/*----------  Prototypes  ----------*/
void  enqueue(int);
int   dequeue();
void* doProcessing(void*);
/*-----------  Globals  ------------*/
int rear = 0, front = 0, queueElementCount = 0, MAX_QUEUE_SIZE = 0;

/*----------  this array will hold the connection descriptors  ----------*/
int* queue = NULL;

/*----------  This semaphore is used to wake threads when there is a   ----------*/
/*----------  connection in the queue, avoiding 'busywaiting' spinlock ----------*/
sem_t hasConnection;

/*----------  Mutex used to implement blocking queue with binary lock  ----------*/
pthread_mutex_t queueLock = PTHREAD_MUTEX_INITIALIZER;

/**
 * This thread work function gets a connection from the blocking queue,
 * and then receives an integer from the client and sends the product back.
 *
 * This function used a semaphore to wait for the queue to have a connection,
 * thereby avoiding a busy-waiting spinlock.  
 * 
 * It uses pthread Mutexes to handle syncronization, placing a binary lock on the queue.
 *
 * @param currentConDesc a connection descriptor
 */
void * doProcessing(void *arg)
{
   pthread_t threadId = pthread_self();
   int numbytes;
   char recvBuff[MAX_MSG_SIZE], sendBuff[MAX_MSG_SIZE];
   int numFromClient, result;
   int currentConDesc;

   /*----------  Semaphore wait method is non-busy-waiting spinlock  ----------*/
   while (1)
   {
      /*----------  Semaphore: wait until queue had a connection.  ----------*/
      sem_wait(&hasConnection);

      /*----------  Mutex: queue locked during access ----------*/
      pthread_mutex_lock(&queueLock);
      currentConDesc = dequeue();
      pthread_mutex_unlock(&queueLock);

      /*----------  Initialize buffers  ----------*/  
      memset(recvBuff, '0', sizeof(recvBuff));
      memset(sendBuff, '0', sizeof(sendBuff));

      /*----------  receive data from the client  ----------*/
         numbytes = recv(currentConDesc, recvBuff, sizeof(recvBuff) - 1, 0);
      if (numbytes == -1)
      {
         perror("Error: nothing recieved");
         exit(1);
      }
      recvBuff[numbytes] = '\0';

      /*----------  Extract the number to be multiplied by 10 from packet  ----------*/ 
      sscanf(recvBuff, "%d ", &numFromClient);
      result = numFromClient * 10;

      /*----------  Send result to client  ----------*/
            snprintf(sendBuff, sizeof(sendBuff), "%d", result);
      if (send(currentConDesc, sendBuff, strlen(sendBuff), 0) == -1){
         perror("Error: send failed.");
         exit(1);         
      } else {
         printf("server: thread tid %lu, computed and sent result.\n", threadId);
         close(currentConDesc);
      }      
   }
}

/**
 * Dynamically allocates memory for the queue
 * @param sizeOfQueue size for the queue
 */
void allocQueue(int sizeOfQueue)
{
   // calloc to allocate and initialize to zero
   queue = calloc(sizeOfQueue, sizeof(int));
   if (!queue)     //check for null/error
   {
      perror("Error (c)allocating queue, and we can't just go on as if nothing happened.\n");
      free(queue);
      exit(1);
   }
}

/**
 * Adds element to rear of queue
 * @param elementToAdd element to add
 */
void enqueue(int elementToAdd)
{ 
   int indexForNewElement;
   if (queueElementCount >= MAX_QUEUE_SIZE)
   {
      perror("Error: Queue is full, yet you still tried to add an element.\n");
      free(queue);
      exit(1);
   }
   // calculate derived index of rear/tail based on modulo of the size.
   // makes array circular
   indexForNewElement = (front + queueElementCount) % MAX_QUEUE_SIZE;
   // insert element
   queue[indexForNewElement] = elementToAdd;
   queueElementCount++;
}

/**
 * Return element from head/front of queue
 * @return head element
 */
int dequeue()
{
   int headElement;  // element to return
   if (queueElementCount <= 0)
   {
      perror("Error: Queue is empty. Cannot dequeue.");
      return -1;
   }
   headElement = queue[front];
   // adjust front queue pointer
   front++;
   front %= MAX_QUEUE_SIZE;
   queueElementCount--;
   return headElement;
}

int main(int argc, char* argv[])
{
   int listenfd = 0, connectionDescriptor = 0, i = 0;
   int port, numThreads;
   struct sockaddr_in serv_addr;
   int rv, err;
   
   /*Command line argument: port number*/
   if (argc != 4)
   {
      printf("\nUsage: %s port number_threads queue_size \n", argv[0]);
      return 1;
   }
   else
   {
      printf("°º¤ø,¸¸,ø¤º°`°º¤ø,¸,ø¤°º¤ø,¸¸,ø¤º°`°º¤ø,¸\n");
      printf("  server.c - started with %s threads.  \n    Queue size = %s\n    Listening on port: %s\n", argv[2], argv[3], argv[1]);
   }
   /*Command line processing*/
   port       = atoi(argv[1]);
   numThreads = atoi(argv[2]);
   MAX_QUEUE_SIZE  = atoi(argv[3]);

   pthread_t tidArray[numThreads];

   /*Allocate queue*/
   allocQueue(MAX_QUEUE_SIZE);

   /*Initialize semaphore, first 0 arg indicates shared between the
       threads, second 0 is the semaphore's value*/
   sem_init(&hasConnection, 0, 0);

   /*Create threads*/
   while (i < numThreads){

      err = pthread_create(&tidArray[i], NULL, doProcessing, NULL);

      if (err != 0){
         perror("Error: thread creation failed.");
         exit(1);
      }
      i++;
   }

   /*Socket creation and binding - adapted from 3305 sample code*/
   listenfd = socket(AF_INET, SOCK_STREAM, 0);
   if (listenfd <  0)
   {
      perror("Error: Socket creation failed.");
      free(queue);
      exit(1);
   }
   memset(&serv_addr, '0', sizeof(serv_addr));

   serv_addr.sin_family      = AF_INET;
   serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
   serv_addr.sin_port        = htons(port);
   rv = bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

   if (listenfd <  0)
   {
      perror("Error: Binding failed.");
      free(queue);
      exit(1);
   }

   listen(listenfd, 30);


   /*Accept connection and adds it to the locked queue*/
   while (1)
   {
      connectionDescriptor = accept(listenfd, (struct sockaddr*)NULL, NULL);

      /*----------  Mutex: queue locked during access ----------*/
      pthread_mutex_lock(&queueLock);
      enqueue(connectionDescriptor);
      pthread_mutex_unlock(&queueLock);

      sem_post(&hasConnection);
   }

   free(queue);
   exit(0);
}
