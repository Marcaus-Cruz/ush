#ifndef BUILTIN_H
#define BUILTIN_H

/*Is Built In
 * Char ** args:    Array of strings that may contain a built in
 * 
 * Checks if the input contains a built in function.
 * Returns index of which built in is being called if one is
 * returns -1 otherwise.
 */

int isBuiltIn(char* args[]);

/*Run Built In
 * char** args: string with the built in inside
 * 
 * Runs the built in function.
 */
void runBuiltIn(char* args[]);

#endif
