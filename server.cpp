/* A simple server in the internet domain using TCP
   The port number is passed as an argument
   This version runs forever, forking off a separate
   process for each connection
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "error.c"
#include "socket.c"

#include "commands.c"



int main(int argc, char *argv[]) {
  int sockfd, newsockfd, portno, pid;
  socklen_t clilen;
  struct sockaddr_in serv_addr, cli_addr;

  if (argc < 2) {
    fprintf(stderr,"ERROR, no port provided\n");
    exit(1);
  }

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    error("ERROR opening socket");
  }

  bzero((char *) &serv_addr, sizeof(serv_addr));
  portno = atoi(argv[1]);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);
  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) error("ERROR on binding");

  // Listen on socket.
  listen(sockfd,5);
  clilen = sizeof(cli_addr);

  while (1) {
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

    if (newsockfd < 0)
      error("ERROR on accept");

    pid = fork();
    if (pid < 0)
      error("ERROR on fork");
    if (pid == 0)  {
      close(sockfd);

      char buffer[1000];
      bzero(buffer,1000);
      struct Param command;

      int n = read(newsockfd, buffer, 1000);
      if (n < 0) error("ERROR reading from socket");

      // Parse command from client.
      parseCommand(&command, buffer);


      printf("Params: %s, %d, %d, %s\n", command.hostname, command.type, command.transport, command.filename);
      fflush(stdout);


      connectToServerOther(&command);

      if(command.type == LIST) {
        listCommand(buffer, 1000);
      }
      else if(command.type == GET) {
        // printf("get file %s\n", command.filename);
        getCommand(command.filename, buffer, 1000);
        printf("Buffer: %s\n", buffer);
      }

      n = write(sockfd, buffer, 1000);
      if (n < 0) error("ERROR writing to socket");

      exit(0);
    }
    else close(newsockfd);
  } /* end of while */
  close(sockfd);
  return 0; /* we never get here */
}
