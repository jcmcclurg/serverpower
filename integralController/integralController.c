#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#include "commonFunctions.h"

int main(void) {
	// Register signal and signal handler
	signal(SIGINT, terminateProgram);

	char* str;
	int len;
	double f;
	do {
		str = readLine();
		if(str != NULL){
			printf("Got string \"%s\"\n",str);
		}
	} while(str != NULL);

	return EXIT_SUCCESS;
}
