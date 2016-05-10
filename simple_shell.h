/*
  Constantin Koehler Simple Shell Header File
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <sys/wait.h>
#include "utils.h"
#include <ctype.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/types.h>

#define DELIMS " \n\r"
#define INITALBUFFER 512
#define PATHMAX 1024
void loop(FILE*);
void runScript(FILE*,FILE*);
char* readIn(void);
int execute(char**,FILE*);
char** parseString(char*);
int launch(char**,int);
int shell_cd(char**);
int shell_getenv(char**);
int shell_setenv(char**);
