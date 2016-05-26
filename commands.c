#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>

/**************************************************
** Function: cdCommand
** Description: This is a built in shell function to change the current working directory,
**  to that specified by path. If path is blank, changes directory to that specified by
**  by the HOME path variable in the users environment.
http://www.gnu.org/software/libc/manual/html_node/Simple-Directory-Lister.html
** Parameters: char *path, path the user would like to change directory to.
** Returns: None
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
    while(ep = readdir(dir)) {
      printf("%s\n", ep->d_name);   // Print content name.
      strcat(tempBuff, ep->d_name);
      strcat(tempBuff, "\n");
    }
    (void) closedir(dir);           // Listed all items, close directory.
  }
  resultSize = strlen(tempBuff);

  // printf("Length of Directory Contents: %d\n", resultSize);

  // Place the message size in front of the rest of the message.
  sprintf(buffer, "%d", resultSize);
  strcat(buffer, ":");
  strcat(buffer, tempBuff);

}

void getCommand(char *filename, char *buffer, int size) {
  FILE *file = fopen(filename, "r");         // Open file for reading.
  char lineBuffer[100];
  char tempBuff[100000];
  int resultSize = 0;

  while(fgets(lineBuffer, 100, file) != NULL) {  // Read through each line in the file.
    //printf("%s\n", buffer);
    strcat(tempBuff, lineBuffer);
  }

  // printf("%s\n", buffer);
  resultSize = strlen(tempBuff);

  // Place the message size in front of the rest of the message.
  sprintf(buffer, "%d", resultSize);
  strcat(buffer, ":");
  strcat(buffer, tempBuff);

  fclose(file);
}
