#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include "builtin.h"


typedef void (*builtin_t) (char* args[]);

/*Built In Exit
 * char** args: array of pointers with first index as "exit"
 * 
 * exits the program with whatever number args[1] is
 */
static void builtInExit(char* args[]);

/*Built In Aecho
 * char ** args: array of pointers with first index as "aecho"
 * 
 * Prints out whatever arguments follow "aecho".
 * User can input "-n" as args[1] for Aecho to not print a new line after
 * printing the arguments. 
 */
static void builtInAecho(char* args[]);

/*Built In CD
 * char** args: array of pointers with first index as "cd"
 * 
 * Changes the current working directory to whatever the user inputs
 * as args[1]
 */
static void builtInCd(char* args[]);

/*Built In Env Set
 * char** args: array of pointers with first index as "envset"
 * 
 * Sets the name of an environment from args[1] to a variable from args[2]
 */
static void builtInEnvSet(char* args[]);

/*Built In Env Unset
 * char** args: array of pointers with first index as "envunset"
 * 
 * Unsets an environment given from user in args[1]. Will give the user an
 * error if environment isn't set.
 */
static void builtInEnvUnset(char* args[]);

/* Array of addresses of all built in commands */
static builtin_t builtIns[] = {&builtInExit, &builtInAecho, &builtInCd, &builtInEnvSet, &builtInEnvUnset, NULL};

/* Array of the built in commands that the user must input to use them */
static char* finder[] = {"exit", "aecho", "cd", "envset", "envunset", NULL};

/* Find Built In
 * line:    string of the built in
 * 
 * Returns the index of the built in in finder which is
 * passed to isBuiltIn and runBuiltIn. Returns 0 if not
 * a built in.
 */
static int findBuiltIn(char* line){
    int i = 0;
    while(finder[i] != NULL && strcmp(line, finder[i]) != 0){
        i++;
    }
    return i;
}

int isBuiltIn(char* args[]){
    int i = findBuiltIn(args[0]);
    return finder[i] != NULL;
}

void runBuiltIn(char* args[]){
    int i = findBuiltIn(args[0]);
    
    if(finder[i] != NULL){
        builtIns[i](args);   
    }
    
}

static void builtInExit(char* args[]){
    if(args[1] != NULL){
        exit(atoi(args[1]));
    }
    else {
      exit(0); 
    }
}

static void builtInAecho(char* args[]){
    int i = 1;
    
    if(strcmp(args[1],"-n") == 0){
        i++;
        while(args[i] != NULL){
            dprintf(1, "%s ", args[i]);
            i++;
        }
    }
    else {
        while(args[i] != NULL){
            dprintf(1, "%s ", args[i]);
            i++;
        }
        dprintf(1, "\n");
    }
}

static void builtInCd(char* args[]){
    if(args[1] != NULL){
        if(chdir(args[1])){
            perror("cd");
        }
    }
}

static void builtInEnvSet(char* args[]){
    if(args[1] != NULL && args[2] != NULL){
        int check = setenv(args[1], args[2], 1);
        if(check == -1){
            perror("envset");
        }
    }
}

static void builtInEnvUnset(char* args[]){
    if(args[1] != NULL){
        int check = unsetenv(args[1]);
        if(check == -1){
            perror("envunset");
        }
    }
}

