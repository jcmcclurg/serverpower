/*
Copyright (c) 2012, Intel Corporation

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

	* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
	* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
	* Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* Written by Martin Dimitrov, Carl Strickland
Modified summer 2015 by Josiah McClurg
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>

#define CLOCKID CLOCK_BOOTTIME
#define SLEEPLEN 1000
#define SIG SIGRTMIN
#define errExit(msg)	do { perror(msg); exit(EXIT_FAILURE); } while (0)


char		 *progname;
const char* version = "1.0";
const char* default_fname = "powerDeviation.txt";
char *prefix;
char *fname;
FILE *file;
struct timespec start_time;
struct itimerspec its;
timer_t timerid;
double value;
char value_valid;

// The length of a ring buffer is  (end - start)

struct timespec convert_sec_to_time(double sec){
	struct timespec ts;
	ts.tv_sec = (time_t) sec;
	ts.tv_nsec = (long) ((sec - ((double) ts.tv_sec))*1.0e9);
	return ts;
}

double convert_time_to_sec(struct timespec* tv){
	double elapsed_time = ((double)(tv->tv_sec)) + (((double)(tv->tv_nsec))/1.0e9);
	return elapsed_time;
}

static void handler(int sig, siginfo_t *si, void *uc) {
	double time, newValue;
	int numScanned;
	struct timespec ts;
	if(value_valid){
		if(prefix != NULL)
			fprintf(stdout,"%s%lf\n",prefix,value);
		else
			fprintf(stdout,"%lf\n",value);
	}
	else
		value_valid = 1;

	numScanned = fscanf(file,"%lf,%lf\n",&time, &newValue);
	if(numScanned == EOF){
		exit(EXIT_SUCCESS);
	}
	else if(numScanned != 2){
		errExit("problem parsing file.");
	}

	value = newValue;
	ts = convert_sec_to_time(time);
	its.it_value.tv_sec = start_time.tv_sec + ts.tv_sec;
	its.it_value.tv_nsec = start_time.tv_nsec + ts.tv_nsec;

	clock_gettime(CLOCKID, &ts);
	if(ts.tv_sec > its.it_value.tv_sec || (ts.tv_sec == its.it_value.tv_sec && ts.tv_nsec > its.it_value.tv_nsec)){
		its.it_value.tv_sec = 0;
		its.it_value.tv_nsec = 1;
		fprintf(stderr,"Scheduling %lf as soon as possible.\n",time);
		timer_settime(timerid, 0, &its, NULL);
	}
	else{
		timer_settime(timerid, TIMER_ABSTIME, &its, NULL);
	}
}

void usage(){
	fprintf(stdout, "\nTimed playback %s\n", version);
	fprintf(stdout, "\nUsage: \n");
	fprintf(stdout, "%s   [-f [filename: (%s by default)]]\n", progname, default_fname);
	fprintf(stdout, "\n");
	fprintf(stdout, "If the program is unable to schedule at the specified time, it notifies via stderr and schedules as soon as possible.\n");
	fprintf(stdout, "\n");
}


int cmdline(int argc, char **argv){
	progname = (char*) argv[0];
	int i;
	char opt = 0;
	char fname_set = 0;
	char prefix_set = 0;

	for(i = 1; i < argc; i++){
		if(argv[i][0] == '-' && i+1 < argc){
			opt = argv[i][1];
		}
		else{
			if(opt == 'f' && !fname_set){
				fname = (char *) argv[i];
				fname_set = 1;
			}
			else if(opt == 'p' && !prefix_set){
				prefix = (char*) argv[i];
				prefix_set = 1;
			}
			else if(i == 1 && argc == 2){
				fname = (char*) argv[i];
			}
			else{
				usage();
				return -1;
			}
			opt = 0;
		}
	}

	if(!prefix_set){
		prefix = NULL;
	}

	if(!fname_set){
		fname = (char*) default_fname;
	}

	file = fopen(fname,"r");
	if(file == NULL){
		fprintf(stderr, "Can't read from %s\n",fname);
		return -1;
	}

	return 0;
}

void sigint_handler(int signum){
	fprintf(stderr,"Exiting on signal %d\n",signum);
	exit(EXIT_SUCCESS);
}

int main(int argc, char **argv){
	/* Clean up if we're told to exit */
	signal(SIGINT, sigint_handler);

	if(cmdline(argc, argv)) {
		exit(EXIT_FAILURE);
	}
	struct sigevent sev;
	sigset_t mask;
	struct sigaction sa;

	/* don't buffer if piped */
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);

	/* Establish handler for timer signal */
	fprintf(stderr,"Establishing handler for signal %d\n", SIG);
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = handler;
	sigemptyset(&sa.sa_mask);
	if (sigaction(SIG, &sa, NULL) == -1)
		errExit("sigaction");

	/* Create the timer */

	sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_signo = SIG;
	sev.sigev_value.sival_ptr = &timerid;
	if (timer_create(CLOCKID, &sev, &timerid) == -1)
		errExit("timer_create");

	fprintf(stderr,"timer ID is 0x%lx\n", (long) timerid);
	if(clock_gettime(CLOCKID, &start_time) == -1)
		errExit("Problem with getting time.");

	/* Start the timer */
	value_valid = 0;
	its.it_value.tv_sec = 0;
	its.it_value.tv_nsec = 1;
	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = 0;

	/* Unlock the timer signal, so that timer notification
	  can be delivered
	*/
	fprintf(stderr,"Unblocking signal %d\n", SIG);
	if (sigprocmask(SIG_UNBLOCK, &mask, NULL) == -1)
		errExit("sigprocmask");

	if (timer_settime(timerid, 0, &its, NULL) == -1)
		errExit("timer_settime");

	while(1){
		sleep(SLEEPLEN);
	}
	exit(EXIT_SUCCESS);
	return 0;
}
