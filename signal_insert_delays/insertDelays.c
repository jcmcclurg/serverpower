#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include "insertDelays.h"
#include "commonFunctions.h"
#include "get_children.h"

double period;
double duty;

int limiting_mode;
int process_running;
struct timespec twork;

int* nopeList;
int numNope;
int* children;
int numChildren;

char* progname;

void setup_insertDelays(void){
	/* don't buffer if piped */
	setbuf(stdout, NULL);

	children = NULL;
	numChildren = 0;
	nopeList = NULL;
	numNope = 0;

	process_running = 0;
	limiting_mode = MODE_NO_PID;
	period = INITIAL_PERIOD;
	duty = INITIAL_DUTY;

	init_children();
}

void close_insertDelays(void){
	numChildren = 0;
	children = NULL;
	free(nopeList);
	close_children();
}

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

		if(f > 0)
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
		if(f > 0)
			limiting_mode |= MODE_PID_DIRTY;
	}
}

void update_duty(double d){
	double ontime = d*period;

	twork.tv_sec = (long) ontime;
	twork.tv_nsec = (long) ((ontime - ((double) twork.tv_sec))*1.0e9);
	duty = d;
	#ifdef DEBUG
	fprintf(stderr,"Duty = %f\n",d);
	#endif
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
		if(f > 0)
			limiting_mode |= MODE_PID_DIRTY;
	}
}

void update_period(double p, timer_t timerid){
	struct itimerspec its;
	its.it_value.tv_sec = (long) p;
	its.it_value.tv_nsec = (long) ((p - ((double) its.it_value.tv_sec))*1.0e9);
	its.it_interval.tv_sec = its.it_value.tv_sec;
	its.it_interval.tv_nsec = its.it_value.tv_nsec;
	#ifdef DEBUG
	fprintf(stderr,"Timer vals: %ld, %ld\n", its.it_value.tv_sec, its.it_value.tv_nsec);
	#endif

	if (timer_settime(timerid, 0, &its, NULL) == -1)
		errExit("update period fail");

	period = p;
	#ifdef DEBUG
	fprintf(stderr,"Period = %f\n",p);
	#endif
	update_duty(duty);
}

void stop_timer(timer_t timerid){
	limiting_mode = MODE_NO_PID;

	struct itimerspec its;
	its.it_value.tv_sec = 0;
	its.it_value.tv_nsec = 0;
	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = 0;

	if(timer_settime(timerid, 0, &its, NULL) == -1)
		errExit("stop timer fail");
}

void start_timer(timer_t timerid){
	limiting_mode = MODE_LIMIT;
	update_period(period,timerid);
}

void usage(){
	fprintf(stdout, "\nInsert Delays\n");
	fprintf(stdout, "\nUsage: \n");
	fprintf(stdout, "%s [pid whose children to control] [pid list to exclude from control]\n", progname);
	fprintf(stdout, "\nExample: %s -p 1 -e 1\n", progname);
	fprintf(stdout, "\n");
}

int cmdline(int argc, char **argv){
	int i,j;
	int numParents;
	int* parentList;

	progname = argv[0];

	char p_flag_pos = -1;
	char e_flag_pos = -1;
	j = 0;
	for(i = 1; i < argc; i++){
		if(argv[i][0] == '-'){
			if(argv[i][1] == 'p'){
				p_flag_pos = i;
			}
			else if(argv[i][1] == 'e'){
				e_flag_pos = i;
			}
			j++;
		}
	}

	if(j > 2){
		usage();
		return -1;
	}

	// User does not specify parent pid list. The default is to collect the modifyable processes.
	if((e_flag_pos == 1 && p_flag_pos == -1) || argc == 1){
		process_tree* ptree;
		int i, len;
		update_proc_tree(&ptree, &len);
		for(i = 0; i < len; i++){
			ptree[i].class = NONCHILD_CLASS;
		}
		#ifdef DEBUG
		fprintf(stderr,"Trying to controll all the PIDs.\n");
		#endif
	}
	else{
		if(p_flag_pos == -1){
			if(e_flag_pos == -1){
				numParents = argc - 1;
			}
			else{
				numParents = e_flag_pos - 1;
			}
			p_flag_pos = 0;
		}
		else if(p_flag_pos < e_flag_pos){
			numParents = e_flag_pos - p_flag_pos - 1;
		}
		else if(p_flag_pos == 1){
			numParents = argc - p_flag_pos - 1;
		}
		else{
			usage();
			return -1;
		}

		parentList = (int*) malloc(sizeof(int)*numParents);
		#ifdef DEBUG
		fprintf(stderr,"Parent list (%d): ",numParents);
		#endif
		for(i = 0; i < numParents; i++){
			parentList[i] = atoi(argv[i + p_flag_pos + 1]);
			#ifdef DEBUG
			fprintf(stderr,"%d ",parentList[i]);
			#endif
		}
		#ifdef DEBUG
		fprintf(stderr,"\n");
		#endif
		tag_proc_tree_children(parentList, numParents);
		free(parentList);
	}

	// There are excluded processes
	if(e_flag_pos != -1){
		if(p_flag_pos > e_flag_pos){
			numNope = p_flag_pos - e_flag_pos - 1;
		}
		else if(p_flag_pos == 1 || p_flag_pos == -1){
			numNope = argc - e_flag_pos - 1;
		}
		else{
			usage();
			return -1;
		}

		nopeList = (int*) malloc(sizeof(int)*numNope);
		#ifdef DEBUG
		fprintf(stderr,"Exclusion list (%d): ",numNope);
		#endif
		for(i = 0; i < numNope; i++ ){
			nopeList[i] = atoi(argv[i + e_flag_pos + 1]);
			#ifdef DEBUG
			fprintf(stderr,"%d ",nopeList[i]);
			#endif
		}
		#ifdef DEBUG
		fprintf(stderr,"\n");
		#endif
	}
	update_children(&children, &numChildren, nopeList, numNope, 1);
	#ifdef DEBUG
	fprintf(stderr,"Controllable list: ");
	for(i = 0; i < numChildren; i++){
		fprintf(stderr,"%d ",children[i]);
	}
	fprintf(stderr,"\n");
	#endif

	return 0;
}


int main(int argc, char *argv[]) {
	struct sigevent sev;
	sigset_t mask;
	struct sigaction sa;
	timer_t timerid;

	if(signal(SIGINT,sig_handler) == SIG_ERR)
		errExit("Can't catch SIGINT.");

	setup_insertDelays();

	if(cmdline(argc, argv))
		errExit("Couldn't get set up.");

	/* Establish handler for timer signal */
	#ifdef DEBUG
	fprintf(stderr,"Establishing handler for signal %d\n", SIGRTMIN);
	#endif
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

	#ifdef DEBUG
	fprintf(stderr,"timer ID is 0x%lx\n", (long) timerid);
	#endif

	/* Unlock the timer signal, so that timer notification
	  can be delivered
	*/
	#ifdef DEBUG
	fprintf(stderr,"Unblocking signal %d\n", SIGRTMIN);
	#endif
	if (sigprocmask(SIG_UNBLOCK, &mask, NULL) == -1)
		errExit("sigprocmask");

	start_timer(timerid);
	while(1){
		char* str;
		str = readLine();
		if(str != NULL){
			//else if(str[0] == 't'){
			if(str[0] == 't'){
				double t = atof(str + 1);
				update_period(t, timerid);
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

		// If any of the signals didn't go through, we ought to update the list of children.
		if((limiting_mode & MODE_PID_DIRTY)){
			update_children(&children, &numChildren, nopeList, numNope, 1);
		}
	}

	normExit();
	return 0;
}
