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

// void error(const char *msg)
// {
//     perror(msg);
//     exit(0);
// }
//
// void startServer();

// enum req_type {LIST, GET};
//
// struct Param {
//   char hostname[100];
//   int contport;
//   int transport;
//   enum req_type type;
//   char filename[100];
// };
//

// hostName is a pointer to a pointer to argv[1]
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


// struct Param {
//   char hostname[100];
//   int contport;
//   int transport;
//   enum req_type type;
//   char filename[100];
// };
//


int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    struct Param serverParams;

    parseParams(argc, argv, &serverParams);

    printf("Back from parsing params");
    printf("hostname: %s, contport: %d, transport: %d, req_type: %d, filename: %s\n",
      serverParams.hostname, serverParams.contport, serverParams.transport, serverParams.type, serverParams.filename);

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

    // printf("Please enter the message: ");
    bzero(buffer,256);

    sprintf(buffer, "%d", serverParams.transport);

    printf("%s\n", buffer);

    // fgets(buffer,255,stdin);
    n = write(sockfd,buffer,strlen(buffer));

    if (n < 0)
      error("ERROR writing to socket");

    createServer(serverParams.transport);

    bzero(buffer,256);
    n = read(sockfd,buffer,255);
    if (n < 0)
         error("ERROR reading from socket");

    printf("%s\n",buffer);
    close(sockfd);
    return 0;
}
