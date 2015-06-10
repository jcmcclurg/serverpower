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

int my_pid;
int my_ppid;
int limiting_mode;
int process_running;
struct timespec twork;

int* children;
int childrenLen;
int numChildren;

int* parentList;
int parentListLen;
int numParents;

int* nopeList;
int nopeListLen;
int numNope;

char* progname;

void setup_insertDelays(void){
	/* don't buffer if piped */
	setbuf(stdout, NULL);

	parentList = NULL;
	nopeList = NULL;
	children = NULL;
	parentListLen = 0;
	childrenLen = 0;
	nopeListLen = 0;
	numNope = 0;
	numChildren = 0;
	numParents = 0;

	process_running = 0;
	pid = -1;
	limiting_mode = MODE_NO_PID;
	period = INITIAL_PERIOD;
	duty = INITIAL_DUTY;

	init_children();
}

void close_insertDelays(void){
	free(parentList);
	free(nopeList);
	free(children);

	parentListLen = 0;
	childrenLen = 0;
	nopeListLen = 0;
	numNope = 0;
	numChildren = 0;
	numParents = 0;

	close_children();
}

void sig_handler(int signo){
	if(signo == SIGINT){
		printf("\rreceived SIGINT\n");

		int f = 0;
		if(!process_running && (limiting_mode & MODE_LIMIT)){
			int i;
			for(i = 0; i < numChildren; i++){
				if(children[i] > 0)
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
			if(children[i] > 0)
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
			if(children[i] > 0)
				f += kill(children[i],SIGSTOP);
		}
		process_running = 0;
		if(f)
			limiting_mode |= MODE_PID_DIRTY;
	}
}

void update_period(double p, timer_t timerid){
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

void stop_timer(timer_t timerid){
	limiting_mode = MODE_NO_PID;

	struct itimerspec its;
	its.it_value.tv_sec = 0;
	its.it_value.tv_nsec = 0;
	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = 0;

	if (timer_settime(timerid, 0, &its, NULL) == -1)
		errExit("stop timer fail");
}

void start_timer(timer_t timerid){
	limiting_mode = MODE_LIMIT;
	update_period(period,timerid);
}

int update_children(int p){
	int i,j,k;
	fprintf(stderr,"Hi\n");
	numChildren = get_children(&children, NULL, NULL, NULL, p);
	fprintf(stderr,"Hi\n");
	if(numChildren > 0){
		limiting_mode = MODE_LIMIT;

		// Remove the children from the nope list.
		fprintf(stderr,"  %d exclusions:\n",nopeListLen);
		for(i = 0; i < numChildren; i++){
			for(j = 0; j < nopeListLen; j++){
				if(children[i] == nopeList[j]){
					// Shift everything to remove the child.
					fprintf(stderr,"  excluding %d\n",children[i]);
					for(k = i+1; k < numChildren; k++){
						children[k-1] = children[k];
					}
					numChildren--;
				}
			}
		}
		
		// Print out the children list.
		fprintf(stderr,"   => {");
		for(i = 0; i < numChildren-1; i++){
			fprintf(stderr,"%d ", children[i]);
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

int set_pid(int p,timer_t timerid){
	process_running = 1;
	fprintf(stderr,"Hi\n");
	if(timerid)
		stop_timer(timerid);
	fprintf(stderr,"Hi\n");
	if(!update_children(p)){
		pid = p;
		if(timerid)
			start_timer(timerid);
		fprintf(stderr,"Target PID = %d\n",p);
	}
	else{
		process_running = 0;
		fprintf(stderr,"Target PID = %d\n",pid);
		return -1;
	}
	return 0;
}

void usage(){
	fprintf(stdout, "\nInsert Delays\n");
	fprintf(stdout, "\nUsage: \n");
	fprintf(stdout, "%s [pid whose children to control] [pid list to exclude from control]\n", progname);
	fprintf(stdout, "\nExample: %s -p 1 -e 1\n", progname);
	fprintf(stdout, "\n");
}


int set_default_parents(void){
	int i,j;
	char c;
	process_tree* ptree;
	int numProcs;

	// Run get children on a known root, so we can populate ptree
	if(get_children(NULL, NULL, &ptree, &numProcs, 1)){
		fprintf(stderr, "Couldn't get children of init.\n");
		return -1;
	}

	if(numProcs > parentListLen){
		free(parentList);
		parentList = (int*) malloc(sizeof(int)*numProcs);
	}
	
	numParents = 0;
	for(i = 0; i < numProcs; i++){
		if( ( j = get_stoppable_status(ptree[i].pid)) == STOPPABLE)
			parentList[numParents++] = ptree[i].pid;
		else if(j == DOES_NOT_CONTINUE || j == INSUFFICIENT_PERMISSIONS_CONT){
			fprintf(stderr, "Process %d didn't start back up again.\n",ptree[i].pid);
			return -1;
		}
	}
	fprintf(stderr, "Found %d restartable processes.\n",numParents);

	prune_parents_list();
	return 0;
}

int cmdline(int argc, char **argv){
	int i,j,k,l;
	progname = argv[0];
	my_ppid = getppid();
	my_pid = getpid();
	fprintf(stderr,"My PID is %d\n",my_pid);
	fprintf(stderr,"My PPID is %d\n",my_ppid);

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
		if(set_default_parents()){
			fprintf(stderr,"Couldn't get the default parent list");
			return -1;
		}
	}
	else if(p_flag_pos == -1){
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
	else if(p_flag_pos == 1)
		numParents = argc - p_flag_pos + 1;
	}
	else{
		usage();
		return -1;
	}

	if(numParents > parentListLen){
		free(parentList);
		parentList = (int*) malloc(sizeof(int)*numParents);
	}

	for(i = 0; i <= numParents; i++){
		parentList[i] = atoi(argv[i + p_flag_pos + 1]);
	}
	prune_parents_list();

	// There are excluded processes
	if(e_flag_pos != -1){
		if(p_flag_pos > e_flag_pos){
			numNope = p_flag_pos - e_flag_pos + 1;
		}
		else if(p_flag_pos == 1 || p_flag_pos == -1)
			numNope = argc - e_flag_pos + 1;
		}
		else{
			usage();
			return -1;
		}

		if(numNope > nopeListLen){
			free(nopeList);
			nopeList = (int*) malloc(sizeof(int)*numNope);
		}

		for(i = 0; i < numNope; i++ ){
			nopeList[i] = atoi(argv[i + e_flag_pos + 1]);
		}
	}

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

	normExit();

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

	while(1){
		char* str;
		str = readLine();
		if(str != NULL){
			//else if(str[0] == 't'){
			if(str[0] == 't'){
				double t = atof(str + 1);
				update_period(t, timerid);
			}
			else if(str[0] == 'p'){
				if(set_pid(atoi(str + 1), timerid))
					errExit("set_pid");
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
			update_children(pid);
		}
	}

	normExit();
	return 0;
}
