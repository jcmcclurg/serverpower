/*
Written fall 2015 by Josiah McClurg
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

#define CLOCKID CLOCK_REALTIME
#define SLEEPLEN 1000
#define SIG SIGRTMIN
#define errExit(msg)	do { perror(msg); exit(EXIT_FAILURE); } while (0)
#define STATE_RUNNING 1
#define STATE_FINISHED 0

char		 *progname;
const char* version = "1.0";
char *prefix;
char *fname;
FILE *file;
struct timespec start_time;
struct itimerspec its;
timer_t timerid;
char value[128];
char state;
char verbose;

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
	double time;
	int numScanned;
	struct timespec ts;

	if(value[0] != 0){
		if(prefix == NULL)
			fprintf(stdout,"%s\n",value);
		else
			fprintf(stdout,"%s%s\n",prefix,value);

		if(verbose){
			clock_gettime(CLOCKID,&ts);
			fprintf(stderr,"%lf,%s\n",convert_time_to_sec(&ts),value);
		}
	}

	numScanned = fscanf(file,"%lf,%s\n",&time, value);
	if(numScanned == EOF){
		state = STATE_FINISHED;
		exit(EXIT_SUCCESS);
	}
	else if(numScanned != 2){
		errExit("problem parsing file.");
	}

	ts = convert_sec_to_time(time);
	its.it_value.tv_sec = start_time.tv_sec + ts.tv_sec;
	its.it_value.tv_nsec = start_time.tv_nsec + ts.tv_nsec;

	clock_gettime(CLOCKID, &ts);
	if(ts.tv_sec > its.it_value.tv_sec || (ts.tv_sec == its.it_value.tv_sec && ts.tv_nsec > its.it_value.tv_nsec)){
		its.it_value.tv_sec = 0;
		its.it_value.tv_nsec = 1;
		if(verbose)
			fprintf(stderr,"Scheduling (%lf,%s) as soon as possible.\n",time,value);
		timer_settime(timerid, 0, &its, NULL);
	}
	else{
		timer_settime(timerid, TIMER_ABSTIME, &its, NULL);
	}
}

void usage(){
	fprintf(stdout, "\nTimed playback %s\n", version);
	fprintf(stdout, "\nUsage: %s\n",progname);
	fprintf(stdout, "   -f    the csv file to play back. first column is seconds (relative to now). second column is value to play back\n");
	fprintf(stdout, "   -p    prefix before playback\n");
	fprintf(stdout, "   -v    verbose output\n");
	fprintf(stdout, "\n");
	fprintf(stdout, "If the program is unable to schedule at the specified time, it notifies via stderr and schedules as soon as possible. \n");
	fprintf(stdout, "\n");
}


int cmdline(int argc, char **argv){
	progname = (char*) argv[0];
	int i;
	char opt = 0;
	verbose = 0;
	state = STATE_RUNNING;
	value[0] = 0;
	fname = NULL;
	prefix = NULL;

	for(i = 1; i < argc; i++){
		if(argv[i][0] == '-'){
			opt = argv[i][1];
			if(opt == 'v')
				verbose = 1;
			else if(opt == 'h'){
				usage();
				exit(EXIT_SUCCESS);
			}
		}
		else{
			if(opt == 'f'){
				fname = (char *) argv[i];
			}
			else if(opt == 'p'){
				prefix = (char *) argv[i];
			}
			else{
				usage();
				errExit("Invalid arguments.");
			}
			opt = 0;
		}
	}

	if(fname == NULL)
		errExit("No input file.");

	file = fopen(fname,"r");
	if(file == NULL){
		errExit("Can't read input file.");
	}
	return 0;
}

void sigint_handler(int signum){
	if(verbose)
		fprintf(stderr,"Exiting on signal %d\n",signum);

	state = STATE_FINISHED;
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

	if(clock_gettime(CLOCKID, &start_time) == -1)
		errExit("Problem with getting time.");

	/* Start the timer */
	its.it_value.tv_sec = 0;
	its.it_value.tv_nsec = 1;
	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = 0;

	/* Unlock the timer signal, so that timer notification
	  can be delivered
	*/
	if (sigprocmask(SIG_UNBLOCK, &mask, NULL) == -1)
		errExit("sigprocmask");

	if (timer_settime(timerid, 0, &its, NULL) == -1)
		errExit("timer_settime");

	while(state == STATE_RUNNING){
		pause();
	}
	exit(EXIT_SUCCESS);
	return 0;
}
