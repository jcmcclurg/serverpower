#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include "insertDelays.h"
#include "commonFunctions.h"
#include "get_children.h"

#define MAX_DUTY 0.999
#define MIN_DUTY 0.001
#define DEFAULT_UPDATE_INTERVAL 1.0
#define DEFAULT_WORK_INTERVAL 0.1
#define DEFAULT_DUTY MAX_DUTY

double update_interval;
double work_interval;
double duty;
char verbose;
int* parentList;
char explicitParentList;
int* exclusionList;
char explicitExclusionList;

// Contains the list of processes specified by parentList and exclusionList
int* currentPIDList;

char* progname;

void close_insertDelays(void){
	int f = 0;
	if(verbose)
		fprintf(stderr,"Closing PIDs: ")
	if(currentPIDList != NULL){
		int i;
		while(currentPIDList[i] > 0){
			if(verbose)
				fprintf(stderr,"%d ",currentPIDList[i])
			f -= kill(currentPIDList[i++],SIGCONT);
		}
		if(verbose)
			fprintf(stderr,"\n")
	}

	if(f > 0)
		errExit("Couldn't send SIGCONT");

	free(parentList);
	free(exclusionList);
	free(currentPIDList);
	close_pstree();
}

void sig_handler(int signo){
	if(signo == SIGINT){
		printf("\rreceived SIGINT\n");
		normExit();
	}
	else{
		errExit("Caught weird signal");
	}
}

void usage(){
	fprintf(stdout, "\nInsert Delays\n");
	fprintf(stdout, "\nUsage: \n");
	fprintf(stdout, "%s   -p   [list of PIDs to control (default: all that this user has permissions to)]\n", progname);
	fprintf(stdout, "     -o   Only control PIDs specified at the beginning. Leave their children alone. Does not require -p.\n");
	fprintf(stdout, "     -e   [list of PIDs to exclude (default: none)]\n");
	fprintf(stdout, "     -x   Only exclude specified PIDs. Allow their children to potentially be controlled. Requires -e.\n");
	fprintf(stdout, "     -d   [Initial duty (default: %lf)]\n", DEFAULT_DUTY);
	fprintf(stdout, "     -u   [Process tree update interval in seconds (default: %lf)]\n", DEFAULT_UPDATE_INTERVAL);
	fprintf(stdout, "     -w   [Work interval in seconds (default: %lf)]\n", DEFAULT_WORK_INTERVAL);
	fprintf(stdout, "     -v   Verbose mode\n");
	fprintf(stdout, "\n");
}

int cmdline(int argc, char **argv){
	progname = argv[0];

	int i,j;
	char p_flag_pos = -1;
	int parentListLen = -1;
	char e_flag_pos = -1;
	int exclusionListLen = -1;

	update_interval = DEFAULT_UPDATE_INTERVAL;
	work_interval = DEFAULT_WORK_INTERVAL;
	duty = DEFAULT_DUTY;
	verbose = 0;
	explicitParentList = 0;
	explicitExclusionList = 0;
	parentList = NULL;
	exclusionList = NULL;
	
	char opt = 0;
	j = 0;
	for(i = 1; i < argc; i++){
		if(argv[i][0] == '-'){
			opt = argv[i][1];

			if(p_flag_pos > 0){
				parentListLen = i - p_flag_pos - 1;
			}
			if(e_flag_pos > 0){
				exclusionListLen = i - e_flag_pos - 1;
			}

			if(opt == 'p'){
				p_flag_pos = i;
			}
			else if(opt == 'o'){
				explicitParentList = 1;
			}
			else if(opt == 'e'){
				e_flag_pos = i;
			}
			else if(opt = 'x'){
				explicitExclusionList = 1;
			}
			else if(opt == 'v'){
				verbose = 1;
			}
			else{
				usage();
				return -1;
			}
			j++;
		}
	}

	if(e_flags_pos > 0){
		if(exclusionListLen < 0){
			exclusionListLen = i - e_flags_pos - 1;
		}

		if(exclusionListLen == 0){
			usage();
			return -1;
		}
	}
	if(p_flags_pos > 0){
		if(parentListLen < 0){
			parentListLen = i - p_flags_pos - 1;
		}

		if(parentListLen == 0){
			usage();
			return -1;
		}
	}

	if(e_flags_pos < 0 && explicitExclusionList){
		fprintf(stderr,"You have to specify an exclusion list to use the -x option.\n");
		usage();
		return -1;
	}


	if(e_flags_pos > 0){
		exclusionList = (int*) malloc(sizeof(int)*exclusionListLen);
		if(verbose)
			fprintf(stderr,"Exclusion list (%d): ",exclusionListLen);
		for(i = 0; i < exclusionListLen; i++){
			exclusionList[i] = atoi(argv[i + e_flag_pos + 1]);
			if(verbose)
				fprintf(stderr,"%d ",exclusionList[i]);
		}
		if(verbose)
			fprintf(stderr,"\n");
	}

	if(p_flags_pos > 0){
		parentList = (int*) malloc(sizeof(int)*parentListLen);
		if(verbose)
			fprintf(stderr,"Parent list (%d): ",parentListLen);
		for(i = 0; i < parentListLen; i++){
			parentList[i] = atoi(argv[i + p_flag_pos + 1]);
			if(verbose)
				fprintf(stderr,"%d ",parentList[i]);
		}
		if(verbose)
			fprintf(stderr,"\n");
	}
	else if(explicitParentList){
		int* plist;
		int plistLen;
		if(pstree_update(NULL, NULL) || pstree_mark_parents_all() || pstree_prune_unmodifyable() ){
			perror("Problem with getting the modifyable processes.");
			exit(EXIT_FAILURE);
		}
		if(get_modifyable_processes(&plist, &plistLen)){
		}

		parentList = (int*) malloc(sizeof(int)*plistLen);
		memcpy(parentList, plist, sizeof(int)*plistLen);
	}

	return 0;
}

