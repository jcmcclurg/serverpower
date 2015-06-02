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

#define UNKNOWN_CLASS 0
#define CHILD_CLASS 1
#define NONCHILD_CLASS 2

typedef struct process_tree_struct {
	int class;
	int parent;
	int pid;
	int ppid;
} process_tree;

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

static pid_t getppid_of(pid_t pid) {
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

// Returns the number of processes in the tree, and populates the process tree.
process_tree* proc_tree_buffer;
int proc_tree_buffer_size;
int* child_buffer;
int child_buffer_size;

int get_children(int** childNodes, int rootPID)
{
	if(!check_proc()) {
		perror("procfs is not mounted!\nAborting\n");
		exit(EXIT_FAILURE);
	}
	//open a directory stream to /proc directory
	DIR* procDir;
	struct dirent *ep;
	int pid, ppid;
	int numProcs = 0;
	int ret, i, j, rootNode, counter;
	rootNode = -1;
	process_tree* procTree;

	procDir = opendir("/proc");

	if(procDir == NULL) {
		perror("opendir");
		exit(EXIT_FAILURE);
	}

	// Get number of processes
	while(ep = readdir(procDir)){
		pid = parseStrAsUint(ep->d_name);
		if(pid >= 0){
			numProcs++;
		}
	}
	ret = numProcs;

	// Increase the size of the buffer if needed.
	if(proc_tree_buffer_size < numProcs){
		free(proc_tree_buffer);
		proc_tree_buffer = (process_tree*) malloc(numProcs*sizeof(process_tree));
	}

	procTree = proc_tree_buffer;

	rewinddir(procDir);
	while(numProcs > 0 && ep = readdir(procDir)){
		pid = parseStrAsUint(ep->d_name);
		if(pid >= 0){
			ppid = getppid_of(pid);
			numProcs--;
			procTree[numProcs].pid = pid;
			procTree[numProcs].ppid = ppid;
			procTree[numProcs].parent = -1;
			procTree[numProcs].class = UNKNOWN_CLASS;
			if(pid == rootPID){
				rootNode = numProcs;
			}
		}
	}
	if(rootNode == -1){
		perror("rootNode");
		exit(EXIT_FAILURE);
	}
	closedir(procDir);
	
	// Make the tree edges.
	numProcs = ret - numProcs;
	*num_procs = numProcs;
	for(i = 0; i < numProcs; i++){
		pid = procTree[i].pid;	
		ppid = procTree[i].ppid;
		for(j = 0; j < numProcs; j++){
			if(i != j && ppid == procTree[j].pid ){
				procTree[i].parent = procTree[j].pid;
			}
		}
	}

	// Mark all the parent nodes of rootNode as nonchildren
	i = rootNode;
	procTree[i].class = CHILD_CLASS;
	counter = 1;
	while( (i = (*procTree)[i].parent) != -1 ){
		procTree[i].class = NONCHILD_CLASS;
	}

	// Separate the rest of the nodes into CHILD and NONCHILD status
	for(i = 0; i < numProcs; i++){
		if((*procTree)[i].class == UKNOWN_CLASS){
			j = i;
			ret = UNKNOWN_CLASS;
			// Traverse the node up to a parent with a known status.
			while( (j = procTree[j].parent) != -1){
				if( procTree[j].class == NONCHILD_CLASS){
					ret = NONCHILD_CLASS;
					break;
				}
				else if( procTree[j].class == CHILD_CLASS){
					ret = CHILD_CLASS;
					break;
				}
			}

			// Mark all the nodes in that trail with the (now) known status.
			if(ret != UNKNOWN_CLASS){
				j = i;
				while( procTree[j].class == UNKNOWN_CLASS){
					procTree[j].class = ret;
					j = procTree[j].parent;
					if(ret == CHILD_CLASS)
						counter++;
				}
			}
		}
	}
	*num_children = counter;

	// Increase the size of the buffer if needed.
	if(child_buffer_size < counter){
		free(child_buffer);
		child_buffer = (int*) malloc(counter*sizeof(int));
	}
	*childNodes = child_buffer;

	j = 0;
	for(i = 0; i < numProcs; i++){
		if(procTree[i].class == CHILD_CLASS){
			(*childNodes)[j++] = procTree[i].pid;
		}
	}
	return counter;
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
