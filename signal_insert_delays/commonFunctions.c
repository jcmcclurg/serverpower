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
char* readLine(void){
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

// Returns true if stdin has stuff to read, false if not.
int checkStdin(void){
	fd_set rfds;
	struct timeval tv;
	int retval;

	/* Watch stdin (fd 0) for 0 seconds to see when it has input. */
	FD_ZERO(&rfds);
	FD_SET(0, &rfds);
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	/* check if stdin has input */
	retval = select(1, &rfds, NULL, NULL, &tv);
	if(retval == -1){
		perror("checkStdin() experienced error.");
		exit(EXIT_FAILURE);
	}

	return retval;
}
