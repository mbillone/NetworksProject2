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

#define STRING_SIZE 1024
double timeoutLen;
double plr;
double alr;
    

// void getData(){
//     printf("Please input a Timeout Quantity ");
//     scanf("%d", &timeoutLen);
//     printf("Please input an ACK Loss Rate ");
//     scanf("%d", &ACKLossRate);
//     printf("Please input a Packet Loss Rate ");
//     scanf("%d", &packetLossRate);
//     printf("%d",packetLossRate);
//     return;
// }

int main(void) {
    srand(time(NULL));
    int sequenceNumber= 0;
    char inputTimeoutLen[STRING_SIZE];
    char inputPacketLossRate[STRING_SIZE];
    char inputACKLossRate[STRING_SIZE];
   int sock_client;  /* Socket used by client */ 
    int firstRound= 0;
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
    sscanf(inputTimeoutLen, "%lf", &timeoutLen);
    printf("Please input an ACK Loss Rate:\n");
    scanf("%s", inputACKLossRate);
    // sscanf(inputACKLossRate, "%lf", &alr);
    printf("Please input a Packet Loss Rate:\n");
    scanf("%s", inputPacketLossRate);
    // sscanf(inputPacketLossRate, "%lf", &plr);

    printf("\n");
    int i = 0;
    // int messageLen = 0;
    char message[100];
    while ((read = getline(&line, &len, fp)) != -1){
        int flag = 0;
        memset(message, 0, 100 );
        int convertdata = read-1;
        char datastr[2];
        if (firstRound == 0){
            sprintf(message,"%s;%s;%d;%d;%s", inputPacketLossRate,inputACKLossRate,convertdata,sequenceNumber,line);
            firstRound+=1;
        }
        else{
            sprintf(message,"%d;%d;%s", convertdata,sequenceNumber,line);
        }
        
        if (sequenceNumber==0)
            sequenceNumber = 1;
        else   
            sequenceNumber = 0;
        printf("%s\n",message);
        while (flag == 0){
            bytes_sent = sendto(sock_client, message, convertdata, 0,
                (struct sockaddr *) &server_addr, sizeof (server_addr));
            printf("Waiting for response from server...\n");
            bytes_recd = recvfrom(sock_client, modifiedSentence, STRING_SIZE, 0,
                            (struct sockaddr *) 0, 0);
            printf("\nThe response from server is:\n");
            printf("%s\n\n", modifiedSentence);
            flag = 1;
        }
        // i = packetLoss();
        // if (i==0){
        //     printf("dropped\n");
        // }
        // else{
        //     printf("kept\n");
        // }
    };
    printf("\n");
//    printf("Please input a sentence:\n");
//    scanf("%s", sentence);
//    msg_len = strlen(sentence) + 1;

//    bytes_sent = sendto(sock_client, sentence, msg_len, 0,
//             (struct sockaddr *) &server_addr, sizeof (server_addr));

//    /* get response from server */
  
//    printf("Waiting for response from server...\n");
//    bytes_recd = recvfrom(sock_client, modifiedSentence, STRING_SIZE, 0,
//                 (struct sockaddr *) 0, 0);
//    printf("\nThe response from server is:\n");
//    printf("%s\n\n", modifiedSentence);

   /* close the socket */

   close (sock_client);
}
