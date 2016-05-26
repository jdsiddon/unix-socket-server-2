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
      printf("%s\n", ep->d_name);   // Print content name.
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
** Returns: Nothing
**************************************************/
void getCommand(char *filename, char *buffer) {
  FILE *file = fopen(filename, "r");         // Open file for reading.
  char lineBuffer[100];
  char tempBuff[100000];
  int resultSize = 0;

  while(fgets(lineBuffer, 100, file) != NULL) {  // Read through each line in the file.
    strcat(tempBuff, lineBuffer);
  }

  resultSize = strlen(tempBuff);

  // Place the message size in front of the rest of the message.
  sprintf(buffer, "%d", resultSize);
  strcat(buffer, ":");
  strcat(buffer, tempBuff);

  fclose(file);   // Close file.
}
