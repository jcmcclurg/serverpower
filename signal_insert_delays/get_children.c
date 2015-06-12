/**
 *
 * cpulimit - a CPU limiter for Linux
 *
 * Copyright (C) 2005-2012, by:  Angelo Marletta <angelo dot marletta at gmail dot com> 
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * Modified summer 2015 by Josiah McClurg
 */

#include <sys/vfs.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "get_children.h"

static int check_proc(void) {
	struct statfs mnt;
	if (statfs("/proc", &mnt) < 0)
		return 0;
	if (mnt.f_type!=0x9fa0)
		return 0;
	return 1;
}

// Returns the int or -1 if it's not an int.
static int parseStrAsUint(char* s){
	char* s2 = s;
	while(*s2 != 0){
		if(*s2 >= '0' && *s2 <= '9'){
			s2++;
		}
		else{
			return -1;
		}
	}
	return atoi(s);
}

pid_t get_ppid_of(pid_t pid) {
	char statfile[20];
	char buffer[1024];
	sprintf(statfile, "/proc/%d/stat", pid);
	FILE *fd = fopen(statfile, "r");
	if (fd==NULL) return -1;
	if (fgets(buffer, sizeof(buffer), fd)==NULL) {
		fclose(fd);
		return -1;
	}
	fclose(fd);
	char *token = strtok(buffer, " ");
	int i;
	for (i=0; i<3; i++) token = strtok(NULL, " ");
	return atoi(token);
}

char get_status_of(pid_t pid){
	char statfile[20];
	char buffer[1024];
	sprintf(statfile, "/proc/%d/stat", pid);
	FILE *fd = fopen(statfile, "r");
	if (fd==NULL) return -1;
	if (fgets(buffer, sizeof(buffer), fd)==NULL) {
		fclose(fd);
		return -1;
	}
	fclose(fd);
	char *token = strtok(buffer, " ");
	int i;
	for (i=0; i<2; i++) token = strtok(NULL, " ");
	return token[0];
}

// Returns the number of processes in the tree, and populates the process tree, or returns -1 on failure.
process_tree* proc_tree_buffer;
int proc_tree_buffer_size;
int num_procs;

int* child_buffer;
int child_buffer_size;
int num_children;

int tag_proc_tree_nonchildren(int* nonChildren, int numNonChildren){
	process_tree* procTree = proc_tree_buffer;
	int i,j;
	int numProcs = num_procs;

	int numTagged;
	for(i = 0; i < numProcs; i++){
		procTree[i].class = UNKNOWN_CLASS;
		for(j = 0; j < numNonChildren; j++){
			if(nonChildren[j] == procTree[i].pid){
				procTree[i].class = NONCHILD_CLASS;
				numTagged++;
				#ifdef DEBUG
				fprintf(stderr,"Tagged %d as nonchild.\n",procTree[i].pid);
				#endif
				break;
			}
		}
	}
	return numTagged;
}

int update_proc_tree(process_tree** procTree, int* numProcs){
	if(!check_proc()) {
		perror("procfs is not mounted!\nAborting\n");
		return -1;
	}
	//open a directory stream to /proc directory
	DIR* procDir;
	int i,j,k;
	struct dirent *ep;
	int pid, ppid;
	int totalPIDs = 0;

	procDir = opendir("/proc");

	if(procDir == NULL) {
		perror("opendir");
		return -1;
	}

	// Get number of processes
	while( (ep = readdir(procDir)) ){
		pid = parseStrAsUint(ep->d_name);
		if(pid >= 0){
			totalPIDs++;
		}
	}

	// Increase the size of the buffer if needed.
	if(proc_tree_buffer_size < totalPIDs){
		free(proc_tree_buffer);
		proc_tree_buffer = (process_tree*) malloc(totalPIDs*sizeof(process_tree));
		proc_tree_buffer_size = totalPIDs;
	}

	// Populate the process tree.
	rewinddir(procDir);
	k = 0;
	while ( (k < totalPIDs) && (ep = readdir(procDir)) ){
		pid = parseStrAsUint(ep->d_name);
		if(pid >= 0){
			ppid = get_ppid_of(pid);
			proc_tree_buffer[k].pid = pid;
			proc_tree_buffer[k].ppid = ppid;
			proc_tree_buffer[k].parent = -1;
			proc_tree_buffer[k].class = UNKNOWN_CLASS;
			k++;
		}
	}
	// Adjust for processes which died between the first and second read of the directory.
	totalPIDs = k;
	num_procs = k;
	closedir(procDir);
	
	// Make the tree edges. 
	for(i = 0; i < totalPIDs; i++){
		pid = proc_tree_buffer[i].pid;	
		ppid = proc_tree_buffer[i].ppid;
		for(j = 0; j < totalPIDs; j++){
			if(i != j && ppid == proc_tree_buffer[j].pid ){
				proc_tree_buffer[i].parent = j;
				#ifdef DEBUG
				fprintf(stderr,"g[%d]=%d has parent g[%d]=%d\n",i,pid,j,ppid);
				#endif
			}
		}
	}

	if(procTree != NULL)
		*procTree = proc_tree_buffer;
	if(numProcs != NULL)
		*numProcs = num_procs;

	return 0;
}

