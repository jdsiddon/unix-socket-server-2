/**************************************************
** File: client.c
** Author: Justin Siddon
** Description: This file provides the client interface to a file server.
**  Once it send a message to a server (in the correct format) it starts
**  its own server based on the passed port for the server to connect to.
**************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "error.c"
#include "socket.c"


// hostName is a pointer to a pointer to argv[1]
/**************************************************
** Function: parseParams
** Description: This function parses apart the parameters
http://www.gnu.org/software/libc/manual/html_node/Simple-Directory-Lister.html
** Parameters: char *path, path the user would like to change directory to.
** Returns: None
**************************************************/
void parseParams(int count, char* params[], struct Param *param ) {

  if(count < 5) {                  // Check correct command line arguments provided.
    error("usage: [hostname] [control port] [control command] [filename] [reply port]\n");   // Provide hostname and port.
  }

  strcpy(param->hostname, params[1]);               // Get hostname.
  param->contport = atoi(params[2]);     // Convert control port number.

  // Command type.
  if(strcmp(params[3], "-l") == 0) {
    param->type = LIST;
  } else {
    param->type = GET;
  }

  // Check if they provided a file name or not.
  if(count < 6) {
    param->transport = atoi(params[4]);  // Convert transfer port number.
    strcpy(param->filename, "");
  } else {
    strcpy(param->filename, params[4]);
    param->transport = atoi(params[5]);
  }
}


int main(int argc, char *argv[]) {
  int sockfd, portno, n;
  struct sockaddr_in serv_addr;
  struct hostent *server;

  char buffer[1000];
  struct Param serverParams;

  // Parse user passed in parameters and store in serverParams.
  parseParams(argc, argv, &serverParams);

  // printf("Back from parsing params");
  // printf("hostname: %s, contport: %d, transport: %d, req_type: %d, filename: %s\n",
  // serverParams.hostname, serverParams.contport, serverParams.transport, serverParams.type, serverParams.filename);

  portno = serverParams.contport;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
    error("ERROR opening socket");

  server = gethostbyname(serverParams.hostname);
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


  // Create a command from the user entered params and store it in the 'serverParams' struct.
  createCommand(&serverParams, buffer, 1000);

  // printf("sending: %s\n", buffer);

  // Send command to server.
  n = write(sockfd, buffer, strlen(buffer));
  if (n < 0)
    error("ERROR writing to socket");

  // Create server to receive data back from server.
  createServer(&serverParams);

  // bzero(buffer, 1000);
  // readSocket(sockfd);
  // n = read(sockfd, buffer, 999);
  // if (n < 0)
  //   error("ERROR reading from socket");

  // printf("fdafds\n");
  // close(sockfd);
  return 0;
}
