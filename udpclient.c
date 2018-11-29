/* udp_client.c */
/* Programmed by Adarsh Sethi */
/* Sept. 13, 2018 */

#include <stdio.h>          /* for standard I/O functions */
#include <stdlib.h>         /* for exit */
#include <string.h>         /* for memset, memcpy, and strlen */
#include <netdb.h>          /* for struct hostent and gethostbyname */
#include <sys/socket.h>     /* for socket, sendto, and recvfrom */
#include <netinet/in.h>     /* for sockaddr_in */
#include <unistd.h>         /* for close */
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <math.h>

#define STRING_SIZE 1024
int timeoutLen;
double plr;
double alr;
int expectedACK = 0;
int receivedACK;

typedef struct dataPacket{
  int dataCount;
  int sequenceNumber;
  char data[80];
} dataPacket;

int main(void) {

    srand(time(NULL));
    int sequenceNumber= 0;
    char inputTimeoutLen[STRING_SIZE];
    char inputPacketLossRate[STRING_SIZE];
    char inputACKLossRate[STRING_SIZE];
   int sock_client;  /* Socket used by client */
   struct sockaddr_in client_addr;  /* Internet address structure that
                                        stores client address */
   unsigned short client_port;  /* Port number used by client (local port) */

   struct sockaddr_in server_addr;  /* Internet address structure that
                                        stores server address */
   struct hostent * server_hp;      /* Structure to store server's IP
                                        address */
   char server_hostname[STRING_SIZE]; /* Server's hostname */
   unsigned short server_port;  /* Port number used by server (remote port) */

   char sentence[STRING_SIZE];  /* send message */
   char modifiedSentence[STRING_SIZE]; /* receive message */
   char ack[2]; //ack to be received from the server
   unsigned int msg_len;  /* length of message */
   int bytes_sent, bytes_recd; /* number of bytes sent or received */
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
   /* open a socket */

   if ((sock_client = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
      perror("Client: can't open datagram socket\n");
      exit(1);
   }


    fp = fopen("sample.txt", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

   /* Note: there is no need to initialize local client address information
            unless you want to specify a specific local port.
            The local address initialization and binding is done automatically
            when the sendto function is called later, if the socket has not
            already been bound.
            The code below illustrates how to initialize and bind to a
            specific local port, if that is desired. */

   /* initialize client address information */

   client_port = 0;   /* This allows choice of any available local port */

   /* Uncomment the lines below if you want to specify a particular
             local port: */
   /*
   printf("Enter port number for client: ");
   scanf("%hu", &client_port);
   */

   /* clear client address structure and initialize with client address */
   memset(&client_addr, 0, sizeof(client_addr));
   client_addr.sin_family = AF_INET;
   client_addr.sin_addr.s_addr = htonl(INADDR_ANY); /* This allows choice of
                                        any host interface, if more than one
                                        are present */
   client_addr.sin_port = htons(client_port);

   /* bind the socket to the local client port */

   if (bind(sock_client, (struct sockaddr *) &client_addr,
                                    sizeof (client_addr)) < 0) {
      perror("Client: can't bind to local address\n");
      close(sock_client);
      exit(1);
   }


   /* end of local address initialization and binding */

   /* initialize server address information */

   printf("Enter hostname of server: ");
   scanf("%s", server_hostname);
   if ((server_hp = gethostbyname(server_hostname)) == NULL) {
      perror("Client: invalid server hostname\n");
      close(sock_client);
      exit(1);
   }
   printf("Enter port number for server: ");
   scanf("%hu", &server_port);

   /* Clear server address structure and initialize with server address */
   memset(&server_addr, 0, sizeof(server_addr));
   server_addr.sin_family = AF_INET;
   memcpy((char *)&server_addr.sin_addr, server_hp->h_addr,
                                    server_hp->h_length);
   server_addr.sin_port = htons(server_port);

   /* user interface */
    printf("Please input a Timeout Quantity:\n");
    scanf("%s", inputTimeoutLen);
    sscanf(inputTimeoutLen, "%d", &timeoutLen);
    char            name[20] = {0}; // in case of single character input
    struct timeval timeout;
    if (timeoutLen>=6){
        timeoutLen = timeoutLen%6;
        timeout.tv_sec = pow(10,timeoutLen);
        timeout.tv_usec = 0;
    }
    else{
        timeout.tv_sec = 0;
        timeout.tv_usec = pow(10,timeoutLen);
    }
    char message[100];
    setsockopt (sock_client, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
                sizeof(timeout));
    int totalData = 0;
    int totalPackets = 0;
    int totalTransmissions = 0;
    int totalRetransmissions = 0;
    int totalACKs = 0;
    int totalTimeouts = 0;
    while ((read = getline(&line, &len, fp)) != -1){
        int retransmissions = 0;
        int flag = 0;
        memset(message, 0, 100 );
        int convertdata = read-1;
        char datastr[2];
        char buffer[100], *ptr;
        struct dataPacket temp = {.dataCount = convertdata, .sequenceNumber = sequenceNumber, .data = * line};
        memcpy(temp.data,line,strlen(line));
        printf("data is: %s\n", temp.data);

        if (sequenceNumber==0)
            sequenceNumber = 1;
        else
            sequenceNumber = 0;
        while (flag == 0){
            if (retransmissions==0){
                totalPackets+=1;
                totalData+=convertdata;
                printf("Packet %d transmitted with %d data bytes\n",temp.sequenceNumber, temp.dataCount);
            }
            else{
                printf("Packet %d retransmitted with %d data bytes\n",temp.sequenceNumber, temp.dataCount);
            }
            totalTransmissions+=1;
            retransmissions+=1;
            bytes_sent = sendto(sock_client, &temp, sizeof(temp), 0,
                (struct sockaddr *) &server_addr, sizeof (server_addr));
            if ((recvfrom(sock_client, ack, STRING_SIZE, 0,
                (struct sockaddr *) 0, 0))<0){
                printf("\nTimeout expired for packet numbered %d\n", temp.sequenceNumber);
                totalTimeouts+=1;
            }
            else{
                flag = 1;
                receivedACK = atoi(ack);
                if (expectedACK == receivedACK && receivedACK == 1){
                  expectedACK = 0;
                  printf("Received ACK has sequence number: %d\n", receivedACK);
                  printf("Next ACK should be: %d\n", expectedACK);
                }
                else if (expectedACK == receivedACK && receivedACK == 0){
                  expectedACK = 1;
                  printf("Received ACK has sequence number: %d\n", receivedACK);
                  printf("Next ACK should be: %d\n", expectedACK);
                }
                else if (expectedACK != receivedACK && receivedACK == 0){
                  printf("ACK recevied was: %d\n", receivedACK);
                  printf("ACKS do not match. Retransmit packet of sequence number%d\n", expectedACK);
                  expectedACK = 1;
                }
                else if (expectedACK != receivedACK && receivedACK == 1){
                 printf("ACK received was: %d\n", receivedACK);
                  printf("ACKS do not match. Retransmit packet of sequnce number: %d\n", expectedACK);
                  expectedACK = 0;
                }
                else{

                  printf("ACK sequence numbers not equal or error, ACK received is: %d \n", receivedACK);
                }
                totalACKs+=1;
            }
        }
        printf("\n\n\n");
        if (retransmissions>1){
            totalRetransmissions+=retransmissions-1;
        }

    };
    struct dataPacket temp = {.dataCount = 0, .sequenceNumber = sequenceNumber, .data = "Finished"};
    printf("End of Transmission Packet with sequence number %d transmitted with c data bytes %d\n",temp.sequenceNumber,temp.dataCount);
    sendto(sock_client, &temp, 10, 0,(struct sockaddr *) &server_addr, sizeof (server_addr));
    printf("\nTotal Packets: %d", totalPackets);
    printf("\nTotal Data: %d",totalData);
    printf("\nTotal Retransmissions: %d",totalRetransmissions);
    printf("\nTotal Transmissions: %d",totalTransmissions);
    printf("\nTotal ACKs: %d",totalACKs);
    printf("\nTotal Timeouts: %d",totalTimeouts);
    printf("\n");
    /* close the socket */

   close (sock_client);
   fclose(fp);
}
