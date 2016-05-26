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

  printf("Server open on %d\n", portno);
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

      // Connect to new server.
      connectToServer(&command);

      char bigBuff[100000];
      bzero(bigBuff, 100000);

      if(command.type == LIST) {              // Client requested a list.
        printf("List directory requested on port %d.\n", command.transport);
        fflush(stdout);
        listCommand(bigBuff, 100000);
        printf("%s\n", bigBuff);
        printf("Sending directory contents to %s:%d.\n", command.hostname, command.transport);
        fflush(stdout);
      }
      else if(command.type == GET) {          // Client requested a file.
        printf("File \"%s\" requested on port %d.\n", command.filename, command.transport);
        fflush(stdout);
        getCommand(command.filename, bigBuff);
        printf("Sending \"%s\" to %s:%d.\n", command.filename, command.hostname, command.transport);
        fflush(stdout);
      }

      n = write(sockfd, bigBuff, 100000);
      if (n < 0) error("ERROR writing to socket");

      exit(0);
    }
    else close(newsockfd);
  } /* end of while */
  close(sockfd);
  return 0; /* we never get here */
}
