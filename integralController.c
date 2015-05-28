#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#define BUFLEN 64

// Define the function to be called when ctrl-c (SIGINT) signal is sent to process
void signal_callback_handler(int signum) {
   // Cleanup and close up stuff here

   // Terminate program
   printf("Terminating from signal %d.\n",signum);
   exit(signum);
}

// Reads a line from stdin, and strips the newline at the end.
// Exits the program if someone types more than BUFLEN characters in a line.
char _readLine_buf[BUFLEN];
char* readLine(){
	char* str;
	int len;
	_readLine_buf[BUFLEN - 1] = 0;
	str = fgets(_readLine_buf, BUFLEN, stdin);
	if(str != NULL){
		// Just to be sure that strlen doesn't go off the end of the string.
		if(str[BUFLEN - 1] != 0){
			perror("Line length too long. Caused a buffer overflow in readLine().");
			exit(EXIT_FAILURE);
		}

		// Remove the newline character.
		len = strlen(str);
		str[len - 1] = 0;
	}
	return str;
}

int main(void) {
	// Register signal and signal handler
	signal(SIGINT, signal_callback_handler);

	char buf[BUFLEN];
	char* str;
	int len;
	double f;
	do {
		str = readLine();
		if(str != NULL){
			
			printf("Got string %s\n",str)
		}
	} while(str != NULL);

	return EXIT_SUCCESS;
}
