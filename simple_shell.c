/*
  Constantin Koehler Simple Shell Implementation
*/

#include "simple_shell.h"

void runScript(FILE* scriptFile,FILE* historyFile){

  char buf[INITALBUFFER];
  char **tokens;

  if(scriptFile){
    while(fgets (buf,sizeof(buf),scriptFile)){
      if (strncmp(buf,"#",1) == 0){
        // printf("Comment, Ignore\n");
      } else {
        tokens = parseString(buf);
        execute(tokens,historyFile);
      }
    }
  }

}

void loop(FILE* historyFile){

  char *commandString;
  char **tokens;
  int status;

  do{
    printf("> ");
    commandString = readIn();
    fprintf(historyFile, "%s\n",commandString);
    if (strncmp(commandString,"#",1) == 0){
      printf("Comment, Ignore\n");
      status = 1;
    } else {
      tokens = parseString(commandString);
      status = execute(tokens,historyFile);
    }
  } while (status);
}

int execute(char** args,FILE* fp){
  int lastArg,lastLetter;
  int flag = 0;

  if(args[0] == NULL){
    return 1;
  }

  // printf("Execute: %s",args[0]);

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

    if (!args[1]){
      fclose(fp);
      exit(0);
    }
    int exit_code = atoi(args[1]);
    if (exit_code >= 1 && exit_code <= 9){
      fclose(fp);
      exit(exit_code);
    }
    else{
      fprintf(stderr, "Not a Valid Exit Command\n");
      return 1;
    }
  }


  for (lastArg = 0;args[lastArg] != NULL; lastArg++){
    // printf("Count: %d\n",lastArg);
  }
  lastArg--;

  for (lastLetter = 0; args[lastArg][lastLetter] != '\0';lastLetter++){
    // printf("Letter: %c\n",args[lastArg][lastLetter]);
  }
  lastLetter--;

  if(args[lastArg][lastLetter] == '&'){
    // printf("Background!\n");
    flag = 1;
    args[lastArg] = NULL;
  }

  return launch(args,flag);
}

int launch(char **args,int flag){
  pid_t pid;
  int status;

  // printf("launch %s\n", args[0]);

  pid = fork();
  if(pid == 0){
    if (execvp(args[0],args) == -1) {
      perror("Launch Error");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0){
    perror("Fork Error!");
  }
  if(flag == 0)
    waitpid(pid,NULL,0);

  return 1;
}

char** parseString(char* line){
  int index = 0;
  int space = 0;
  int bufSize = INITALBUFFER;
  char token[INITALBUFFER];
  char* save;
  char** tokens = malloc(INITALBUFFER * sizeof(char*));

  // printf("In parseString, spaces: %d\n",count_spaces(line));

  if(count_spaces(line) == 0){
    tokens[index] = line;
    tokens[index+1] = NULL;
    return tokens;
  }

  do {
    space = first_unquoted_space(line);
    strncpy(token,line,space);
    token[space] = '\0';
    tokens[index] = unescape(token,stderr);
    line += space+1;
    index++;

    if(index >= bufSize){
      bufSize += INITALBUFFER;
      tokens = realloc(tokens,INITALBUFFER*sizeof(char*));
      if(!tokens){
        fprintf(stderr, "Allocation Error\n");
        exit(EXIT_FAILURE);
      }
    }
  } while(first_unquoted_space(line) != -1);

  tokens[index++] = unescape(line,stderr);

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
  }
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

int main(int argc,char **argv){

  FILE* historyFile;
  FILE* profileFile;
  FILE* scriptFile;
  char* homedir;
  char* profileDir;
  char* scriptDir;

  homedir = getpwuid(getuid())->pw_dir;

  strcat(homedir,"/");
  profileDir = homedir;
  strcat(profileDir,".421sh_profile");
  historyFile = fopen(".421sh_history","a");
  profileFile = fopen(profileDir,"r");

  if(argc >= 2){
    strcpy(scriptDir,homedir);
    strcat(scriptDir,argv[1]);
    scriptFile = fopen(scriptDir,"r");
    if(!scriptFile){
      fprintf(stderr, "File \"%s\" not found!\n",argv[1]);
      exit(-1);
    } else {
      runScript(scriptFile,historyFile);
      exit(0);
    }
  }

  if(profileFile){
    runScript(profileFile,historyFile);
  }
  loop(historyFile);
  return 0;
}
