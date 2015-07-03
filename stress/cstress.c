#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define MAXDUTY 0.999
#define MINDUTY 0.001

long num_iterations;

void usage(char* cmd_name){
	fprintf(stdout,"%s [-h print help] [-n] [number of iterations]\n",cmd_name);
}

int cmdline(int argc, char** argv){
	num_iterations = -1;

	if(argc <= 3){
		if(argc == 1){
			 num_iterations = 2000000;
		}
		else if(argc == 2){
			num_iterations = atol(argv[1]);
		}
		else if(argc == 3 && argv[1][0] == '-' && argv[1][1] == 'n' ){
			num_iterations = atol(argv[2]);
		}
	}

	if(num_iterations < 1){
		return -1;
	}
	else{
		return 0;
	}
}

int main (int argc, char** argv) {
	useconds_t sleeplen;
	int num_readable;
	long i;
	double s;
	int fd_stdin = fileno(stdin);
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	double slen = 0;
	double duty = 0.999;
	char buf[128];
	fd_set readfds;
	double prevTime_sec, currentTime_sec;
	struct timespec currentTime;

	// Clear output buffering on STDOUT
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);

	if(cmdline(argc,argv)){
		usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	fprintf(stderr,"Set duty to %lf\n",duty);
	slen = 0;
	sleeplen = 0;
	if(clock_gettime(CLOCK_BOOTTIME, &currentTime)){
		fprintf(stderr,"Problem with gettime\n");
		exit(EXIT_FAILURE);
	}
	prevTime_sec = ((double)(currentTime.tv_sec)) + (((double)(currentTime.tv_nsec))/1.0e9);

	while(1) {
		FD_ZERO(&readfds);
		FD_SET(fd_stdin, &readfds);
		num_readable = select(fd_stdin +1, &readfds, NULL, NULL, &tv);
		if (num_readable == 0) {
			for(i = 0; i < num_iterations; i++){
				s = sqrt((double) i);
			}
			if(clock_gettime(CLOCK_BOOTTIME, &currentTime)){
				fprintf(stderr,"Problem with gettime\n");
				exit(EXIT_FAILURE);
			}
			currentTime_sec = ((double)(currentTime.tv_sec)) + (((double)(currentTime.tv_nsec))/1.0e9);
			slen = (1.0-duty)*(currentTime_sec - prevTime_sec);
			sleeplen = (useconds_t)(slen*1.0e6);
			//fprintf(stderr,"%lf...\n",slen);
			usleep(sleeplen);
			prevTime_sec = currentTime_sec;
		}
		else if (num_readable == 1) {
			if(scanf("%lf",&duty) > 0){
				if(duty > MAXDUTY){
					duty = MAXDUTY;
				}
				else if(duty < MINDUTY){
					duty = MINDUTY;
				}
				fprintf(stderr,"Set duty to %lg (previous sleeplen was %lg)\n",duty, slen);
			}
			else if(scanf("%s",buf) > 0 ){
				if(buf[0] == 'q'){
					fprintf(stderr,"Quitting\n");
					break;
				}
				else{
					fprintf(stderr,"Quit on scanf\n");
					exit(EXIT_FAILURE);
				}
			}
			else{
				fprintf(stderr,"Failed on scanf\n");
				exit(EXIT_FAILURE);
			}
		}
		else{
			fprintf(stderr,"Quit on select\n");
			exit(EXIT_FAILURE);
		}
	}

	exit(EXIT_SUCCESS);
	return (int) s;
}
