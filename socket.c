/**************************************************
** File: socket.c
** Author: Justin Siddon
** Description: This file provides the standard functions
**  to read from the socket, parse apart commands passed, and
**  connect to another socket.
**  Code heavily based off: http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html
**************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <wait.h>


enum req_type {LIST, GET};

// Struct to hold passed parameters
struct Param {
  char hostname[100];
  int contport;
  int transport;
  enum req_type type;
  char filename[100];
};


/**************************************************
** Function: readSocket
** Description: Reads all the data from the socket and places it into the passed buffer.
**  based off code from: http://stackoverflow.com/questions/666601/what-is-the-correct-way-of-reading-from-a-tcp-socket-in-c-c
** Parameters: int socket - socket to read from. char *returnBuff - buffer to store
**  data read from the socket.
** Returns: socket data stored in the passed buffer.
**************************************************/
void readSocket(int socket, char *returnBuff) {
  char buffer[100000];
  bzero(buffer, 100000);
  char *sizeString;
  int messageSize = 0;
  int bytesRead = 0;

  char *message;          // Pointer to message portion.

  bytesRead = read(socket, buffer, 100000);   // Get first send to pluck off the total message size.
  sizeString = strtok(buffer, ":");           // Get value as string
  messageSize = atoi(sizeString);             // Convert to integer.
  message = strtok(NULL, "");

  strcpy(returnBuff, message);                 // Record data so far/

  while (bytesRead < messageSize) {           // Keep reading data until got all.
    bzero(buffer, 100000);
    bytesRead += read(socket, buffer, messageSize - bytesRead);
    strcat(returnBuff, buffer);
  }

  return;
}


/**************************************************
** Function: handleRequest
** Description: Converts a command received by a connected socket client from a text
**  buffer to a Param struct.
**  buffer could be in two formats
**  [hostname] [type] [return socket] [filename (only if type == GET)]
** Parameters: struct Param *param - pointer to param struct, char *buffer - text
**  that was sent to socket client.
** Returns: 1 if command is good, 0 if command isn't valid.
**************************************************/
int handleRequest(struct Param *params, char *buffer) {
  char *param;
  int paramCount = 0;

  param = strtok(buffer, " ");

  // Loop through each passed parameter and create a struct.
  while(param != NULL) {
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


/**************************************************
** Function: connectToServer
** Description: Connects to a server with the passed params in the
**  Param struct.
**  Based off: http://www.linuxhowtos.org/C_C++/socket.htm
** Parameters: struct Param *server - server attributes to connect to.
** Returns: Nothing.
**************************************************/
void connectToServer(struct Param *params) {
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
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) == 0) {  // Connection is successful.
      break;
    }
  }
}
