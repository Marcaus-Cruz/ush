/* CS 347 -- Mini Shell!  
 * Original author Phil Nelson 2000
 * Modified by Aran Clauson 2019, Marcaus Cruz 2020
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "argparse.h"
#include "builtin.h"


/* CONSTANTS */ 

/* PROTOTYPES */


/* Process Line
 * line  The command to process
 * 
 * The parameter line is interpreted as a command name.  This function creates a
 * new process that executes that command.  
 */
void processline (char *line);

/* Get Input
 * line     A pointer to a char* that points at a buffer of size *size or NULL.
 * size     The size of the buffer *line or 0 if *line is NULL.
 * returns  The length of the string stored in *line.  
 *
 * This function prompts the user for input.  If the input fits in the buffer
 * pointed to by *line, the input is placed in *line.  However, if there is not
 * enough room in *line, *line is freed and a new buffer of adequate space is
 * allocated.  The number of bytes allocated is stored in *size.  
 */
ssize_t getinput(char** line, size_t* size);

/* New Line
 *
 * Literally prints a new line to std out.
*/
void newLine(){
  dprintf(1, "\n");
}


/* Main
 * 
 * This function is the main entry point to the program.  This is essentially
 * the primary read-eval-print loop of the command interpreter.  
 */
int main () {

  signal(2, newLine);

  char*   line = NULL;
  size_t  size = 0;
  ssize_t len = getinput(&line, &size);

  while(len != -1) {
    processline (line);
    len = getinput(&line, &size);
  }

  if (!feof(stdin))
    perror ("read");
    
  free(line);
  return EXIT_SUCCESS;		
}

/* Get Input
 * 
 */
ssize_t getinput(char** line, size_t* size) {
  assert(line != NULL && size != NULL);

  fprintf(stderr, "%% ");
  ssize_t len = getline(line, size, stdin);
  while(errno == EINTR){
    errno = 0;
    len = getline(line, size, stdin);
  }
  
  if(len > 0 && (*line)[len-1] == '\n') {
    (*line)[len-1] = '\0';
    --len;
  }

  return len;
}


/* Process Line
 *
 * Note the three cases of the switch: fork failed, fork succeeded and this is
 * the child, fork succeeded and this is the parent (see fork(2)).  
 */
void processline (char *line)
{
  assert(line != NULL);

  pid_t cpid; 
  int   status;
  int argNum;
  char new[1024] = {'\0'};
  int newSize = 1024;
  
  if(expand(line, new, newSize)){
        line = new;
  }
  char** args = argparse(line, &argNum);
  
  if(args[0] != NULL){
    if(isBuiltIn(args)){
        runBuiltIn(args);
    } else {
        
        cpid = fork();
        switch(cpid) {
        case -1:
            perror("fork");
            break;
            
        case 0:
            execvp(args[0], args);
            perror("exec");
            exit(EXIT_FAILURE);
            
        default:
            wait (&status);
            break;
        }
    }
  }
  free(args);
}
