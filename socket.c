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


// http://stackoverflow.com/questions/666601/what-is-the-correct-way-of-reading-from-a-tcp-socket-in-c-c
void readSocket(int socket, char *returnBuff) {
  char buffer[100000];
  bzero(buffer, 100000);
  char *sizeString;
  int messageSize = 0;
  int bytesRead = 0;


  bytesRead = read(socket, buffer, 100000);   // Get first send to pluck off the total message size.
  printf("%d\n", bytesRead);
  sizeString = strtok(buffer, ":");           // Get value as string
  messageSize = atoi(sizeString);             // Convert to integer.

  strcpy(returnBuff, buffer);                 // Record data so far.

  while (bytesRead < messageSize) {           // Keep reading data until got all.
    bzero(buffer, 100000);
    bytesRead += read(socket, buffer, messageSize - bytesRead);
    strcat(returnBuff, buffer);
  }

  // printf("%s\n", returnBuff);

  return;
}

// Creates a command string in buffer with the following format:
//  [hostname] [command type] [transport number] [filename]
void createCommand(struct Param *params, char *buffer, int size) {
  char tempBuff[1000];      // Temp buffer to hold converted integers.

  // Connection successful, send command.
  bzero(buffer, size);

  // Hostname
  strcpy(buffer, params->hostname);
  strcat(buffer, " ");

  // Command type
  sprintf(tempBuff, "%d", params->type);
  strcat(buffer, tempBuff);
  strcat(buffer, " ");

  // Transport port number
  bzero(tempBuff, 1000);
  sprintf(tempBuff, "%d", params->transport);
  strcat(buffer, tempBuff);

  // File name is present
  if(strlen(params->filename) > 0) {
    strcat(buffer, " ");
    strcat(buffer, params->filename);
  }
  return;

}




/**************************************************
** Function: parseCommand
** Description: Converts a command received by a connected socket client from a text
**  buffer to a Param struct.
**  buffer could be in two formats
**  [hostname] [type] [return socket] [filename (only if type == GET)]
** Parameters: struct Param *param - pointer to param struct, char *buffer - text
**  that was sent to socket client.
** Returns: 1 if command is good, 0 if command isn't valid.
**************************************************/
int parseCommand(struct Param *params, char *buffer) {
  char *param;
  int paramCount = 0;

  param = strtok(buffer, " ");

  while(param != NULL) {

    printf("%s\n", param);

    if(paramCount == 0) {                 // Hostname
      strcpy(params->hostname, param);
    }
    else if(paramCount == 1) {            // Type
      if(strcmp(param, "0") == 0) {       // Client requests list.
        params->type = LIST;
      }
      else if(strcmp(param, "1") == 0) {  // Client requests a file.
        params->type = GET;
      }
      else {
        return 0;                         // Invalid command.
      }
    }
    else if(paramCount == 2) {            // Type.
      params->transport = atoi(param);
    }
    else if(params->type == GET && paramCount == 3) {    // Get, next arg will be file name.
      strcpy(params->filename, param);
    }

    param = strtok(NULL, " ");
    paramCount++;
  }


  if(params->type == GET && paramCount < 3) {            // Didn't provide enough args for get.
    return 0;
  }
  else if(params->type == LIST && paramCount > 2) {
    return 0;
  }

  return 1;
}







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
      // dostuff(newsockfd);
      char readBuff[100000];
      readSocket(newsockfd, readBuff);
      printf("%s\n", readBuff);
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
void connectToServer (int sock, int portno) {
  int n;
  // char buffer[256];

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





void connectToServerOther(struct Param *params) {
  int n;
  int sockfd;
  struct sockaddr_in serv_addr;
  struct hostent *server;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
     error("ERROR opening socket");

  server = gethostbyname(params->hostname);
  if (server == NULL) {
     fprintf(stderr,"ERROR, no such host\n");
     exit(0);
  }

  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr,
    (char *)&serv_addr.sin_addr.s_addr,
    server->h_length);
  serv_addr.sin_port = htons(params->transport);

  // Continue looping until connection is successful.
  while(1) {
    printf("Connecting to %d...\n", params->transport);
    fflush(stdout);

    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
      printf("Unsuccessful Connection...\n");
      fflush(stdout);

    } else {      // Connection was opened!
      printf("Successfully Connected!\n");
      fflush(stdout);

      break;
    }
  }

  // Write to server.
  // n = write(sockfd, "file 1 \n file 2\n", 17);
  // if (n < 0) error("ERROR writing to socket");
}