// Returns the number of failures between telling the current PID list to start and to stop.
int do_work(){
	// Allow the other processes to do work.
	int f = 0;
	if(currentPIDList != NULL){
		while(currentPIDList[i] > 0){
			f -= kill(currentPIDList[i++],SIGCONT);
		}
	}

	// Wait for them to do it.
	usleep((useconds_t)(work_interval*1.0e6));

	// Tell the other process to stop working.
	if(currentPIDList != NULL){
		while(currentPIDList[i] > 0){
			f -= kill(currentPIDList[i++],SIGSTOP);
		}
	}
	return f;
}

int main(int argc, char *argv[]) {
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
	double prevTime_sec, currentTime_sec, time_last_updated;
	struct timespec currentTime;

	// Clear output buffering on STDOUT
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);

	if(signal(SIGINT,sig_handler) == SIG_ERR)
		errExit("Can't catch SIGINT.");

	if(cmdline(argc,argv)){
		errExit("Invalid command line arguments.");
	}

	if(init_pstree()){
		errExit("Couldn't initialize pstree interface.");
	}

	if(verbose) fprintf(stderr,"Set duty to %lf\n",duty);
	slen = 0;
	sleeplen = 0;
	if(clock_gettime(CLOCK_BOOTTIME, &currentTime)){
		fprintf(stderr,"Problem with gettime\n");
		exit(EXIT_FAILURE);
	}
	prevTime_sec = ((double)(currentTime.tv_sec)) + (((double)(currentTime.tv_nsec))/1.0e9);

	// LEFT OFF HERE
	time_last_updated = prevTime_sec;

	while(1) {
		FD_ZERO(&readfds);
		FD_SET(fd_stdin, &readfds);
		num_readable = select(fd_stdin +1, &readfds, NULL, NULL, &tv);

		// If the user has not typed something, do some work.
		if (num_readable == 0) {
			int errs = do_work();
			if(errs && verbose)
					fprintf(stderr,"There were %d errors\n",errs)

			if(clock_gettime(CLOCK_BOOTTIME, &currentTime)){
				errExit("Problem with gettime");
			}
			currentTime_sec = ((double)(currentTime.tv_sec)) + (((double)(currentTime.tv_nsec))/1.0e9);
			slen = (1.0-duty)*(currentTime_sec - prevTime_sec);
			sleeplen = (useconds_t)(slen*1.0e6);
			usleep(sleeplen);
			prevTime_sec = currentTime_sec;
		}

		// If the user has typed something, process what the user has typed.
		else if (num_readable == 1) {
			if(scanf("%lf",&duty) > 0){
				if(duty > MAXDUTY){
					duty = MAXDUTY;
				}
				else if(duty < MINDUTY){
					duty = MINDUTY;
				}
				if(verbose)
					fprintf(stderr,"Set duty to %lg (previous sleeplen was %lg)\n",duty, slen);
			}
			else if(scanf("%s",buf) > 0 ){
				if(buf[0] == 'q'){
					if(verbose)
						fprintf(stderr,"Quitting\n");
					break;
				}
				else{
					errExit("Quit on scanf");
				}
			}
			else{
				errExit("Failed on scanf");
			}
		}
		else{
			errExit("Quit on select");
		}
	}

	normExit();
	return 0;
}
