/*
 * Tai Smith
 * COMP 350-001
 * November 24, 2020
 * Final Checkpoint of the Unix ASH Shell Project: This program interactively executes
 * single commands with or with out arguments and is supposed to
 * include redirection and batch processing.
 */
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <sys/wait.h>
#include <cstring>
#include <cstdint>
#include <stdlib.h>
#include <cstdlib>
#include <sys/types.h>

#include <errno.h>
#include <assert.h>
#include <fcntl.h>
#include <ctype.h>

using namespace std;

char **splitFunction(char *line) { //takes character point and splits tokens into arguments
  int size = 64;
  int position = 0;
  char **tokens = (char **) malloc(size * sizeof(char *));
  char *symbol;

  if (!tokens) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  symbol = strtok(line, " \n");
  while (symbol != NULL) {
    tokens[position] = symbol;
    position++;

    if (position >= size) {
      size += 64;
      tokens = (char **) realloc(tokens, size * sizeof(char *));
      if (!tokens) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
    symbol = strtok(NULL, " \n");
  }
  tokens[position] = NULL;
  return tokens;
}

char *readLines() {  //reads line that gets typed into command prompt and puts it in a character pointer
  char *line = NULL;
  size_t size = 0;
  getline(&line, &size, stdin);
  return line;
}

char *get_cd() {    //repeatedly print out the current directory
  char existingDirectory[1024];
  getcwd(existingDirectory, sizeof(existingDirectory));
  printf("\n%s", existingDirectory);
  return 0;
}

int cdFunction(char **args) {   //uses the chdir() system call with the argument supplied by the user
  if (args[1] == NULL) {
    cout << "Please enter one argument" << endl;
  } else if (args[2] != NULL) {
    cout << "Please enter one argument" << endl;
  } else {
    if (chdir(args[1]) != 0) {
      cout << "Please enter only one valid argument" << endl;
    }
    return 1;
  }
  return 1;
}

int pathFunction(char **path) {  // takes 0 or more arguments, with each argument separated by whitespace from the others.
  if (path == NULL) {
    if (chdir(getenv("/usr")) == -1) {
      cout << "Newly specified path created" << endl;
    }
    return 1;
  }
    /*if (chdir(*(path + 1)) == 0) {
     setenv("oldpath", getenv("newpath"), 0);
   }
     */
  else {
    if (chdir(path[1]) == -1) {
      cout << "Path overwritten" << endl;
    }
  }
  return 1;
}

int exitFunction(char **args) {    //couts exit farewell
  cout << "Unix Shell Created by Tai Smith \n";
  return 0;
}

char *commands[] = {(char *) "exit", (char *) "cd", (char *) "path"};

int (*commandFunctions[])(char **) =  {&exitFunction, &cdFunction, &pathFunction};

int ashCommands() {
  return sizeof(commands) / sizeof(char *);
}

int forkFunction(char **args) { //uses execvp to take first argument as a command. A fork is created
  int output;
  pid_t pid;
  pid_t pid2;
  char **argIn;
  int argCount;
  int pos = 0;
  char str[500];
  int status;
  pid = fork();
  if (pid == 0) {
    // while(fgets(str, 500, stdin) != NULL) {
    // cout << "String loaded: " << str;
    //argCount = makearg(str, &argIn);
    //execvp(argIn[0], argIn);

    if (execvp(args[0], args) == -1) {
      cout << "Please enter valid command" << endl;
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    cout << "Please enter valid command" << endl;
  }
  else {
    do {
      pid2 = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  dup2(output, 1);
  return 1;
}

int redirection(int argc, char *argv[]) {  //attempt at redirection
  int output;
  char *args[3];
  char *arguments[300];
  char file[300];
  int num = 0;
  int redirect;
  /*
  if (output) {
    redirect = splitFunction2(arguments[num], ">", args);
  }
line = fgets(line, 3000, file);
argCount = split(line, "\n;", arguments);
*/
  string arg = "";
  int n;
  int direct;
  n = fork();
  int fd[2]; // an array that will hold file descriptors
  char buffer[100];
  char *out[3];
  if (n < 0) {
    //fork has failed
    printf("Error \n");
    exit(1);
  } else if (n == 0) {
    if ((direct = open(file, 0, 300)) < 0) {
      execvp(out[0], out);
    }
    close(fd[0]);  // Child process closes up input side
    dup2(fd[1], 1);  //Duplicating a file descriptor
    dup2(direct, 1);
    close(fd[1]);
    execvp(args[0], args);
  } else {
    wait(NULL);
    close(fd[1]);   //Parent process closes up output side
    dup2(fd[0], 1);     //Duplicating a file descriptor
    close(fd[0]);
    read(fd[0], buffer, 100);  //read to a file
  }
return 0;
}

char *strdup(const char *t) {  //implement strdup function manually (used to duplicate a string)
  size_t length = strlen(t);
  char *result = (char *) malloc(length + 1);
  if (result == NULL) {
    return NULL;
  }
  std::memcpy(result, t, length + 1);
  return result;
}

int batchMode(char **args, char *argv[]) {  //intended to read input from a batch file and executes commands
  FILE *batchFile;
  char line[100];  //linesize
  char *val;
  char *result;
  while (fgets(line, sizeof(line), batchFile)) {
    batchFile = fopen(argv[1], "r");  //open batch file for input operations
    result = strtok(NULL, ",");
    if (batchFile == NULL) {
      cout << "Error opening batch file\n" << endl;
      exit(0);
    }
  }
  fclose(batchFile);
  return 0;
}

int executeCommands(char **args) { //takes arguments and calls forkFunction(), interactive or batch
  if (args[0] == NULL) {
    return 1;
  }

  //if (args[2] == NULL) {      //switch to batch mode, segmentation fault
  // batchMode(args,NULL);
  // }

  for (int i = 0; i < ashCommands(); i++) {
    if (strcmp(args[0], commands[i]) == 0) {
      return (*commandFunctions[i])(args);
    }
  }
  return forkFunction(args);
}


void loops() {  //brings functions together in do while loop
  char *existingDirectory;
  char *line;
  int position;
  char **args;
  do {
    existingDirectory = get_cd();

    printf("%s > ", existingDirectory);  //causes prompt to say null
    line = readLines();
    args = splitFunction(line);
    position = executeCommands(args);
    free(line);
    free(args);
    free(existingDirectory);
  } while (position);
}

int main() {

  loops();

  return 0;
//    if (command == "cd") {
//      if (argument > 1 || 0) {
//        cout << "Error. Please enter only one argument \n";
//      }
//      if (command == "cd" && "cd" == 0) {
//         chdir(argv[1]);
//      }
//      cdFunction(inputToken + 1);
//    }
//    //if the current working directory is /usr,
//    //you can call chdir("bin") to enter /usr/bin.)
//    if (command == "path") {
//      if (argument < 0) {
//        pathFunction(inputToken);
//      }
//    }
//  }
//  }
}