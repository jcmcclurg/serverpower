#ifndef commonFunctions_h
#define commonFunctions_h

#define READLINE_BUFLEN 64

// Terminates the program, and prints a message indicating the signal.
void terminateProgram(int sigNum);

// Reads a line from stdin and strips the newline at the end. Exits
// the program with EXIT_FAILURE if someone types more than
// READLINE_BUFLEN characters per line.
char* readLine(void);

int checkStdin(void);

#endif
