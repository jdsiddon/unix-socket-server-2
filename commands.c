/**************************************************
** File: client.c
** Author: Justin Siddon
** Description: This file processes the commands received by the server or clients.
**  Has the GET command which returns the contents of a file into a buffer.
**  Has the LIST command which returns the contents of a directory in a buffer.
**************************************************/
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>

/**************************************************
** Function: listCommand
** Description: This function gets a list of files in the current directory and
**  places it into the passed buffer.
**  Based off code at: http://www.gnu.org/software/libc/manual/html_node/Simple-Directory-Lister.html
** Parameters: char *buffer - buffer to place folder contents into.
**  int size - size of th passed buffer.
** Returns: Nothing
**************************************************/
void listCommand(char *buffer, int size) {
  DIR *dir;
  struct dirent *ep;
  char tempBuff[100000];
  int resultSize = 0;

  bzero(tempBuff, 100000);
  bzero(buffer, size);

  dir = opendir("./");              // Open current directory.

  if(dir != NULL) {
    while((ep = readdir(dir))) {
      // printf("%s\n", ep->d_name);   // Print content name.
      strcat(tempBuff, ep->d_name);
      strcat(tempBuff, "\n");
    }
    (void) closedir(dir);           // Listed all items, close directory.
  }

  resultSize = strlen(tempBuff);

  // Place the message size in front of the rest of the message.
  sprintf(buffer, "%d", resultSize);
  strcat(buffer, ":");
  strcat(buffer, tempBuff);
}

/**************************************************
** Function: getCommand
** Description: This function opens the passed file and places its contents
**  into the passed buffer.
**  Based off code at: http://www.gnu.org/software/libc/manual/html_node/Simple-Directory-Lister.html
** Parameters: char *filename - name of file to read.
**  char *buffer - buffer to place the file contents into.
** Returns: 1 if success (file available), 0 if not.
**************************************************/
int getCommand(char *filename, char *buffer) {
  char lineBuffer[100];
  char tempBuff[100000];
  int resultSize = 0;
  FILE *file = fopen(filename, "r");         // Open file for reading.
  int success = 1;

  if(file == NULL) {          // File doesn't exist.
    strcat(tempBuff, "[ERROR] FILE NOT FOUND\n"); // "[ERROR] indicates error to client."
    success = 0;        // File not found. Failure :(

  } else {                    // File opened.
    while(fgets(lineBuffer, 100, file) != NULL) {  // Read through each line in the file.
      strcat(tempBuff, lineBuffer);
    }
    fclose(file);   // Close file.
  }

  resultSize = strlen(tempBuff);      // Get size of file.

  // Place the message size in front of the rest of the message.
  sprintf(buffer, "%d", resultSize);
  strcat(buffer, ":");
  strcat(buffer, tempBuff);
  return success;     // Sucess!
}
