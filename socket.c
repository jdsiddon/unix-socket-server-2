#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


enum req_type {LIST, GET};

struct Param {
  char hostname[100];
  int contport;
  int transport;
  enum req_type type;
  char filename[100];
};


/******** DOSTUFF() *********************
 There is a separate instance of this function
 for each connection.  It handles all communication
 once a connnection has been established.
 *****************************************/
void dostuff (int sock)
{
   int n;
   char buffer[256];

   printf("In 'dostuff'\n");

   bzero(buffer,256);
   n = read(sock,buffer,255);
   if (n < 0) error("ERROR reading from socket");
   printf("Here is the message: %s\n",buffer);
   n = write(sock,"I got your message",18);
   if (n < 0) error("ERROR writing to socket");
}



void createServer(int portno) {
  int sockfd, newsockfd, pid;
  socklen_t clilen;
  struct sockaddr_in serv_addr, cli_addr;

  printf("%d\n", portno);

  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd < 0)
    error("ERROR opening socket");

  bzero((char *) &serv_addr, sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
   error("ERROR on binding");

  listen(sockfd,5);
  clilen = sizeof(cli_addr);

  printf("Waiting for connections...\n");

  while (1) {
     newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

     if (newsockfd < 0)
         error("ERROR on accept");

     pid = fork();
     if (pid < 0)
         error("ERROR on fork");
     if (pid == 0)  {
         close(sockfd);
         dostuff(newsockfd);
         exit(0);
     }
     else close(newsockfd);
  } /* end of while */
  close(sockfd);
  return; /* we never get here */
}








/******** DOSTUFF() *********************
 There is a separate instance of this function
 for each connection.  It handles all communication
 once a connnection has been established.
 *****************************************/
void connectToServer (int sock, int portno)
{
   int n;
   char buffer[256];


   // Connect to new connection.
   int sockfd;
   struct sockaddr_in serv_addr;
   struct hostent *server;


   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   if (sockfd < 0)
       error("ERROR opening socket");
   server = gethostbyname("localhost");

   if (server == NULL) {
       fprintf(stderr,"ERROR, no such host\n");
       exit(0);
   }

   bzero((char *) &serv_addr, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
   serv_addr.sin_port = htons(portno);

   if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
       error("ERROR connecting");


  n = write(sockfd,"I got your message",18);
  if (n < 0) error("ERROR writing to socket");

}






void connectToServerOther(int sock, int portno)
{
   int n;
   char buffer[256];


   // Connect to new connection.
   int sockfd;
   struct sockaddr_in serv_addr;
   struct hostent *server;

   printf("New Port: %d\n", portno);
   fflush(stdout);

   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   if (sockfd < 0)
       error("ERROR opening socket");
   server = gethostbyname("localhost");

   if (server == NULL) {
       fprintf(stderr,"ERROR, no such host\n");
       exit(0);
   }

   bzero((char *) &serv_addr, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
   serv_addr.sin_port = htons(portno);

   if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
       error("ERROR connecting");


  n = write(sockfd,"I got your message",18);
  if (n < 0) error("ERROR writing to socket");

}
