#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include "commonFunctions.h"
#include "get_children.h"

#define errExit(msg)	do { close_children(); perror(msg); exit(EXIT_FAILURE); } while (0)
#define normExit()	do { close_children(); exit(EXIT_SUCCESS); } while (0)

#define INITIAL_PERIOD 0.1
#define INITIAL_DUTY 0.2

#define MODE_NO_PID 0
#define MODE_LIMIT 1
#define MODE_PID_DIRTY 2

#define MAX_DUTY 0.99
#define MIN_DUTY 0.01

double period;
double duty;

int pid;
int limiting_mode;
int process_running;
struct timespec twork;
timer_t timerid;
int* children;
int numChildren;

void sig_handler(int signo){
	if(signo == SIGINT){
		printf("\rreceived SIGINT\n");

		int f = 0;
		if(!process_running && (limiting_mode & MODE_LIMIT)){
			int i;
			for(i = 0; i < numChildren; i++){
				f += kill(children[i],SIGCONT);
			}
			process_running = 1;
		}

		if(f)
			errExit("Couldn't send SIGCONT");

		normExit();
	}
	else{
		errExit("Caught weird signal");
	}
}

static void handler(int sig, siginfo_t *si, void *uc) {
	if(!process_running && (limiting_mode & MODE_LIMIT)){
		int i;
		int f = 0;
		for(i = 0; i < numChildren; i++){
			f += kill(children[i],SIGCONT);
		}
		process_running = 1;
		if(f)
			limiting_mode |= MODE_PID_DIRTY;
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
	if(process_running && (limiting_mode & MODE_LIMIT)){
		nanosleep(&twork, NULL);
		int i;
		int f = 0;
		for(i = 0; i < numChildren; i++){
			f += kill(children[i],SIGSTOP);
		}
		process_running = 0;
		if(f)
			limiting_mode |= MODE_PID_DIRTY;
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
		errExit("update period fail");

	period = p;
	fprintf(stderr,"Period = %f\n",p);
	update_duty(duty);
}

void stop_timer(void){
	limiting_mode = MODE_NO_PID;

	struct itimerspec its;
	its.it_value.tv_sec = 0;
	its.it_value.tv_nsec = 0;
	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = 0;

	if (timer_settime(timerid, 0, &its, NULL) == -1)
		errExit("stop timer fail");
}

void start_timer(void){
	limiting_mode = MODE_LIMIT;
	update_period(period);
}

int update_children(int p){
	int i;
	numChildren = get_children(&children, p);
	if(numChildren > 0){
		limiting_mode = MODE_LIMIT;
		fprintf(stderr,"   => {");
		for(i = 0; i < numChildren-1; i++){
			fprintf(stderr,"%d,", children[i]);
		}
		fprintf(stderr,"%d", children[numChildren - 1]);
		fprintf(stderr,"}\n");
		return 0;
	}
	else{
		fprintf(stderr,"Can't get the children of %d anymore.\n",pid);
		return -1;
	}
}

int set_pid(int p){
	process_running = 1;
	fprintf(stderr,"Target PID = %d\n",pid);
	stop_timer();
	if(!update_children(p)){
		pid = p;
		start_timer();
	}
	else{
		process_running = 0;
	}
	return pid;
}

int main(int argc, char *argv[]) {
	struct sigevent sev;
	sigset_t mask;
	struct sigaction sa;
	process_running = 0;
	pid = -1;
	limiting_mode = MODE_NO_PID;
	period = INITIAL_PERIOD;
	duty = INITIAL_DUTY;

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

	/* Unlock the timer signal, so that timer notification
	  can be delivered
	*/
	fprintf(stderr,"Unblocking signal %d\n", SIGRTMIN);
	if (sigprocmask(SIG_UNBLOCK, &mask, NULL) == -1)
		errExit("sigprocmask");

	fprintf(stderr,"My PID is %d\n",getpid());

	int p = -1;
	init_children();

	if(argc > 1){
		p = set_pid(atoi(argv[1]));
		fprintf(stderr,"I am controlling PID %d\n",pid);
	}

	while(1){
		char* str;
		str = readLine();
		if(str != NULL){
			//else if(str[0] == 't'){
			if(str[0] == 't'){
				double t = atof(str + 1);
				update_period(t);
			}
			else if(str[0] == 'p'){
				p = set_pid(atoi(str + 1));
			}
			else if(str[0] == 'q'){
				normExit();
			}
			else{
				if(str[0] == 'd'){
					str++;
				}

				double d = atof(str);
				if(d > MAX_DUTY)
					d = MAX_DUTY;
				else if(d < MIN_DUTY)
					d = MIN_DUTY;

				update_duty(d);
			}
		}

		do_work();
		if((limiting_mode & MODE_PID_DIRTY)){
			update_children(p);
		}
	}

	normExit();
	return 0;
}
