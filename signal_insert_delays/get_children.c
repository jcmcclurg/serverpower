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

static int check_proc() {
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
int* child_buffer;
int child_buffer_size;

int tag_proc_tree(process_tree* procTree, int* numProcs, int* nonChildren, int numNonChildren){
	int numTagged;
	for(i = 0; i < numProcs; i++){
		procTree[i].class = UNKNOWN_CLASS;
		for(j = 0; j < numNonChildren; j++){
			if(nonChildren[j] == procTree[i].pid){
				procTree[i].class = NONCHILD_CLASS;
				numTagged++;
				break;
			}
		}
	}
	return 0;
}

int update_proc_tree(process_tree** procTree){
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
	int ret, i, j, rootNode, counter;

	if(rootPIDs == NULL)
		numRootPIDs = 0;

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
	for(j = 0; (j < totalPIDs) && (ep = readdir(procDir)); j++ ){
		pid = parseStrAsUint(ep->d_name);
		if(pid >= 0){
			ppid = get_ppid_of(pid);
			proc_tree_buffer[j].pid = pid;
			proc_tree_buffer[j].ppid = ppid;
			proc_tree_buffer[j].parent = -1;
			proc_tree_buffer[j].class = UNKNOWN_CLASS;
		}
	}
	// Adjust for processes which died between the first and second read of the directory.
	totalPIDs = j;

	if(rootNode == -1){
		perror("There is no process with the specified PID");
		return -1;
	}
	closedir(procDir);
	
	// Make the tree edges. 
	for(i = 0; i < totalPIDs; i++){
		pid = proc_tree_buffer[i].pid;	
		ppid = proc_tree_buffer[i].ppid;
		for(j = 0; j < totalPIDs; j++){
			if(i != j && ppid == proc_tree_buffer[j].pid ){
				proc_tree_buffer[i].parent = j;
				//fprintf(stdout,"g[%d]=%d has parent g[%d]=%d\n",i,pid,j,ppid);
			}
		}
	}
	return 0;
}

int get_children(	int** childNodes, int* numChildren,
						process_tree* procTree, int numProcs,
						int* rootPIDs, int numRootPIDs) {

	// Mark all the parent nodes of rootNode as nonchildren
	i = rootNode;
	proc_tree_buffer[i].class = CHILD_CLASS;
	counter = 1;
	while( (i = proc_tree_buffer[i].parent) != -1 ){
		proc_tree_buffer[i].class = NONCHILD_CLASS;
	}

	// Separate the rest of the nodes into CHILD and NONCHILD status
	for(i = 0; i < totalPIDs; i++){
		if(proc_tree_buffer[i].class == UNKNOWN_CLASS){
			j = i;
			ret = UNKNOWN_CLASS;
			// Traverse the node up to a parent with a known status.
			while( (j = proc_tree_buffer[j].parent) != -1){
				if( proc_tree_buffer[j].class == NONCHILD_CLASS){
					ret = NONCHILD_CLASS;
					break;
				}
				else if( proc_tree_buffer[j].class == CHILD_CLASS){
					ret = CHILD_CLASS;
					break;
				}
			}

			// Mark all the nodes in that trail with the (now) known status.
			if(ret != UNKNOWN_CLASS){
				j = i;
				while( proc_tree_buffer[j].class == UNKNOWN_CLASS){
					proc_tree_buffer[j].class = ret;
					j = proc_tree_buffer[j].parent;
					if(ret == CHILD_CLASS)
						counter++;
				}
			}
		}
	}

	// Increase the size of the buffer if needed.
	if(child_buffer_size < counter){
		free(child_buffer);
		child_buffer = (int*) malloc(counter*sizeof(int));
		child_buffer_size = counter;
	}
	j = 0;
	for(i = 0; i < totalPIDs; i++){
		if(proc_tree_buffer[i].class == CHILD_CLASS){
			child_buffer[j++] = proc_tree_buffer[i].pid;
		}
	}

	if(childNodes != NULL){
		*childNodes = child_buffer;
	}
	
	if(numChildren != NULL)
		*numChildren = counter;

	if(procTree != NULL)
		*procTree = proc_tree_buffer;

	if(numProcs != NULL)
		*numProcs = totalPIDs;

	return 0;
}

void init_children(void){
	proc_tree_buffer = NULL;
	proc_tree_buffer_size = 0;
	child_buffer = NULL;
	child_buffer_size = 0;
}

void close_children(void){
	proc_tree_buffer_size = 0;
	child_buffer_size = 0;
	free(proc_tree_buffer);
	free(child_buffer);
}

void reset

int get_roots(process_tree* procTree, int numProcs, int* children, int numChildren){
	int i,j;
	char changed;

	// Mark the appropriate nodes in the tree tentatively as nonchildren. All the rest are unknown.
	
	for(i = 0; i < numProcs; i++){
		if(procTree[i].class == NONCHILD_CLASS){
			j = i;
			// Find out whether this node has any other marked nodes in its lineage.
			changed = 0;
			//fprintf(stderr,"%d->",procTree[j].pid );
			while( (j = procTree[j].parent) != -1 ){
				//fprintf(stderr,"%d",procTree[j].pid );
				if(procTree[j].class != UNKNOWN_CLASS ){
					changed = 1;
					break;
				}
				//else{
				//	fprintf(stderr,"->");
				//}
			}

			// If so, the whole lineage (excluding the topmost node) are children.
			if(changed){
				//fprintf(stderr," CHILDREN\n" );
				procTree[i].class = CHILD_CLASS;
				j = procTree[i].parent;
				while( procTree[j].class == UNKNOWN_CLASS  ){
					procTree[j].class = CHILD_CLASS;
					j = procTree[j].parent;
				}
			}
			//else{
			//	fprintf(stderr," ROOT\n" );
			//}
		}
	}

	j = 0;
	for(i = 0; i < numProcs; i++){
		if(procTree[i].class == NONCHILD_CLASS){
			children[j] = procTree[i].pid;
			j++;
		}
	}
	return j;
}

int get_stoppable_status(pid_t pid){
	if(pid == my_pid){
		return IS_SELF;
	}
	else if(pid != my_ppid){
		return IS_PARENT;
	}
	else{
		if(kill(pid, SIGSTOP)){
			return INSUFFICIENT_PERMISSIONS_STOP;
		}
		else{
			usleep(CHECK_EXTERNAL_RESTART_WAITLEN_US);
			if( (c = get_status_of(pid)) != 'T'){
				return EXTERNALLY_RESTARTED;
			}
			else if(kill(pid, SIGCONT)){
				return INSUFFICIENT_PERMISSIONS_CONT;
			}

			usleep(CHECK_CONT_WAITLEN_US);
			if(get_status_of(pid) == 'T'){
				return DOES_NOT_CONTINUE;
			}
			else{
				return STOPPABLE;
			}
		}
	}
}
