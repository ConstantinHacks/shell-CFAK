/*
  Constantin Koehler Simple Shell
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define DELIMS " \t\r\n"
#define INITALBUFFER 1000

void loop(void);
int execute(char**);
char** parseString(char*);

void loop(void){
  char* commandString = malloc(sizeof(char) * INITALBUFFER);
  ssize_t init_buffsize = 0;

  char** tokens;
  do{
    printf("> ");
    getline(&commandString,&init_buffsize,stdin);
    tokens = parseString(commandString);
    execute(tokens);
    commandString = malloc(sizeof(char) * INITALBUFFER);
  } while (1);
}

int execute(char** tokens){
  pid_t pid;

  if ((pid = fork()) == -1)
     perror("fork error");
  else if (pid == 0) {
     execv("/bin/ls", tokens);
     printf("Return not expected. Must be an execv error.n");
  }

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
        exit(1);
      }
    }

    token = strtok_r(NULL,DELIMS,&save);
  }
  tokens[index] = NULL;
  return tokens;
}

int main(){
  loop();
  return 0;
}
