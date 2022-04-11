#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include "argparse.h"

/* Num of Args
 * line:    string of command arguments
 * 
 * Returns the number of arguments
 */
static int numOfArgs(char* line);

/* Quote Count
 * line:    string of command arguments
 * 
 * Returns the number of quotation marks in line.
 * If there is an odd number of quotation marks, 
 * argparse will see line as invalid.
 */
static int quoteCount(char* line);

/* Remove Quotes
 * line:    string that may or may not contain quotes 
 *
 * Changes line to remove quotes if line contains them.
 */
static void rmquotes(char* line);

/* Expand Check
 * line:    input string with variables that may need expanding
 * 
 * Returns a 1 if variables need to be expanded and 0 otherwise. 
 */
static int expandCheck(char *line);

/* Remove Comment
 *
 * line:     string that needs comment removal
 * index:    index of where '#' is
 *
 * Changes line and sets each element from index to end of string to null.
 */
static void rmComment(char* line, int index);

/*
       space, #                    character, #
         +---+                        +---+
         |   |                        |   |
         |   v                        |   v
       +--------+    character     +--------+
       |--------| +--------------> |--------|
       ||  B   ||                  ||   A  ||
+----> ||      ||                  ||      ||
       |--------| <--------------+ |--------|
       +--------+      space       +--------+
            |                            |
            |                        ^   |
            |                      " |   | "
            |                        |   v
            |                        |
            |                      +-+------+
            |                      |        |
            +--------------------> |   Q    |
                "                  |        |
                                   |        |
                                   +-+---+--+
                                     |   ^
                                     |   |
                                     +---+
                              character, space, #
*/

char** argparse(char* line, int* argcp){
    
    assert(line != NULL);
    
    char** args = NULL;
    
    int quotes = quoteCount(line);
    if((quotes % 2) != 0){
        dprintf(1, "Invalid quotation marks\n");
        *argcp = 0;
    }
    else {
        int count = numOfArgs(line);
        *argcp = count-1;
        
        args = (char**)malloc(sizeof(char*)*count);
        
        int i = 0;
        int j = 0;
        int q = 0;
        enum{B, A, Q} state = B;
        
        while(line[i] != '\0'){
            
            if(state == B){
                if(isspace(line[i])){
                    line[i] = '\0';
                }
                else if(line[i] == '\"'){
                    q = i;
                    state = Q;
                } 
                else if(line[i] == '#'){
                    rmComment(line, i);
                    args[j] = &line[i];
                    j++;
                } 
                else if(!isspace(line[i])){
                    args[j] = &line[i];
                    j++;
                    state = A;
                }
            }
            else if(state == A){
                if(line[i] == '#'){
                    rmComment(line, i);
                    args[j] = &line[i];
                    j++;
                } 
                else if(line[i] == '\"'){
                    q = i;
                    state = Q;
                } 
                else if(isspace(line[i])){
                    line[i] = '\0';
                    state = B;
                }
            }
            else if(state == Q){
                if(line[i] == '\"'){
                    args[j] = &line[q];
                    j++;
                    state = A;
                } 
            }
            i++;
        }
        args[j] = NULL;
        
        for(int x = 0; args[x] != NULL; ++x){
            rmquotes(args[x]);
        }
        
    }
    return args;
}

static void rmquotes(char* line){
    char* from = line;
    while(*line != '\0'){
        if(*from != '\"'){
            *line = *from;
            ++line;
        }
        ++from;
    }
}

static void rmComment(char* line, int index){
    int i = index;
    while(line[i] != '\0'){
        line[i] = '\0';
        i++;
    }
}

/*
        space                       character
        +----+                       +----+
        |    |                       |    |
        |    v                       |    v
       +-------+     character      +-------+
       |-------| +----------------> |-------|
       ||  B  ||                    ||  A  ||
+----> ||     ||                    ||     ||
       |-------| <----------------+ |-------|
       +-------+       space        +-------+
*/

static int numOfArgs(char* line){
    
    int count = 0;
    int i = 0;
    enum{B, A} state = B;
    while(line[i] != '\0'){
        
        if(state == B){
            if(!isspace(line[i])){
                count++;
                state = A;
            }
        }
        else if(state == A){
            if(isspace(line[i])){
                state = B;
            }
        }
        i++;
    }
    
    count++; //null character
    return count;
}


static int quoteCount(char* line){

    int i = 0;
    int counter = 0;
    while(line[i] != '\0'){
        if(line[i] == '\"'){
            counter++;   
        }
        i++;
    }
    return counter;
}

/*   
I know you hate these "state machines" but since it's what I initially had, I just wanted to stick with it.

                   +----------+
                   |----------|          ${             +--------+
                   ||        || +---------------------> |        | +---+
             +---> ||  BET   ||                         |  VAR   |     | anything else
$$, c, space |     ||        ||                         |        | <---+
             +---+ ||        || <---------------------+ |        |
                   |----------|            }            +--------+
                   +----------+
                       |  ^
                       |  |
                    $( |  |  )
                       |  |
                       v  |
                    +--+--+--+
                    |        |
                    |  CMD   |
                    |        |
                    |        |
                    +-+---+--+
                      |   ^
                      |   |
                      +---+
                   anything else


I have some code at the bottom of this file trying to show that I did try. I just don't understand really.
*/

