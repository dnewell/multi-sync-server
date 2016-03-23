#include "as3.h"

int main (int argc, char* argv[])
{
    char to_send[INT_LEN], to_rec[INT_LEN];
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int connfd, numBytes, num_result;

    if (argc != 4)
    {
        printf("\nUsage: %s host_name port_number num\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    memset(&hints, 0, sizeof(struct addrinfo));
    
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if(getaddrinfo(argv[1], argv[2], &hints, &result) != 0)
    {
        perror("gettaddr");
        exit(EXIT_FAILURE);
    }

    //Go through returned list to find address that can be used.
    
    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
        connfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        //If error then continue with the next address
        if (connfd == -1)
            continue;

        if (connect(connfd, rp->ai_addr, rp->ai_addrlen) != -1)
            break;

        close(connfd);
    }

    if (rp == NULL)
    {
        perror("connection");
        exit(EXIT_FAILURE);
    }
    
    freeaddrinfo(result);

    memset(to_send, '0', sizeof(to_send));
    memset(to_rec, '0', sizeof(to_rec));

    sprintf(to_send, "%d", atoi(argv[3]));

    numBytes = send(connfd, to_send, sizeof(to_send), 0);

    if(numBytes == -1)
    {
        perror("send");
        exit(EXIT_FAILURE);
    }
   
    numBytes = recv(connfd, to_rec, sizeof(to_rec)-1, 0);
    if (numBytes == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }

    to_rec[numBytes] = '\0';
    sscanf(to_rec, "%d", &num_result);
    printf("Sent: \t%s\tReceived result is: \t%d\n", to_send, num_result);
    close(connfd);
    exit(EXIT_SUCCESS);
}
