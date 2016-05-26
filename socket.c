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
** Function: createCommand
** Description: Creates a command string in buffer with the following format:
**  [hostname] [command type] [transport number] [filename]
** Parameters: struct Param *params - requested data. char *buffer - command to be stored for sending
**  int size - size of the passed buffer.
** Returns: command in the passed buffer.
**************************************************/
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
** Function: saveFile
** Description: Saves file to current folder.
** Parameters: char *filename - filename to save the file as. char *contents
**  pointer to the data to store in the file.
** Returns: noting.
**************************************************/
int saveFile(char *filename, char *contents) {
  FILE *file;
  int success = 1;

  file = fopen(filename, "r");

  if(file != NULL) {
    printf("File already stored!\nSelect new file or delete the existing file to get a fresh copy!\n");                  // Write type to file.
    fclose(file);
    success = 0;

  } else {
    file = fopen(filename, "w");
    if(file != NULL) {
      fprintf(file, "%s\n", contents);                  // Write type to file.
      fclose(file);
    } else {
      success = 0;
    }
  }

  return success;
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
** Function: createServer
** Description: Creates the server side of socket based on the parameters
**  passed in. Used for the client to host a socket for the server to connect back to
**  to send its data to. Based off code from: http://www.linuxhowtos.org/C_C++/socket.htm
** Parameters: struct Param *server - server attributes to set up.
** Returns: Nothing.
**************************************************/
void createServer(struct Param *server) {
  int sockfd, newsockfd, pid, status;
  socklen_t clilen;
  struct sockaddr_in serv_addr, cli_addr;


  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd < 0)
    error("ERROR opening socket");

  bzero((char *) &serv_addr, sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(server->transport);

  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    error("ERROR on binding");

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
      char readBuff[100000];                   // Big buffer to hold data.

      readSocket(newsockfd, readBuff);         // Read data sent by server.

      if(server->type == GET) {                // Requested a file get, so save it.
        // Check if file was found.
        char *message = strtok(readBuff, " ");
        if(strcmp(message, "[ERROR]") == 0) {     // "[ERROR]" indicates file read was unsuccessful
          message = strtok(NULL, "");
          printf("%s:%d says %s\n", server->hostname, server->transport, message);
          fflush(stdout);
        } else {
          int saved = saveFile(server->filename, readBuff);
          if(saved) {
            printf("File transfer complete.\n");
            fflush(stdout);
          }
        }
      }
      else if(server->type == LIST) {          // Requested file list, so print list.
        printf("Receiving directory structure from %s:%d\n", server->hostname, server->transport);
        fflush(stdout);
        printf("%s\n", readBuff);
        fflush(stdout);
      }

      exit(0);
    }
    else {
      waitpid(0, &status, 0);                 // Wait for child to complete.
      close(newsockfd);
      return;
    }
  } /* end while */
  return;
}



/**************************************************
** Function: connectToServer
** Description: Connects to a server with the passed params in the
**  Param struct.
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
