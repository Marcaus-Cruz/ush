#ifndef ARGPARSE_H
#define ARGPARSE_H

/* Arg Parse
 * line     The string of arguments
 * argcp    The number of arguments
 *
 * Separates arguments in line and stores into an array of pointers 
 * to each arg. argcp will be used for built-in commands. Client code 
 * should free memory returned by this program.
 */
char** argparse(char* line, int* argcp);

/* Expand
 * orig    The input string that may contain variables to be expanded
 * new     An output buffer that will contain a copy of orig with all 
 *         variables expanded
 * newsize The size of the buffer pointed to by new.
 * returns 1 upon success or 0 upon failure. 
 *
 * Example: "Hello, ${PLACE}" will expand to "Hello, World" when the environment
 * variable PLACE="World".
 */
int expand(char* orig, char* new, int newsize);

#endif