int expand(char* orig, char* new, int newsize){
    
    if(expandCheck(orig) == 1){
        
        enum{BET, VAR} state = BET;
        
        int i = 0;
        int j = 0;
        int varSpot;        
        int emptyEnv;
        
        while(orig[i] != '\0'){
            if(state == BET){
                if(orig[i] == '$' && orig[i+1] == '{'){
                    emptyEnv = i;
                    i = i + 2;
                    varSpot = i;
                    state = VAR;
                } 
                else if(orig[i] == '$' && orig[i+1] == '$'){
                    i = i + 2;
                    j += sprintf(&new[j], "%d", getpid());
                }
                else {
                    new[j] = orig[i];
                    i++;
                    j++;
                }
                
            } else if(state == VAR){
                if(orig[i] == '}'){
                    char varName [1024] = "";
                    int v = 0;
                    
                    while(orig[varSpot] != '}'){
                        varName[v] = orig[varSpot];
                        varSpot++;
                        v++;
                    }
                    
                    char* envName = getenv(varName);
                    
                    if(envName == NULL){
                        dprintf(1, "Environment not set.\n");
                        orig[emptyEnv] = '\0';
                        return 0;
                    }

                    int e = 0;
                    while(envName[e] != '\0'){
                        new[j] = envName[e];
                        j++;
                        e++;
                    }
                    i++;
                    state = BET;
                }
                else {
                    i++;
                }
            } 
        }
        return 1;
    }
    else {   
            return 0;
    }
}

/*

                +---------+                           +---------+
                |---------|             $             |---------|
         +----> ||       || +-----------------------> ||       ||
c, space |      ||  BET  ||                           ||  DOL  ||
         +----+ ||       || <-----------------------+ ||       ||
                |---------|        $ or not {         |---------|
                +---------+                           +---------+
                     ^                                    |
                     |                                    |
                     |              +-------+             |
                     |              |       |             |
                     +------------+ |  VAR  | <-----------+
                        } or )      |       |    { or (
                                    +-------+

                                     ^    +
                                     |    |
                                     |    |
                                     +----+
                                    c, space
*/

static int expandCheck(char *line){
    
    enum{BET, DOL, VAR} state = BET;
    
    int i = 0;
    int invalidVar; 
    int countExpansions = 0;
    
    while(line[i] != '\0'){
        if(state == BET){
            if(line[i] == '$'){
                invalidVar = i;
                state = DOL;
            }
        } 
        else if(state == DOL){
            if(line[i] == '{'){
                state = VAR;
            } 
            else if(line[i] == '$'){
                countExpansions++;
                state = BET; 
            }
            else if(line[i] == '('){
                state = VAR;
            }
            else{
                state = BET;
            }
        }
        else if(state == VAR){
            if(line[i] == '}'){
                countExpansions++;
                state = BET;
            }
            else if(line[i] == ')'){
                countExpansions++;
                state = BET;
            }
            else if(line[i] == '\0'){
                dprintf(1, "error: no matching bracket\n");
                line[invalidVar] = '\0';
                return 0;
            }
        }
        i++;
    }
    
    return (countExpansions != 0);
}



/* mercy, please

struct Stack{
    char* top;
    struct Stack* next;
};


char* pop(struct Stack* st){
    assert(st != NULL);

    char* top1 = *st->top;
    struct Stack old = st;
    *st = old->next;
    free(old);
    return top1;
    
} 

void push(struct Stack* st, char* top){
    struct stack* n = malloc(sizeof(n));
    if(n!=NULL){
        n->top = top;
        n->next = NULL;
        while((*st) != NULL){
            st = &((*st)->next);
        }
        *st = n;
    }
}


pid_t spawn(char** args, int in, int out){
    pid_t cid = fork();
    if(cid == 0){
        int i = dup(in);
        int o = dup(out);
        dup2(i, 0);
        dup2(o, 1);
        closefrom(3);

        execvp(args[0], args);
        perror("exec");
        exit(1);
    }
    return cid;
}

*/
//        enum{BET, VAR, CMD} state = BET;
//       int cmd;
/*                else if(orig[i] == '$' && orig[i+1] == '('){
                    i = i + 2;
                    cmd = i;
                    push(st,cmd)
                    state = CMD;
                }
*/
/*            else if(state == CMD){
                if(orig[i] == ')'){
                    int fd[2];
                    cmd = pop();
                    char** args = argparse(cmd);
                    pipe
                    spawn
                    while(read(new, 1)){
                        new++;
                    }
                } else{
                    i++;
                }
            }
*/