#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include "trie.h"
#include "get_children.h"
#include "cmdline.h"

// Used for exiting
#define errExit(msg)	do { close_insertDelays(); perror(msg); exit(EXIT_FAILURE); } while (0)
#define normExit()	do { close_insertDelays(); exit(EXIT_SUCCESS); } while (0)

#define DEFAULT_MAX_DUTY 0.999
#define DEFAULT_MIN_DUTY 0.001
#define DEFAULT_UPDATE_INTERVAL 10.0
#define DEFAULT_WORK_INTERVAL 0.1
#define DEFAULT_DUTY max_duty

double update_interval;
double work_interval;
double max_duty;
double min_duty;
double duty;
FILE* fp;

char verbose;
char update_on_error;
pid_t my_pid;

proc_tree_info* ptree_info;
trie_root* proc_tree;
char* progname;

void close_insertDelays(void){
	if(proc_tree != NULL){
		int f = 0;
		if(verbose)
			fprintf(stderr,"Restarting PIDs: ");

		if(ptree_info->stoppableList != NULL){
			int i = 0;
			while(ptree_info->stoppableList[i] > 0){
				LOG("%d ",ptree_info->stoppableList[i]);
				f -= kill(ptree_info->stoppableList[i++],SIGCONT);
			}
		}
		LOG("\n");

		if(f > 0)
			errExit("Couldn't send SIGCONT");

		trie_deallocate(proc_tree);
	}
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

// Returns the number of failures between telling the current PID list to start and to stop.
int do_work(void){
	// Allow the other processes to do work.
	int f = 0;
	int i = 0;
	if(ptree_info->stoppableList != NULL){
		while(ptree_info->stoppableList[i] > 0){
			f -= kill(ptree_info->stoppableList[i++],SIGCONT);
		}
	}

	// Wait for them to do it.
	usleep((useconds_t)(work_interval*1.0e6));

	// Tell the other process to stop working.
	i = 0;
	if(ptree_info->stoppableList != NULL){
		while(ptree_info->stoppableList[i] > 0){
			f -= kill(ptree_info->stoppableList[i++],SIGSTOP);
		}
	}
	return f;
}

int main(int argc, char *argv[]) {
	useconds_t sleeplen;
	int num_readable;
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

	if(cmdline(argc,argv) == NULL){
		errExit("Invalid command line arguments.");
	}

	// TODO: update fields from structure that cmdline returns.

	if(clock_gettime(CLOCK_BOOTTIME, &currentTime)){
		errExit("Problem with gettime.");
	}
	slen = 0;
	sleeplen = 0;

	prevTime_sec = ((double)(currentTime.tv_sec)) + (((double)(currentTime.tv_nsec))/1.0e9);
	time_last_updated = prevTime_sec;

	while(1) {
		FD_ZERO(&readfds);
		FD_SET(fd_stdin, &readfds);
		num_readable = select(fd_stdin +1, &readfds, NULL, NULL, &tv);

		// If the user has not typed something, do some work.
		if (num_readable == 0) {
			int errs = do_work();
			if(errs){
				fprintf(stderr,"There were %d errors\n",errs);
			}

			if(clock_gettime(CLOCK_BOOTTIME, &currentTime)){
				errExit("Problem with gettime");
			}
			currentTime_sec = ((double)(currentTime.tv_sec)) + (((double)(currentTime.tv_nsec))/1.0e9);

			// If the PID list hasn't been updated in a while, update it.
			if(currentTime_sec - time_last_updated > update_interval || (errs && update_on_error)){
				// Update the PID list.
				if(update_proc_tree(proc_tree)){
					errExit("Couldn't update process tree.");
				}
				else if(verbose){
					LOG("Updated process tree. ");
					LOG("Stoppable list: ");
					if(ptree_info->stoppableList != NULL){
						int i = 0;
						while(ptree_info->stoppableList[i] > 0){
							LOG("%d ",ptree_info->stoppableList[i++]);
						}
					}
					LOG("\n");
				}

				if(clock_gettime(CLOCK_BOOTTIME, &currentTime)){
					errExit("Problem with gettime");
				}
				time_last_updated = ((double)(currentTime.tv_sec)) + (((double)(currentTime.tv_nsec))/1.0e9);
				//time_last_updated = currentTime_sec;
			}
			//slen = (1.0-duty)*(currentTime_sec - prevTime_sec);
			//slen = (1.0-duty)*0.1;
			//slen = (1.0-duty)*work_interval*10.0;
			slen = work_interval*(1.0/duty-1.0);			
			sleeplen = (useconds_t)(slen*1.0e6);
			//if(verbose)
			//	fprintf(stderr,"Sleeping for %g seconds\n",slen);
			usleep(sleeplen);
			prevTime_sec = currentTime_sec;
		}

		// If the user has typed something, process what the user has typed.
		else if (num_readable == 1) {
			if(scanf("%lf",&duty) > 0){
				if(duty > max_duty){
					duty = max_duty;
				}
				else if(duty < min_duty){
					duty = min_duty;
				}
				LOG("Set duty to %lg (previous sleeplen was %lg)\n",duty, slen);
			}
			else if(scanf("%s",buf) > 0 ){
				if(buf[0] == 'q'){
					LOG("Quitting\n");
					break;
				}
				else if(buf[0] == 'w'){
					if(sscanf(buf+1,"%lf",&work_interval) > 0){
						LOG("Set work interval to %lf\n", work_interval);
					}
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
