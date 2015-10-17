#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include "trie.h"
#include "get_children.h"

// Used for exiting
#define errExit(msg)	do { close_insertDelays(); perror(msg); exit(EXIT_FAILURE); } while (0)
#define normExit()	do { close_insertDelays(); exit(EXIT_SUCCESS); } while (0)

#define MAX_DUTY 0.999
#define MIN_DUTY 0.001
#define DEFAULT_UPDATE_INTERVAL 10.0
#define DEFAULT_WORK_INTERVAL 0.1
#define DEFAULT_DUTY MAX_DUTY

double update_interval;
double work_interval;
double duty;
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
				if(verbose)
					fprintf(stderr,"%d ",ptree_info->stoppableList[i]);
				f -= kill(ptree_info->stoppableList[i++],SIGCONT);
			}
		}
		if(verbose)
			fprintf(stderr,"\n");

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

void usage(){
	fprintf(stdout, "\nInsert Delays\n");
	fprintf(stdout, "\nUsage: \n");
	fprintf(stdout, "%s\n", progname);
	fprintf(stdout, "     -p   [list of PIDs to control (default: all that this user has permissions to)]\n");
	fprintf(stdout, "     -o   Only control PIDs specified at the beginning. Leave their children alone. Does not require -p.\n");
	fprintf(stdout, "     -e   [list of PIDs to exclude (default: none)]\n");
	fprintf(stdout, "     -x   Only exclude specified PIDs. Allow their children to potentially be controlled. Requires -e.\n");
	fprintf(stdout, "     -d   [Initial duty (default: %lf)]\n", DEFAULT_DUTY);
	fprintf(stdout, "     -u   [Process tree update interval in seconds (default: %lf)]\n", DEFAULT_UPDATE_INTERVAL);
	fprintf(stdout, "     -w   [Work interval in seconds (default: %lf)]\n", DEFAULT_WORK_INTERVAL);
	fprintf(stdout, "     -v   Verbose mode\n");
	fprintf(stdout, "     -U   Update on error\n");
	fprintf(stdout, "\n");
}

// Turn stoppable children of init (pid = 1) into parents.
// Clear all the child flags, since this only gets called with explicitParentList set.
int update_snapshot_pid(int pid, void* value, trie_root* root){
	proc_info* pinfo = (proc_info*) value;
	if(pinfo->flags & FLAG_STOPPABLE){
		pinfo->flags |= FLAG_PARENT;
	}
	pinfo->flags &= ~(FLAG_CHILD);
	return 1;
}

