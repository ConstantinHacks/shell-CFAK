/*
  Constantin Koehler Simple Shell
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define DELIMS " \t\r\n"
#define INITALBUFFER 512

void loop();
char* readIn(void);
int execute(char**);
char** parseString(char*);
int launch(char **args);
int cd(char **args);

void loop(){

  char *commandString;
  char **tokens;
  int status;

  do{
    printf("> ");
    commandString = readIn();
    tokens = parseString(commandString);
    status = execute(tokens);
  } while (status);
}

int execute(char** args){
  int i;

  if(args[0] == NULL){
    return 1;
  }

  if(strcmp(args[0],"cd") == 0 || strcmp(args[0],"chdir") == 0) {
    return cd(args);
  }
  // other stuff

  return launch(args);
}

int launch(char **args){
  pid_t pid;
  int status;

  pid = fork();
  if(pid == 0){
    if (execvp(args[0],args) == -1) {
      perror("Error!");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0){
    perror("Fork Error!");
  }

  return 1;

}

char** parseString(char* line){
  int index = 0;
  int bufSize = INITALBUFFER;
  char* token;
  char* save;
  char** tokens = malloc(INITALBUFFER * sizeof(char*));

  token = strtok_r(line,DELIMS,&save);
  while(token != NULL){
    tokens[index] = token;
    index++;

    if(index >= bufSize){
      bufSize += INITALBUFFER;
      tokens = realloc(tokens,INITALBUFFER*sizeof(char*));
      if(!tokens){
        fprintf(stderr, "Allocation Error\n");
        exit(1);
      }
    }

    token = strtok_r(NULL,DELIMS,&save);
  }
  tokens[index] = NULL;
  return tokens;
}

char* readIn(void){

  char* buffer = malloc(sizeof(char) * INITALBUFFER);
  int position = 0;
  int commandString;
  int bufferSize = INITALBUFFER;

  if(!buffer) {
    fprintf(stderr, "Allocation Error!\n" );
    exit(EXIT_FAILURE);
  }

  while(1){
    commandString = getchar();

    if( commandString == EOF || commandString == '\n' ) {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = commandString;
    }
    ++position;

    if (position >= bufferSize){
      //add on 512 more
      bufferSize += INITALBUFFER;
      buffer = realloc(buffer,bufferSize);
      if(!buffer){
        fprintf(stderr, "Memory reallocation error!\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

int cd(char **args){
  if(args[1] == NULL){
    fprintf(stderr, "Need better argument to cd\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("Error!");
    }
  }
  return 1;
}

int main(int argc,char **argv){
  loop();
  return 0;
}
