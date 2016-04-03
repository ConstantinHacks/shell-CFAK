/*
  Constantin Koehler Simple Shell
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include "utils.h"

#define DELIMS " \n"
#define INITALBUFFER 512

void loop();
char* readIn(void);
int execute(char**);
char** parseString(char*);
int launch(char**);
int shell_cd(char**);
int shell_getenv(char**);
int shell_setenv(char**);


void loop(){

  char *commandString;
  char **tokens;
  int status;

  do{
    printf("> ");
    commandString = readIn();
    printf("First Char: %c\n",commandString[0]);
    if (strncmp(commandString,"#",1) == 0){
      printf("Comment, Ignore\n");
      status = 1;
    } else {
      tokens = parseString(commandString);
      status = execute(tokens);
    }
  } while (status);
}

int execute(char** args){

  if(args[0] == NULL){
    return 1;
  }

  if(strcmp(args[0],"cd") == 0 || strcmp(args[0],"chdir") == 0) {
    return shell_cd(args);
  }
  else if (strcmp(args[0],"setenv") == 0){
    return shell_setenv(args);
  }
  else if (strcmp(args[0],"getenv") == 0){
    return shell_getenv(args);
  }
  else if (strcmp(args[0],"echo") == 0){
    char* newStr = args[1];
    int counter = 2;
    while(1){
      if(args[counter] == NULL){
        break;
      }
      strcat(newStr,args[counter]);
      strcat(newStr," ");
      counter++;
    }
    printf("%s\n",unescape(newStr,stderr));
    return 1;
  }
  else if (strcmp(args[0],"exit") == 0){
    int exit_code = atoi(args[1]);
    if (args[1] == NULL){
      exit(0);
    }
    else if (exit_code >= 1 && exit_code <= 9){
      exit(exit_code);
    }
    else{
      fprintf(stderr, "Not a Valid Exit Command\n");
      return 1;
    }
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
    // printf("Token added: %s\n", token);
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

int shell_cd(char **args){
  if(args[1] == NULL){
    fprintf(stderr, "Need better argument to cd\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("Error!");
    }
    char dir[PATH_MAX];
    getcwd(dir,PATH_MAX);
    setenv("PWD",dir,1);
  }
  return 1;
}

int shell_setenv(char **args){
  //setenv MESSAGE="Hello, World"
  //setenv MESSAGE="Hello,\ \"Constantin\".\ How" are you today?"

  if(args[1] == NULL){
    fprintf(stderr, "Need better argument to setenv\n");
    return -1;
  }
  char* newStr;
  int pathIndex = 0;
  char* path;
  int counter = 2;

  strcpy(newStr,args[1]);
  for(int i=0; i < strlen(newStr);i++){
    if(newStr[i] == '='){
      pathIndex = i;
    }
  }
  if(pathIndex == 0){
    fprintf(stderr, "Need better argument to setenv\n");
    return -1;
  }

  strncpy(path,newStr,pathIndex); // path
  path[strlen(path)] = '\0'; // null terminate

  //TODO errorcheck path

  newStr += pathIndex+1; //chop off path and =

  while(1){
    strcat(newStr," ");
    if(args[counter] == NULL){
      break;
    }
    strcat(newStr,args[counter]);
    counter++;
  }
  printf("%s\n",unescape(newStr,stderr));
  setenv(path,unescape(newStr,stderr),1);
  return 1;
}

int shell_getenv(char **args){
  if(args[1] == NULL){
    fprintf(stderr, "Need better argument to getenv\n");
  } else {
    char* newStr;
    int pathIndex = 0;
    char* path = args[1];

    strncpy(path,newStr,pathIndex); // path
    path[strlen(path)] = '\0'; // null terminate
    char* env = getenv(path);
    if(!env){
      fprintf(stderr, "%s is not a valid path\n",path);
      printf("\n");
      return 1;
    }
    printf("%s\n",env);
    return 1;
  }
}

int main(int argc,char **argv){
  loop();
  return 0;
}
