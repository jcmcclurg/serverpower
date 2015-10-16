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

char proc_tree_verbose = 0;
char proc_tree_name_buff[128];

void proc_tree_set_verbose(char v){
	proc_tree_verbose = v;
}

//
// Returns the number of processes in the tree, and populates the process tree, or returns -1 on failure.

static int check_proc(void) {
	struct statfs mnt;
	if (statfs("/proc", &mnt) < 0)
		return 0;
	if (mnt.f_type!=0x9fa0)
		return 0;
	return 1;
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

char* get_name_of(pid_t pid, char* nameof_buff, int size) {
	char statfile[20];
	sprintf(statfile, "/proc/%d/stat", pid);
	FILE *fd = fopen(statfile, "r");
	if (fd==NULL) return NULL;
	if (fgets(nameof_buff, size, fd)==NULL) {
		fclose(fd);
		return NULL;
	}
	fclose(fd);

	char* buffer = strtok(nameof_buff, " ");
	buffer = strtok(NULL, " ");
	return buffer;
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

proc_info* proc_info_new(pid_t ppid, char flags){
	proc_info* proc = (proc_info*) malloc(sizeof(proc_info));
	proc->flags = flags;
	proc->ppid = ppid;
	return proc;
}

proc_tree_info* proc_tree_info_new(char explicitParentList, char explicitExclusionList){
	proc_tree_info* ptree_info = (proc_tree_info*) malloc(sizeof(proc_tree_info));
	ptree_info->explicitParentList = explicitParentList;
	ptree_info->explicitExclusionList = explicitExclusionList;
	ptree_info->stoppableList = NULL;
	ptree_info->stoppableListSize = 0;
	return ptree_info;
}

void deallocate_proc_tree(trie_root* ptree){
	proc_tree_info* ptree_info = (proc_tree_info*) ptree->data;
	free(ptree_info->stoppableList);
	free(ptree_info);
	trie_deallocate(ptree);
}

trie_root* create_proc_tree(int* parentList, char explicitParentList, int* exclusionList, char explicitExclusionList){
	int i = 0;
	int r = 0;
	trie_root* ptree = trie_new();
	if(ptree == NULL){
		return NULL;
	}
	ptree->data = (void*) proc_tree_info_new(explicitParentList, explicitExclusionList);

	if(parentList != NULL){
		if(proc_tree_verbose)
			fprintf(stderr,"Creating proc tree...\n   Inserting parents ");
		while(parentList[i] > 0){
			if(proc_tree_verbose)
				fprintf(stderr,"%d ",parentList[i]);
			void* proc = (void*) proc_info_new(0, FLAG_PARENT | FLAG_MARK_NEW);
			void* v = trie_insert(parentList[i], proc, ptree);

			// Success at inserting
			if(v == proc){
				if(proc_tree_verbose)
					fprintf(stderr,"(yes) ");
				r++;
			}
			else{
				if(proc_tree_verbose)
					fprintf(stderr,"(no) ");
				free(proc);
			}
			i++;
		}
		if(proc_tree_verbose)
			fprintf(stderr,"\n");
	}

	if(exclusionList != NULL){
		i = 0;
		if(proc_tree_verbose)
			fprintf(stderr,"   Inserting exclusions ");
		while(exclusionList[i] > 0){
			if(proc_tree_verbose)
				fprintf(stderr,"%d ",exclusionList[i]);
			proc_info* proc = proc_info_new(0, FLAG_EXCLUDE);
			void* v = trie_insert(exclusionList[i], proc, ptree);

			// Failure at inserting
			if(v != proc){
				if(proc_tree_verbose)
					fprintf(stderr,"(no) ");
				free(proc);
				proc = (proc_info*) v;
				proc->flags = FLAG_EXCLUDE;
			}
			else{
				if(proc_tree_verbose)
					fprintf(stderr,"(yes) ");
				r++;
			}
			i++;
		}
		if(proc_tree_verbose)
			fprintf(stderr,"\n");
	}
	return ptree;
}

int update_proc_tree_item(int pid, void* value, trie_root* ptree){
	proc_info* pinfo = (proc_info*) value;
	proc_tree_info* ptree_info = (proc_tree_info*) ptree->data;

	char explicitParentList = ptree_info->explicitParentList;
	char explicitExclusionList = ptree_info->explicitExclusionList;

	if(proc_tree_verbose)
		fprintf(stderr,"Updating PID %d %s: ",pid, get_name_of(pid, proc_tree_name_buff, 128));

	if(pinfo->flags & FLAG_MARK_OK){
		pinfo->flags &= ~(FLAG_MARK_OK);
		if(proc_tree_verbose)
			fprintf(stderr,"cleared_ok ");

		if(pinfo->flags & FLAG_MARK_NEW){
			pinfo->flags &= ~(FLAG_MARK_NEW);
			if(proc_tree_verbose)
				fprintf(stderr,"cleared_new ");

			if(!explicitExclusionList){
				// Navigate up until reaching a parent with FLAG_EXCLUDE
				proc_info* currentNode = pinfo;
				while(currentNode->ppid > 0 ){
					currentNode = (proc_info*) trie_value(currentNode->ppid, ptree);
					if(currentNode->flags & FLAG_EXCLUDE){
						// If you found a parent node with FLAG_EXCLUDE, also exclude this node.
						pinfo->flags |= FLAG_EXCLUDE;
						if(proc_tree_verbose)
							fprintf(stderr,"marked_exclude ");
						break;
					}
				}
			}
			
			// If you didn't find a parent with FLAG_EXCLUDE
			if( !(pinfo->flags & FLAG_EXCLUDE)){
				if(explicitParentList){
					if((pinfo->flags & FLAG_PARENT) && get_stoppable_status(pid) == STOPPABLE){
						if(proc_tree_verbose)
							fprintf(stderr," got_stoppable_status ");
						pinfo->flags |= FLAG_STOPPABLE;
					}
				}
				// navigate up until reaching a parent or a child node.
				else{
					proc_info* currentNode = pinfo;
					while(currentNode->ppid > 0 && !(currentNode->flags & (FLAG_PARENT | FLAG_CHILD))){
						currentNode = (proc_info*) trie_value(currentNode->ppid, ptree);
						if(proc_tree_verbose)
							fprintf(stderr,"->%d",currentNode->ppid);
					}
					// If you found a parent or a child node, mark this as a child
					if(currentNode->flags & (FLAG_PARENT | FLAG_CHILD)){
						if(pinfo != currentNode){
							pinfo->flags |= FLAG_CHILD;
							if(proc_tree_verbose)
								fprintf(stderr," marked_child_status ");
						}

						// Check whether the child is stoppable.
						if(get_stoppable_status(pid) == STOPPABLE){
							if(proc_tree_verbose)
								fprintf(stderr," got_stoppable_status ");
							pinfo->flags |= FLAG_STOPPABLE;
						}
					}
				}
			}
		}

		if(pinfo->flags & FLAG_STOPPABLE){
			if(proc_tree_verbose)
				fprintf(stderr,"add_to_stoppable_list ");
			ptree_info->stoppableList[ptree_info->stoppableListIndex] = pid;
			ptree_info->stoppableListIndex++;
		}
	}
	else{
		if(proc_tree_verbose)
			fprintf(stderr,"removed ");
		trie_remove(pid,ptree);
	}
	if(proc_tree_verbose)
		fprintf(stderr,"\n");
	return 1;
}

int update_proc_tree(trie_root* ptree){
	if(ptree == NULL){
		perror("Null ptree!");
		return -1;
	}
	else if(ptree->num_nodes == 0){
		perror("Number of nodes is zero!");
		return -1;
	}
	else if(ptree->data == NULL){
		perror("Null ptree data!");
		return -1;
	}
	else if(ptree->next_one == NULL && ptree->next_zero == NULL){
		perror("Empty ptree!");
		return -1;
	}
	proc_tree_info* ptree_info = (proc_tree_info*) ptree->data;
	char explicitExclusionList = ptree_info->explicitExclusionList;
	char explicitParentList = ptree_info->explicitParentList;
	ptree_info->stoppableListIndex = 0;

	if(!check_proc()) {
		perror("procfs is not mounted!\nAborting\n");
		return -1;
	}

	//open a directory stream to /proc directory
	DIR* procDir;
	struct dirent *ep;
	int pid, ppid;
	int num_pids = 0;


	procDir = opendir("/proc");
	if(procDir == NULL) {
		perror("opendir");
		return -1;
	}

	// Get number of processes
	while( (ep = readdir(procDir)) ){
		if(sscanf(ep->d_name,"%d",&pid) == 1 && pid > 0){
			if(proc_tree_verbose)
				fprintf(stderr,"Inserting process %d",pid);

			proc_info* proc = (void*) proc_info_new(0, FLAG_MARK_OK | FLAG_MARK_NEW);
			void* v = trie_insert(pid, proc, ptree);

			// Failure at inserting
			if(v != proc){
				if(proc_tree_verbose)
					fprintf(stderr,"   Already inserted.\n");
				free(proc);
				proc = (proc_info*) v;
				proc->flags |= FLAG_MARK_OK;
				if((!explicitExclusionList || !explicitParentList) && proc->ppid == 0 && pid != 1){
					if(proc_tree_verbose)
						fprintf(stderr,"   Getting ppid.\n");
					ppid = get_ppid_of(pid);
					proc->ppid = ppid;
				}
			}
			// Success at inserting
			else if(!explicitExclusionList || !explicitParentList){
				if(proc_tree_verbose)
					fprintf(stderr,"   New! Getting ppid.\n");
				ppid = get_ppid_of(pid);
				proc->ppid = ppid;
			}
			else if(proc_tree_verbose){
				fprintf(stderr,"   New!\n");
			}
			num_pids++;
		}
	}
	closedir(procDir);

	// Reallocate the stoppable list if necessary.
	if(num_pids+1 > ptree_info->stoppableListSize){
		if(proc_tree_verbose)
			fprintf(stderr,"Reallocating stoppable list buffer to size %d.\n",num_pids);
		free(ptree_info->stoppableList);
		ptree_info->stoppableListSize = num_pids+1;
		ptree_info->stoppableList = (int*) malloc(sizeof(int)*(num_pids + 1));
	}

	// Iterate through the PIDs
	trie_iterate(update_proc_tree_item, ptree);
	ptree_info->stoppableList[ptree_info->stoppableListIndex] = 0;
	
	return 0;
}


int print_proc_tree_node(int pid, void* value, trie_root* ptree){
	proc_info* pinfo = (proc_info*) value;
	fprintf(stderr,"PID: %d %s, PPID: %d, Flags: ", pid, get_name_of(pid, proc_tree_name_buff, 128), pinfo->ppid);
	if(pinfo->flags & FLAG_PARENT)
		fprintf(stderr,"PARENT ");
	if(pinfo->flags & FLAG_CHILD)
		fprintf(stderr,"CHILD ");
	if(pinfo->flags & FLAG_EXCLUDE)
		fprintf(stderr,"EXCLUDE ");
	if(pinfo->flags & FLAG_STOPPABLE)
		fprintf(stderr,"STOPPABLE ");
	if(pinfo->flags & FLAG_MARK_OK)
		fprintf(stderr,"OK ");
	if(pinfo->flags & FLAG_MARK_NEW)
		fprintf(stderr,"NEW ");
	fprintf(stderr,"\n");
	return 1;
}

void print_proc_tree(trie_root* ptree){
	trie_iterate(print_proc_tree_node,ptree);
}

int get_stoppable_status(pid_t pid){
	if(kill(pid, SIGSTOP)){
		if(proc_tree_verbose)
			fprintf(stderr,"%d has insufficient permissions to stop.\n",pid);
		return INSUFFICIENT_PERMISSIONS_STOP;
	}
	else{
		usleep(CHECK_EXTERNAL_RESTART_WAITLEN_US);
		if( get_status_of(pid) != 'T'){
			if(proc_tree_verbose)
				fprintf(stderr,"%d is externally restarted.\n",pid);
			return EXTERNALLY_RESTARTED;
		}
		else if(kill(pid, SIGCONT)){
			if(proc_tree_verbose)
				fprintf(stderr,"%d has insufficient permissions to continue.\n",pid);
			return INSUFFICIENT_PERMISSIONS_CONT;
		}

		usleep(CHECK_CONT_WAITLEN_US);
		if(get_status_of(pid) == 'T'){
			if(proc_tree_verbose)
				fprintf(stderr,"%d does not continue.\n",pid);
			return DOES_NOT_CONTINUE;
		}
		else{
			if(proc_tree_verbose)
				fprintf(stderr,"%d is stoppable.\n",pid);
			return STOPPABLE;
		}
	}
}
