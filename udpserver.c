/* udp_server.c */
/* Programmed by Adarsh Sethi */
/* Sept. 13, 2018 */

#include <ctype.h>          /* for toupper */
#include <stdio.h>          /* for standard I/O functions */
#include <stdlib.h>         /* for exit */
#include <string.h>         /* for memset */
#include <sys/socket.h>     /* for socket, sendto, and recvfrom */
#include <netinet/in.h>     /* for sockaddr_in */
#include <unistd.h>         /* for close */

#define STRING_SIZE 1024

/* SERV_UDP_PORT is the port number on which the server listens for
   incoming messages from clients. You should change this to a different
   number to prevent conflicts with others in the class. */

#define SERV_UDP_PORT 3636

double timeoutLen;
typedef struct dataPacket{
  int dataCount;
  int sequenceNumber;
  char data[80];
} dataPacket;

int receivedSequence;
int dataLength;
int expectedSequence = 0;
int ackSequence = 0;
char ack[2];
float inputPacketLossRate;
float inputACKLossRate;

int packetLoss(){
    double random = drand48();
    if (random<inputPacketLossRate){
        return 0;
    }
    else{
        return 1;
    }
}
int ackLoss(){
    double random = drand48();
    if (random<inputACKLossRate){
        return 0;
    }
    else{
        return 1;
    }
}

int main(void) {

   int sock_server;  /* Socket on which server listens to clients */

   struct sockaddr_in server_addr;  /* Internet address structure that
                                        stores server address */
   unsigned short server_port;  /* Port number used by server (local port) */

   struct sockaddr_in client_addr;  /* Internet address structure that
                                        stores client address */
   unsigned int client_addr_len;  /* Length of client address structure */

   char sentence[STRING_SIZE];  /* receive message */
   char modifiedSentence[STRING_SIZE]; /* send message */
   unsigned int msg_len;  /* length of message */
   int bytes_sent, bytes_recd; /* number of bytes sent or received */
   unsigned int i;  /* temporary loop variable */
   int successfulDataPackets = 0;
   int bytesDeliveredToUser = 0;
   int duplicatePackets = 0;
   int packetsDroppedDueToLoss = 0;
   int allPacketsReceived = 0;
   int acksWithoutLoss = 0;
   int droppedACKs = 0;
   FILE *fp;
   /* open a socket */
   fp = fopen("output.txt", "w");
   if ((sock_server = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
      perror("Server: can't open datagram socket\n");
      exit(1);
   }

   /* initialize server address information */

   memset(&server_addr, 0, sizeof(server_addr));
   server_addr.sin_family = AF_INET;
   server_addr.sin_addr.s_addr = htonl (INADDR_ANY);  /* This allows choice of
                                        any host interface, if more than one
                                        are present */
   server_port = SERV_UDP_PORT; /* Server will listen on this port */
   server_addr.sin_port = htons(server_port);

   /* bind the socket to the local server port */

   if (bind(sock_server, (struct sockaddr *) &server_addr,
                                    sizeof (server_addr)) < 0) {
      perror("Server: can't bind to local address\n");
      close(sock_server);
      exit(1);
   }

   // User input for the ACKLossRate and packetLossRate
   printf("Please input an ACK Loss Rate:\n");
   scanf("%f", &inputACKLossRate);
   printf("Please input a Packet Loss Rate:\n");
   scanf("%f", &inputPacketLossRate);

   /* wait for incoming messages in an indefinite loop */

   printf("Waiting for incoming messages on port %hu\n\n",
                           server_port);

   client_addr_len = sizeof (client_addr);

    struct dataPacket temp;
   for (;;) {

     int pl = packetLoss();
     int al = ackLoss();
     memset(temp.data, 0, 80 );
      memset(sentence, 0, STRING_SIZE );
      bytes_recd = recvfrom(sock_server, &temp, sizeof(temp), 0,
                     (struct sockaddr *) &client_addr, &client_addr_len);
        allPacketsReceived+=1;

        dataLength = temp.dataCount;

        receivedSequence = temp.sequenceNumber;

        if (dataLength==0){
            int totalAcks = droppedACKs+acksWithoutLoss;
            printf("End of Transmission Packet with sequence number %d received with c data bytes %d\n",temp.sequenceNumber,temp.dataCount);
            printf("\nSuccessful Data Packets: %d",successfulDataPackets);
            printf("\nBytes Delivered to Users: %d",bytesDeliveredToUser);
            printf("\nDuplicate Packets: %d",duplicatePackets);
            printf("\nPackets Dropped Due to Loss: %d",packetsDroppedDueToLoss);
            printf("\nAll Packets Received: %d",allPacketsReceived);
            printf("\nACKs Without Loss: %d", acksWithoutLoss);
            printf("\nDropped ACKs: %d",droppedACKs);
            printf("\nTotal ACKs: %d",totalAcks);
            printf("\n");
            fclose(fp);
            exit(1);
        }
        else if (pl==0){
            packetsDroppedDueToLoss +=1;
            printf("\nPacket %d lost\n",temp.sequenceNumber);
        }
        else{
            if (expectedSequence != receivedSequence && receivedSequence == 1){
                duplicatePackets+=1;
                ackSequence = 0;
                printf("Duplicate packet %d received with %d databytes",temp.sequenceNumber,temp.dataCount);
            }
            else if(expectedSequence != receivedSequence && receivedSequence == 0){
                duplicatePackets+=1;
                ackSequence = 1;
                printf("Duplicate packet %d received with %d databytes",temp.sequenceNumber,temp.dataCount);
            }
            else if(expectedSequence == receivedSequence && receivedSequence == 1){
                printf("Packet %d received with %d data bytes", temp.sequenceNumber, temp.dataCount);
                expectedSequence = 0;
                ackSequence = 1;
            }
            else if(expectedSequence == receivedSequence && receivedSequence == 0){
                printf("Packet %d received with %d data bytes", temp.sequenceNumber, temp.dataCount); 
                expectedSequence = 1;
                ackSequence = 0;
            }
            else {
                printf("Error on comparing sequence numbers\n");
                EXIT_FAILURE;
            }


            /* prepare the ack to send */

            sprintf(ack, "%d", ackSequence);
            // msg_len = bytes_recd;
            // for (i=0; i<msg_len; i++)
            //    modifiedSentence[i] = toupper (sentence[i]);



            /* send message */

            int al = ackLoss();
            if (al==1){
                successfulDataPackets+=1;
                bytesDeliveredToUser+=temp.dataCount;
                acksWithoutLoss+=1;
                printf("ACK %s transmitted\n", ack);
                bytes_sent = sendto(sock_server, ack, 2, 0,
                    (struct sockaddr*) &client_addr, client_addr_len);
            }
            else if (al==0){
                droppedACKs+=1;
                packetsDroppedDueToLoss +=1;
                printf("\nACK %s lost\n", ack);
            }
            else{
                printf("\n ISSUE\n");
            }
        }
   }
}
