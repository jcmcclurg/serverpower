#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include "commonFunctions.h"

#define errExit(msg)	do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define INITIAL_PERIOD 1.0
#define INITIAL_DUTY 0.5

double period;
double duty;

int pid;
int process_running;
struct timespec twork;
timer_t timerid;

void sig_handler(int signo){
	if(signo == SIGINT){
		printf("received SIGINT\n");

		if(!process_running && pid != -1){
			if(kill(pid,SIGCONT))
				errExit("Couldn't send SIGCONT.");
			process_running = 1;
		}

		exit(EXIT_SUCCESS);
	}
	else{
		exit(EXIT_FAILURE);
	}
}

static void handler(int sig, siginfo_t *si, void *uc) {
	if(!process_running && pid != -1){
		if(kill(pid,SIGCONT))
			errExit("Couldn't send SIGCONT.");
		process_running = 1;
	}
}

void update_duty(double d){
	double ontime = d*period;

	twork.tv_sec = (long) ontime;
	twork.tv_nsec = (long) ((ontime - ((double) twork.tv_sec))*1.0e9);
	duty = d;
	fprintf(stderr,"Duty = %f\n",d);
}

void do_work(void){
	if(process_running && pid != -1){
		nanosleep(&twork, NULL);
		if(kill(pid,SIGSTOP))
			errExit("Couldn't send SIGSTOP.");
		process_running = 0;
	}
}

void update_period(double p){
	struct itimerspec its;
	its.it_value.tv_sec = (long) p;
	its.it_value.tv_nsec = (long) ((p - ((double) its.it_value.tv_sec))*1.0e9);
	its.it_interval.tv_sec = its.it_value.tv_sec;
	its.it_interval.tv_nsec = its.it_value.tv_nsec;
	//fprintf(stdout,"%ld, %ld\n", its.it_value.tv_sec, its.it_value.tv_nsec);

	if (timer_settime(timerid, 0, &its, NULL) == -1)
		errExit("timer_settime");

	period = p;
	fprintf(stderr,"Period = %f\n",p);
	update_duty(duty);
}

void main(void) {
	struct sigevent sev;
	sigset_t mask;
	struct sigaction sa;
	process_running = 0;
	pid = -1;
	struct timespec twork;

	if(signal(SIGINT,sig_handler) == SIG_ERR)
		errExit("Can't catch SIGINT.");

	/* don't buffer if piped */
	setbuf(stdout, NULL);

	/* Establish handler for timer signal */
	fprintf(stderr,"Establishing handler for signal %d\n", SIGRTMIN);
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = handler;
	sigemptyset(&sa.sa_mask);
	if (sigaction(SIGRTMIN, &sa, NULL) == -1)
		errExit("sigaction");

	/* Create the timer */
	sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_signo = SIGRTMIN;
	sev.sigev_value.sival_ptr = &timerid;
	if (timer_create(CLOCK_BOOTTIME, &sev, &timerid) == -1)
		errExit("timer_create");

	fprintf(stderr,"timer ID is 0x%lx\n", (long) timerid);

	update_period(INITIAL_PERIOD);
	update_duty(INITIAL_DUTY);

	/* Unlock the timer signal, so that timer notification
	  can be delivered
	*/
	fprintf(stderr,"Unblocking signal %d\n", SIGRTMIN);
	if (sigprocmask(SIG_UNBLOCK, &mask, NULL) == -1)
		errExit("sigprocmask");

	fprintf(stderr,"PID: %d\n",getpid());
	// A long long wait so that we can easily issue a signal to this process
	while(1){
		char* str;
		str = readLine();
		if(str != NULL){
			if(str[0] == 'd'){
				double d = atof(str + 1);
				update_duty(d);
			}
			else if(str[0] == 't'){
				double t = atof(str + 1);
				update_period(t);
			}
			else if(str[0] == 'p'){
				pid = atoi(str + 1);
				fprintf(stderr,"Target PID = %d\n",pid);
			}
		}
		do_work();
	}

	exit(EXIT_SUCCESS);
}
