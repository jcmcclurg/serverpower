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
		-[o]utputFile   [filepath]  filepath for data log

compile:	gcc calcSetpoint.c -o calcSetpoint
run: 		./calcSetpoint		
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

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
int rate_ms = 500;
char *filepath;

void usage(void);
int cmdline(int argc, char *argv[]);
int get_usr_input(char *buff); // get stdin input
void convert_time_to_string(struct timespec tv, char* time_buf);

int main(int argc, char *argv[])
{	
	if(cmdline(argc,argv)){
		exit(EXIT_FAILURE);
	}

	int quit = 0;
	char stdin_buf[256]; /* stdin buffer for user input or pipe */
	double setpoint = 0,	/* power setpoint (algorithm output) */
		   offset = 0;
	long freq = 60000, 	/* frequency [mHz] */
		 frameNum = 1, 	/* number of frames in buffer */
		 framesInBuffer = frameBufferLen/2,
		 bufferSlope = frameBufferLen/10;
	struct timespec tv_time;
	char time_str[256];

	FILE* fp;
	fp = fopen(filepath, "w");
	if (fp!=NULL) {
		setbuf(fp, NULL);
	}	

	setbuf(stdin, NULL);
	setbuf(stderr, NULL);
	setbuf(stdout, NULL);

	while (quit!=1) {

		/* get stdin or pipe input from user and transmit via COM port */
		if (get_usr_input(stdin_buf)) {
			clock_gettime(CLOCK_REALTIME, &tv_time);
			convert_time_to_string(tv_time, time_str);

			switch (stdin_buf[0]) {
				case 'q':
					quit = 1;
					break;
				case 'f':
					freq = (long)atof(stdin_buf+1);
					break;
				default:
					if ((long)atof(stdin_buf)) {
						frameNum = (long)atof(stdin_buf);
					}
			}

			/* for debug: print received info */
			//printf("received %s\n", stdin_buf);
			//printf("freq = %ld\tframeNum = %ld\n",freq,frameNum);

			/* log data */
			if (fp != NULL) {
				fprintf(fp,"%s,%ld,%.2f,%ld,%.2f,\n",time_str,freq,setpoint,frameNum,offset);
			}

		}	

/* sleep for a bit */
#ifdef _WIN32
    	Sleep(rate_ms);
#else
    	usleep(rate_ms*1000);  /* sleep */
#endif
		
		/* Calculate Setpoint */
		setpoint = (double)(maxPower+minPower)/2+((maxPower-minPower)/(60.03-59.97))*(freq-60000)/1000;
		if (deadlines) {
			if (frameNum > (frameBufferLen-bufferSlope)) {
				offset = -(bufferSlope-(frameBufferLen-frameNum))*(maxPower-minPower)/bufferSlope;
			}
			else if (frameNum < bufferSlope) {
				offset = (bufferSlope-frameNum)*(maxPower-minPower)/bufferSlope;
			}
			else {
				offset = 0;
			}
			setpoint += offset;
			//fprintf(stdout,"setpoint offset = %.2f\n",offset);
		}
		
		/* Keep it in bounds */
		if (setpoint > (double)maxPower)
			setpoint = maxPower;
		if (setpoint < (double)minPower)
			setpoint = minPower;

		/* Send Setpoint*/
		fprintf(stdout,"s%.2f\n",setpoint);
	}
	return EXIT_SUCCESS;
}

void usage(){
	printf("Usage: \n");
}

int cmdline(int argc, char *argv[]){
	int opt;
	progname = argv[0];
	while ((opt = getopt(argc, argv, "d:M:m:B:h")) != -1) {
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
			case 'o':
				filepath = optarg;
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

void convert_time_to_string(struct timespec tv, char* time_buf)
{
    time_t sec;
    long nsec;
    struct tm *timeinfo;
    char tmp_buf[15];

    sec = tv.tv_sec;
    timeinfo = localtime(&sec);
    nsec = tv.tv_nsec;

    strftime(tmp_buf, 15, "%H:%M:%S", timeinfo);
    sprintf(time_buf, "%s.%9ld",tmp_buf,nsec);
}


/*
char* parseString(char *buff) {
	char output[256];
	for (i=0;((buff[i] != '\n') && (buff[i] != '\0') && (i<255));i++) {
		output[i] = buff[i];
	}
	if (buff
}
*/
