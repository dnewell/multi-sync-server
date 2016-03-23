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

/*
 * Server Constants
 */
#define MAX_MSG_SIZE 1024

/*----------  Prototypes  ----------*/
void  enqueue(int);
int   dequeue();
void  enumerateQueue();
void  freeMem();

/*-----------  Globals  ------------*/
int rear = 0, front = 0, queueElementCount = 0, MAX_QUEUE_SIZE = 0;

// this array will hold all connection info: descriptor and ... TODO?
int* queue = NULL;


/**
 * This function receives receives a number from the client and sends the product back
 * @param conDesc a connection descriptor
 */
void doProcessing(int conDesc)
{
   int numbytes;
   char recvBuff[MAX_MSG_SIZE], sendBuff[MAX_MSG_SIZE];
   int numFromClient, result;
   memset(recvBuff, '0', sizeof(recvBuff));
   memset(sendBuff, '0', sizeof(sendBuff));
   /*receive data from the client*/
   numbytes = recv(conDesc, recvBuff, sizeof(recvBuff) - 1, 0);
   if (numbytes == -1)
   {
      perror("recv");
      exit(1);
   }
   recvBuff[numbytes] = '\0';
   /*Extract the number to be multiplied by 10*/
   sscanf(recvBuff, "%d ", &numFromClient);
   result = numFromClient * 10;
   snprintf(sendBuff, sizeof(sendBuff), "%d", result);
   send(conDesc, sendBuff, strlen(sendBuff), 0);
   close(conDesc);
}

/**
 * Dynamically allocates memory for the queue
 * @param sizeOfQueue desired size for the queue
 */
void allocQueue(int sizeOfQueue)
{
   // calloc to allocate and initialize to zero
   queue = calloc(sizeOfQueue, sizeof(int));
   if (!queue)     //check for null/error
   {
      perror("Error (c)allocating queue, and we can't just go on as if nothing happened.\n");
      freeMem();
      exit(1);
   }
}

/**
 * Frees memory used by the queue
 */
void freeMem()
{
   free(queue);
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
      perror("Error: Queue is full, and you still tried to add to it. I *hate* that. Bye.\n");
      freeMem();
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
      perror("Error: Queue is empty. Cannot dequeue. You shouldn't have tried! Ciao.");
      freeMem();
      exit(1);
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
   int listenfd = 0, conDesc = 0;
   int port, numThreads;
   struct sockaddr_in serv_addr;
   int rv;
   /*Command line argument: port number*/
   if (argc != 4)
   {
      printf("\nUsage: %s port number_threads queue_size \n", argv[0]);
      return 1;
   }
   else
   {
      printf("°º¤ø,¸¸,ø¤º°`°º¤ø,¸,ø¤°º¤ø,¸¸,ø¤º°`°º¤ø,¸\n");
      printf("  MultiServer started with %s threads.  \n    Queue size = %s\n    Listening on port: %s\n", argv[2], argv[3], argv[1]);
   }
   /*Command line processing*/
   port       = atoi(argv[1]);
   numThreads = atoi(argv[2]);
   MAX_QUEUE_SIZE  = atoi(argv[3]);
   //printf("MAX_QUEUE_SIZE: %d", MAX_QUEUE_SIZE);
   /*Allocate queue*/
   allocQueue(MAX_QUEUE_SIZE);

   // /*Socket creation and binding*/
   // listenfd = socket(AF_INET, SOCK_STREAM, 0);
   // if (listenfd <  0)
   // {
   //    perror("Error in socket creation");
   //    freeMem();
   //    exit(1);
   // }
   // memset(&serv_addr, '0', sizeof(serv_addr));
   // serv_addr.sin_family      = AF_INET;
   // serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
   // serv_addr.sin_port        = htons(port);
   // rv = bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
   // if (listenfd <  0)
   // {
   //    perror("Error in binding");
   //    freeMem();
   //    exit(1);
   // }
   // listen(listenfd, 10);
   // /*Accept connection and call the doProcessing function */
   // while (1)
   // {
   //    conDesc = accept(listenfd, (struct sockaddr*)NULL, NULL);
   //    doProcessing(conDesc);
   //    //sleep(1);
   // }
   free(queue); // TODO check logic (exit conditions) for possible memory leaks
}