int cmdline(int argc, char **argv){
	progname = argv[0];

	int* parentList;
	char explicitParentList;
	int* exclusionList;
	char explicitExclusionList;

	int i,j;
	char p_flag_pos = -1;
	int parentListLen = -1;
	char e_flag_pos = -1;
	int exclusionListLen = -1;

	update_interval = DEFAULT_UPDATE_INTERVAL;
	work_interval = DEFAULT_WORK_INTERVAL;
	duty = DEFAULT_DUTY;
	verbose = 0;
	update_on_error = 0;
	explicitParentList = 0;
	explicitExclusionList = 0;
	parentList = NULL;
	exclusionList = NULL;
	proc_tree = NULL;
	my_pid = getpid();

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
			else if(opt == 'x'){
				explicitExclusionList = 1;
			}
			else if(opt == 'v'){
				verbose = 1;
			}
			else if(opt == 'U'){
				update_on_error = 1;
			}
			else if(opt == 'h'){
				usage();
				normExit();
			}
			else if(opt == 'u'){
				if(i+1 >= argc || sscanf(argv[i+1],"%lf",&update_interval) != 1) {
					fprintf(stderr,"update: %d, %lf\n",i+1,update_interval);
					usage();
					return -1;
				}
			}
			else if(opt == 'w'){
				if(i+1 >= argc || sscanf(argv[i+1],"%lf",&work_interval) != 1) {
					fprintf(stderr,"work: %d, %lf\n",i+1,work_interval);
					usage();
					return -1;
				}
			}
			else if(opt == 'd'){
				if(i+1 >= argc || sscanf(argv[i+1],"%lf",&duty) != 1 ){
					fprintf(stderr,"duty: %d, %lf\n",i+1,duty);
					usage();
					return -1;
				}
			}
			else{
				fprintf(stderr,"Invalid command line argument -%c\n",opt);
				usage();
				return -1;
			}
			j++;
		}
	}
	//trie_set_verbose(verbose);
	proc_tree_set_verbose(verbose);

	if(e_flag_pos > 0){
		if(exclusionListLen < 0){
			exclusionListLen = i - e_flag_pos - 1;
		}

		if(exclusionListLen == 0){
			fprintf(stderr,"You have to specify an exclusion list.\n");
			usage();
			return -1;
		}
	}

	if(p_flag_pos > 0){
		if(parentListLen < 0){
			parentListLen = i - p_flag_pos - 1;
		}

		if(parentListLen == 0){
			fprintf(stderr,"You have to specify a parent list.\n");
			usage();
			return -1;
		}
	}

	if(e_flag_pos < 0 && explicitExclusionList){
		fprintf(stderr,"You have to specify an exclusion list to use the -x option.\n");
		usage();
		return -1;
	}

	if(e_flag_pos > 0){
		exclusionList = (int*) malloc(sizeof(int)*(exclusionListLen + 2));
		for(i = 0; i < exclusionListLen; i++){
			exclusionList[i] = atoi(argv[i + e_flag_pos + 1]);
		}
		exclusionList[i] = 0;
	}
	else{
		exclusionList = (int*) malloc(sizeof(int)*2);
		exclusionList[0] = my_pid;
		exclusionList[1] = 0;
	}

	if(p_flag_pos > 0){
		parentList = (int*) malloc(sizeof(int)*(parentListLen + 1));
		for(i = 0; i < parentListLen; i++){
			parentList[i] = atoi(argv[i + p_flag_pos + 1]);
		}
		parentList[i] = 0;
	}

	else{
		parentList = (int*) malloc(sizeof(int)*2);
		parentList[0] = 1;
		parentList[1] = 0;
		// First, get a tree with all the children of init
		proc_tree = create_proc_tree(parentList,0,exclusionList,explicitExclusionList);
		ptree_info = (proc_tree_info*) proc_tree->data;
		if(update_proc_tree(proc_tree)){
			perror("Failed to update proc tree (1).");
			free(parentList);
			free(exclusionList);
			return -1;
		}

		// Take a snapshot of the currently modifyable processes, and make that the parent list.
		if(explicitParentList){
			// Then, update all the stoppable nodes to be parent nodes
			trie_iterate(update_snapshot_pid, proc_tree);

			// Then, reset the info of the tree to be correct.
			ptree_info->explicitParentList = explicitParentList;
			free(parentList);
			parentList = (int*) malloc(sizeof(int)*(ptree_info->stoppableListSize));
			memcpy(parentList,ptree_info->stoppableList,sizeof(int)*(ptree_info->stoppableListSize));
		}
	}

	if(verbose){
		fprintf(stderr,"Update interval: %g\n",update_interval);
		fprintf(stderr,"Work interval: %g\n",work_interval);
		fprintf(stderr,"Duty cycle: %g\n",duty);
		fprintf(stderr,"Parent list (");
		if(explicitParentList){
			fprintf(stderr,"these and only these):\n");
		}
		else{
			fprintf(stderr,"these and their children):\n");
		}
		i = 0;
		while(parentList[i] > 0){
			fprintf(stderr,"   %d\n",parentList[i++]);
		}

		fprintf(stderr,"Exclusion list (");
		if(explicitExclusionList){
			fprintf(stderr,"these and only these):\n");
		}
		else{
			fprintf(stderr,"these and their children):\n");
		}
		i = 0;
		while(exclusionList[i] > 0){
			fprintf(stderr,"   %d\n",exclusionList[i++]);
		}
	}

	if(proc_tree == NULL){
		proc_tree = create_proc_tree(parentList, explicitParentList, exclusionList, explicitExclusionList);
		ptree_info = (proc_tree_info*) proc_tree->data;
		if(update_proc_tree(proc_tree)){
			perror("Failed to update proc tree (2).");
			free(parentList);
			free(exclusionList);
			return -1;
		}
	}

	if(verbose){
		fprintf(stderr,"Stoppable list (buffer size: %d, index: %d): ", ptree_info->stoppableListSize, ptree_info->stoppableListIndex);
		if(ptree_info->stoppableList != NULL){
			i = 0;
			while(ptree_info->stoppableList[i] > 0){
				fprintf(stderr,"%d ",ptree_info->stoppableList[i++]);
			}
		}
		fprintf(stderr,"\n");
	}

	free(parentList);
	free(exclusionList);

	if(verbose)
		print_proc_tree(proc_tree);

	return 0;
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

	if(cmdline(argc,argv)){
		errExit("Invalid command line arguments.");
	}

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
					fprintf(stderr,"Updated process tree. ");
					fprintf(stderr,"Stoppable list: ");
					if(ptree_info->stoppableList != NULL){
						int i = 0;
						while(ptree_info->stoppableList[i] > 0){
							fprintf(stderr,"%d ",ptree_info->stoppableList[i++]);
						}
					}
					fprintf(stderr,"\n");
				}

				if(clock_gettime(CLOCK_BOOTTIME, &currentTime)){
					errExit("Problem with gettime");
				}
				time_last_updated = ((double)(currentTime.tv_sec)) + (((double)(currentTime.tv_nsec))/1.0e9);
				//time_last_updated = currentTime_sec;
			}
			//slen = (1.0-duty)*(currentTime_sec - prevTime_sec);
			slen = (1.0-duty)*0.1;
			sleeplen = (useconds_t)(slen*1.0e6);
			//if(verbose)
			//	fprintf(stderr,"Sleeping for %g seconds\n",slen);
			usleep(sleeplen);
			prevTime_sec = currentTime_sec;
		}

		// If the user has typed something, process what the user has typed.
		else if (num_readable == 1) {
			if(scanf("%lf",&duty) > 0){
				if(duty > MAX_DUTY){
					duty = MAX_DUTY;
				}
				else if(duty < MIN_DUTY){
					duty = MIN_DUTY;
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
