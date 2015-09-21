/* 
File: calcSetpoint.c 
Author: Joe Hall
Date: 9/21/15
Description:
	Gets freqEst (frequency estimate) and frameNum (frame number) piped into stdin
	Calculates power setpoint and prints it to stdout
	Usage:
		-[option] 		[value]: 	description
		-----------------------------------------------------------
		-[d]eadlines 	[0 or 1]: 	1 = use frameNum in calculation
		-[M]axPower		[0, inf):	maximum allowed setpoint
		-[m]inPower		[0, inf):	minimum allowed setpoint
		-[B]ufferLength	[0, inf):	maximum frame buffer length

compile:	gcc calcSetpoint.c -o calcSetpoint
run: 		./calcSetpoint		
*/

#include <stdlib.h>
#include <stdio.h>

#ifdef _WIN32
	#include <Windows.h>
#else
	#include <unistd.h>
#endif

char* progname;
int deadlines = 0,
	maxPower = 0,
	minPower = 0;
long frameBufferLen = 0;
int rate_ms = 10;

void usage(void);
int cmdline(int argc, char *argv[]);
int get_usr_input(char *buff); // get stdin input

int main(int argc, char *argv[])
{	
	if(cmdline(argc,argv)){
		exit(EXIT_FAILURE);
	}

	int quit = 0;
	char stdin_buf[256]; /* stdin buffer for user input or pipe */
	double setpoint;

	setbuf(stdin, NULL);
	setbuf(stderr, NULL);
	setbuf(stdout, NULL);

	while (quit!=1) {

		/* get stdin or pipe input from user and transmit via COM port */
		if (get_usr_input(stdin_buf)) {
			
			printf("received %s\n", stdin_buf);

			if (stdin_buf[0] == 'q') 
				quit = 1;

		}	

#ifdef _WIN32
    	Sleep(rate_ms);
#else
    	usleep(rate_ms*1000);  /* sleep */
#endif
	
	}
	return EXIT_SUCCESS;
}

void usage(){
	printf("Usage: \n");
}

int cmdline(int argc, char *argv[]){
	int opt;
	progname = argv[0];
	while ((opt = getopt(argc, argv, "a:d:p:b:r:M:m:B:o:h")) != -1) {
		switch (opt) {
			case 'd':
				deadlines = (int)atof(optarg);
				break;
			case 'M':
				maxPower = (int)atof(optarg);
				break;
			case 'm':
				minPower = (int)atof(optarg);
				break;
			case 'B':
				frameBufferLen = (long)atof(optarg);
				break;
			case 'h':
				usage();
				exit(EXIT_SUCCESS);
				break;
			default:
				usage();
				return -1;
		}
	}

	return 0;
}

int get_usr_input(char *buff) {
    
	fd_set rfds;
    struct timeval tv;
    int retval, len;
	static int bytes_so_far = 0;

    /* Watch stdin (fd 0) to see when it has input. */
    FD_ZERO(&rfds);
    FD_SET(0, &rfds);
	/* Wait up to 1 useconds. */
    tv.tv_sec = 0;
    tv.tv_usec = 1;

    /* check if stdin has input */
    retval = select(1, &rfds, NULL, NULL, &tv);

    if (retval == -1)
    	return -1;
    
    else if (retval) {
    	len = read(0,buff,255);
        if (buff[len-1]=='\n') {
            if (bytes_so_far == 0) {
		    	buff[len-1]='\0'; // replace '\n' with '\0'
		    	//printf("Data received: %s\n",buff);
				}		
			else {
		    	//printf("Data received: %s\n",buff-bytes_so_far);		    		
		    	buff -= bytes_so_far;
				bytes_so_far = 0;	
			}
			return 1;    	 
	   	}
		else {
			bytes_so_far += len;
            buff += len;
			return 0;
	    }
	}        
	else {
		return 0;
	}
}
