#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#include "commonFunctions.h"

// Define the function to be called when ctrl-c (SIGINT) signal is sent to process
void terminateProgram(int signum) {
   // Cleanup and close up stuff here

   // Terminate program
   printf("Terminating from signal %d.\n",signum);
   exit(signum);
}

// Reads a line from stdin, and strips the newline at the end.
// Exits the program if someone types more than BUFLEN characters in a line.
char _readLine_buf[READLINE_BUFLEN];
char* readLine(){
	char* str;
	int len;
	_readLine_buf[READLINE_BUFLEN - 1] = 0;
	str = fgets(_readLine_buf, READLINE_BUFLEN, stdin);
	if(str != NULL){
		// Just to be sure that strlen doesn't go off the end of the string.
		if(str[READLINE_BUFLEN - 1] != 0){
			perror("Line length too long. Caused a buffer overflow in readLine().");
			exit(EXIT_FAILURE);
		}

		// Remove the newline character.
		len = strlen(str);
		str[len - 1] = 0;
	}
	return str;
}
