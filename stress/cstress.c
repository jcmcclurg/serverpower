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
#define DEFAULT_NUM_ITERATIONS ((long) 2000000)
#define DEFAULT_DUTY ((double) MAXDUTY)

double duty;
long num_iterations;
char verbose;
char* cmd_name;

void usage(void){
	fprintf(stdout,"%s -h   print help\n",cmd_name);
	fprintf(stdout,"   -n   [number of iterations (default: %ld]\n", DEFAULT_NUM_ITERATIONS);
	fprintf(stdout,"   -v   verbose\n");
	fprintf(stdout,"   -d   [initial duty (default: %lf)]\n", DEFAULT_DUTY);
}

int cmdline(int argc, char** argv){
	num_iterations = -1;
	int i;
	char opt = 0;
	num_iterations = DEFAULT_NUM_ITERATIONS;
	duty = DEFAULT_DUTY;
	verbose = 0;
	cmd_name = (char*) argv[0];

	if(argc == 2 && argv[1][0] != '-'){
		num_iterations = atol(argv[1]);
	}
	else{
		for(i = 1; i < argc; i++){
			if(argv[i][0] == '-'){
				opt = argv[i][1];
				if(opt == 'h'){
					usage();
					return -1;
				}
				else if(opt == 'v'){
					verbose = 1;
				}
				else if(opt == 'n' && i+1 < argc){
					num_iterations = atol(argv[i+1]);
				}
				else if(opt == 'd' && i+1 < argc){
					duty = atof(argv[i+1]);
				}
				else{
					usage();
					return -1;
				}
			}
			else if(opt == 0){
				usage();
				return -1;
			}
		}
	}
	
	return 0;
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
	char buf[128];
	fd_set readfds;
	double prevTime_sec, currentTime_sec;
	struct timespec currentTime;
	long num_repeats;

	// Clear output buffering on STDOUT
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);

	if(cmdline(argc,argv)){
		exit(EXIT_FAILURE);
	}

	if(verbose) fprintf(stderr,"Set duty to %lf\n",duty);
	slen = 0;
	sleeplen = 0;
	if(clock_gettime(CLOCK_BOOTTIME, &currentTime)){
		fprintf(stderr,"Problem with gettime\n");
		exit(EXIT_FAILURE);
	}
	prevTime_sec = ((double)(currentTime.tv_sec)) + (((double)(currentTime.tv_nsec))/1.0e9);

	num_repeats = 0;
	while(1) {
		FD_ZERO(&readfds);
		FD_SET(fd_stdin, &readfds);
		num_readable = select(fd_stdin +1, &readfds, NULL, NULL, &tv);
		if (num_readable == 0) {
			for(i = 0; i < num_iterations; i++){
				s = sqrt((double) i);
			}
			num_repeats++;
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
				if(verbose) fprintf(stderr,"Set duty to %lg (previous sleeplen was %lg)\n",duty, slen);
			}
			else if(scanf("%s",buf) > 0 ){
				if(buf[0] == 'q'){
					if(verbose) fprintf(stderr,"Quitting\n");
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
	fprintf(stdout,"Ran %ld iterations of %ld sqrt operations.\n",num_repeats,num_iterations);

	exit(EXIT_SUCCESS);
	return (int) s;
}
