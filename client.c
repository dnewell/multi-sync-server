#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

#define MAX_MSG_SIZE 1024

/*
 * The function get_sockaddr converts the server's address and port in anticipation
 * of creating a socket.  Sets up the structs using netdb.h method getaddrinfo,
 * which also performs the necessary DNS and service name lookups.
*/
struct addrinfo* get_sockaddr(const char* hostname, const char* port)
{
   struct addrinfo hints;
   struct addrinfo* results;
   int rv;
   memset(&hints, 0, sizeof(struct addrinfo));
   hints.ai_family   = AF_INET;          //specifies IPv4 address
   hints.ai_socktype = SOCK_STREAM;    //specifies TCP stream sockets
   //Sets up the structs using netdb.h method getaddrinfo and performs DNS and service name lookups
   rv = getaddrinfo(hostname, port, &hints, &results);
   if (rv != 0)
   {
      fprintf(stderr, "error: getaddrinfo: %s\n", gai_strerror(rv));
      freeaddrinfo(results);  // frees the linked list before exit(error)
      exit(1);
   }
   return results;
}

/*
 * The function open_connection establishes a connection to the server
*/
int open_connection(struct addrinfo* addr_list)
{
   struct addrinfo* p;
   int sockfd;
   //Iterate through each addr info in the list; Stop when we successully connect to one
   for (p = addr_list; p != NULL; p = p->ai_next)
   {
      sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
      // Try the next address since the socket operation failed
      if (sockfd == -1) { continue; }
      //Stop iterating of we are able to connect to the server
      if (connect(sockfd, p->ai_addr, p->ai_addrlen) != -1) { break; }
   }
   freeaddrinfo(addr_list);
   if (p == NULL)
   {
      printf("Unable to connect");
      exit(1);
   }
   return sockfd;
}

int main(int argc, char* argv[])
{
   char sendBuff[MAX_MSG_SIZE];
   char recvBuff[MAX_MSG_SIZE];
   struct sockaddr_in serv_addr;
   struct hostent* server;
   int rv, sum, product;

   if (argc != 4)
   {
      printf("\n Usage: %s hostname port muliplicand \n", argv[0]);
      return 1;
   }

   struct addrinfo* results = get_sockaddr(argv[1], argv[2]);  // calls our struct/hostname lookup function
   int socketDescriptor = open_connection(results);

   memset(sendBuff, '0', sizeof(sendBuff));
   memset(recvBuff, '0', sizeof(recvBuff));

   int muliplicand = atoi(argv[3]);

   snprintf(sendBuff, sizeof(sendBuff), "%d", muliplicand);

   write(socketDescriptor, sendBuff, strlen(sendBuff));

   int numbytes = recv(socketDescriptor, recvBuff, sizeof(recvBuff) - 1, 0);

   if (numbytes == -1)
   {
      perror("recv");
      exit(1);
   }

   recvBuff[numbytes] = '\0';

   sscanf(recvBuff, "%d", &product);
   printf("received product is %d\n", product);
   return 0;
}
