/**************************************************
** File: server.cpp
** Author: Justin Siddon
** Description: This file contains the code that starts up a simple file/command
**  server on the passed port. It then returns the client requested data
**  by connecting to the 'transport port number' passed by the client. The client
**  sets up a server and the server connects, becoming a client of the client.
**  Code based off: http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html
**************************************************/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

// Custom libraries
#include "error.c"
#include "socket.c"
#include "commands.c"

// Prototypes
void startUp(int);

int main(int argc, char *argv[]) {
  if (argc < 2) {                 // Make sure they at leasted provided a port.
    fprintf(stderr,"ERROR, no port provided\n");
    exit(1);
  } else {
    startUp(atoi(argv[1]));       // Start the server.
  }
  return 1;
}


/**************************************************
** Function: startUp
** Description: Starts up the list/get file server. Once it receives a request
**  from the client, it starts up a connection back to the client on the passed
**  port. Based off http://www.linuxhowtos.org/C_C++/socket.htm
** Parameters: int portno - port number to start the server on.
** Returns: Nothing.
**************************************************/
void startUp(int portno) {
  int sockfd, newsockfd, pid;
  socklen_t clilen;
  struct sockaddr_in serv_addr, cli_addr;

  // Create socket.
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    error("ERROR opening socket");
  }

  // Set up address
  bzero((char *) &serv_addr, sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);
  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) error("ERROR on binding");

  printf("Server open on %d\n", portno);

  // Listen on socket.
  listen(sockfd,5);
  clilen = sizeof(cli_addr);

  while (1) {
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);   // Accept a connection
    if (newsockfd < 0)
      error("ERROR on accept");

    // Start new process to connect to client's 'server' to send requested data to.
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

      // Parse/handle command from client.
      handleRequest(&command, buffer);
      printf("Connection from %s\n", command.hostname);

      // Connect to new server.
      connectToServer(&command);

      char bigBuff[100000];
      bzero(bigBuff, 100000);

      // Print out status and execute the command.
      if(command.type == LIST) {              // Client requested a list.
        printf("List directory requested on port %d.\n", command.transport);
        fflush(stdout);

        listCommand(bigBuff, 100000);         // Exec list command.

        printf("Sending directory contents to %s:%d.\n", command.hostname, command.transport);
        fflush(stdout);
      }
      else if(command.type == GET) {          // Client requested a file.
        printf("File \"%s\" requested on port %d.\n", command.filename, command.transport);
        fflush(stdout);
        int fileRead = getCommand(command.filename, bigBuff);
        if(fileRead > 0) {        // File was found.
          printf("Sending \"%s\" to %s:%d.\n", command.filename, command.hostname, command.transport);
          fflush(stdout);
        } else {                  // File was not found.
          printf("File not found. Sending error message to %s:%d.\n", command.hostname, command.transport);
          fflush(stdout);
        }
      }

      // Write back to client the result of the command.
      n = write(sockfd, bigBuff, 100000);
      if (n < 0) error("ERROR writing to socket");

      exit(0);
    }
    else close(newsockfd);
  } /* end of while */
  close(sockfd);        // Close our socket
  return;
}