int update_children(	int** childNodes, int* numChildNodes,
						int* exclusions, int numExclusions,
						char exclude_nonstoppable){
	process_tree* procTree = proc_tree_buffer;
	int numProcs = num_procs;
	int i,j,k;
	char c;
	j = 0;
	if(exclusions == NULL)
		numExclusions = 0;

	for(i = 0; i < numProcs; i++){
		if(procTree[i].class != UNKNOWN_CLASS){
			j++;
		}
	}
	// Increase the size of the buffer if needed.
	if(child_buffer_size < j){
		free(child_buffer);
		child_buffer = (int*) malloc(j*sizeof(int));
		child_buffer_size = j;
	}
	*numChildNodes = j;
	#ifdef DEBUG
	fprintf(stderr,"Updating %d children (en=%d):\n", j, exclude_nonstoppable);
	#endif
	j = 0;
	for(i = 0; i < numProcs; i++){
		if((procTree[i].class != UNKNOWN_CLASS) &&
		(!exclude_nonstoppable || get_stoppable_status(procTree[i].pid) == STOPPABLE)	){
			// Check if it is in exclude list
			c = 0;
			for(k = 0; k < numExclusions; k++){
				if(procTree[i].pid == exclusions[k]){
					c = 1;
					break;
				}
			}
			if(!c){
				child_buffer[j] = procTree[i].pid;
				#ifdef DEBUG
				fprintf(stderr,"Yes. %d is child.\n",child_buffer[j]);
				#endif
				j++;
			}
			#ifdef DEBUG
			else{
				fprintf(stderr,"Oops. %d is not a child.\n",procTree[i].pid);
			}
			#endif
		}
		#ifdef DEBUG
		else{
			fprintf(stderr,"%d did not make the cut: CLS:%d, STS:%d\n",procTree[i].pid, procTree[i].class, get_stoppable_status(procTree[i].pid));
		}
		#endif
	}
	*childNodes = child_buffer;
	*numChildNodes = j;
	num_children = j;
	#ifdef DEBUG
	fprintf(stderr,"Found %d children.\n",num_children);
	#endif
	return 0;
}

int tag_proc_tree_children(int* rootPIDs, int numRootPIDs) {
	int numProcs = num_procs;

	if(tag_proc_tree_nonchildren(rootPIDs, numRootPIDs) == 0){
		perror("Couldn't find any PIDs in the list you specified!");
		return -1;
	}

	int i,j;
	char changed;
	process_tree* procTree = proc_tree_buffer;

	for(i = 0; i < numProcs; i++){
		j = i;
		// Find out whether this node has any other marked nodes in its lineage.
		changed = 0;
		#ifdef DEBUG
		fprintf(stderr,"%d->",procTree[j].pid );
		#endif
		while( (j = procTree[j].parent) != -1 ){
			#ifdef DEBUG
			fprintf(stderr,"%d",procTree[j].pid );
			#endif
			if(procTree[j].class != UNKNOWN_CLASS ){
				changed = 1;
				break;
			}
			#ifdef DEBUG
			else{
				fprintf(stderr,"->");
			}
			#endif
		}

		// If so, the whole lineage (excluding the topmost node) are rootPIDs.
		if(changed){
			#ifdef DEBUG
			fprintf(stderr," CHILD\n" );
			#endif
			procTree[i].class = CHILD_CLASS;
			j = procTree[i].parent;
			while( procTree[j].class == UNKNOWN_CLASS  ){
				procTree[j].class = CHILD_CLASS;
				j = procTree[j].parent;
			}
		}
		#ifdef DEBUG
		else{
			fprintf(stderr,"\n" );
		}
		#endif
	}
	return 0;
}

pid_t my_pid;
pid_t my_ppid;
void init_children(void){
	proc_tree_buffer = NULL;
	proc_tree_buffer_size = 0;
	num_procs = 0;
	child_buffer = NULL;
	child_buffer_size = 0;
	num_children = 0;
	my_pid = getpid();
	my_ppid = getppid();
	update_proc_tree(NULL,NULL);
}

void close_children(void){
	proc_tree_buffer_size = 0;
	child_buffer_size = 0;
	num_children = 0;
	num_procs = 0;
	free(proc_tree_buffer);
	free(child_buffer);
	my_pid = -1;
	my_ppid = -1;
}



int get_stoppable_status(pid_t pid){
	if(pid == my_pid){
		#ifdef DEBUG
		fprintf(stderr,"%d is self.\n",pid);
		#endif
		return IS_SELF;
	}
	else if(pid == my_ppid){
		#ifdef DEBUG
		fprintf(stderr,"%d is parent.\n",pid);
		#endif
		return IS_PARENT;
	}
	else{
		if(kill(pid, SIGSTOP)){
			#ifdef DEBUG
			fprintf(stderr,"%d has insufficient permissions to stop.\n",pid);
			#endif
			return INSUFFICIENT_PERMISSIONS_STOP;
		}
		else{
			usleep(CHECK_EXTERNAL_RESTART_WAITLEN_US);
			if( get_status_of(pid) != 'T'){
				#ifdef DEBUG
				fprintf(stderr,"%d is externally restarted.\n",pid);
				#endif
				return EXTERNALLY_RESTARTED;
			}
			else if(kill(pid, SIGCONT)){
				#ifdef DEBUG
				fprintf(stderr,"%d has insufficient permissions to continue.\n",pid);
				#endif
				return INSUFFICIENT_PERMISSIONS_CONT;
			}

			usleep(CHECK_CONT_WAITLEN_US);
			if(get_status_of(pid) == 'T'){
				#ifdef DEBUG
				fprintf(stderr,"%d does not continue.\n",pid);
				#endif
				return DOES_NOT_CONTINUE;
			}
			else{
				#ifdef DEBUG
				fprintf(stderr,"%d is stoppable.\n",pid);
				#endif
				return STOPPABLE;
			}
		}
	}
}
