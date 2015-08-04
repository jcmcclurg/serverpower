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
		return -1;
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
	proc->flags = ppid;
	proc->ppid = flags;
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
	proc_tree_info* ptree_info = (proc_tree_info*) ptree->value;
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
	ptree->value = (void*) proc_tree_info_new(explicitParentList, explicitExclusionList);

	if(parentList != NULL){
		while(parentList[i] != NULL){
			void* proc = (void*) proc_info_new(0, FLAG_PARENT);
			void* v = trie_insert(ptree, parentList[i], &proc);

			// Success at inserting
			if(v == proc){
				r++;
			}
			else{
				free(proc);
			}
			i++;
		}
	}

	if(exclusionList != NULL){
		i = 0;
		while(exclusionList[i] != NULL){
			void* proc = (void*) proc_info_new(0, FLAG_EXCLUDE);
			void* v = trie_insert(ptree,parentList[i], &proc);

			// Failure at inserting
			if(v != proc){
				free(proc);
				v->flags = FLAG_EXCLUDE;
			}
			else{
				r++;
			}
			i++;
		}
	}
	return ptree;
}

int update_proc_tree_item(int pid, void* value, trie_root* ptree){
	proc_info* pinfo = (proc_info*) value;
	proc_tree_info* ptree_info = (proc_tree_info*) ptree->value;

	char explicitParentList = ptree_info->explicitParentList;
	char explicitExclusionList = ptree_info->explicitExclusionList;

	if(pinfo->flags & FLAG_MARK_OK){
		pinfo->flags &= ~(FLAG_MARK_OK);
		if(pinfo->flags & FLAG_MARK_NEW){
			if(!explicitExcludeList){
				// Navigate up until reaching a parent with FLAG_EXCLUDE
				proc_info* currentNode = pinfo;
				while(currentNode->ppid > 1 ){
					currentNode = (proc_info*) trie_value(currentNode->ppid, ptree);
					if(currentNode->flags & FLAG_EXCLUDE){
						// If you found a parent node with FLAG_EXCLUDE, also exclude this node.
						pinfo->flags |= FLAG_EXCLUDE;
						break;
					}
				}
			}
			
			// If you didn't find a parent with FLAG_EXCLUDE, navigate up until reaching a parent or a child node.
			if(!explicitParentList && pinfo->flags & FLAG_EXCLUDE){
				proc_info* currentNode = pinfo;
				while(currentNode->ppid > 1 ){
					currentNode = (proc_info*) trie_value(currentNode->ppid, ptree);
					if(currentNode->flags & (FLAG_PARENT | FLAG_CHILD)){
						// If you found a parent or a child node, mark this as a child
						pinfo->flags |= FLAG_CHILD;

						// Check whether the child is stoppable.
						if(get_stoppable_status(pid) == STOPPABLE){
							pinfo->flags |= FLAG_STOPPABLE;
						}
						break;
					}
				}
			}

			if(pinfo->flags & FLAG_STOPPABLE){
				ptree_info->stoppableList[ptree_info->stoppableListIndex] = pid;
				ptree_info->stoppableListIndex++;
			}
		}
	}
	else{
		trie_remove(pid,ptree);
	}
}

int update_proc_tree(trie_root* ptree){
	if(ptree == NULL || ptree->num_nodes == 0 || ptree->next_one == NULL || ptree->next_zero == NULL || ptree->data == NULL){
		perror("Tree is empty or invalid. You gotta create it first!");
		return -1;
	}
	proc_tree_info* ptree_info = (proc_tree_info*) ptree->data;
	char explicitExcludeList = ptree_info->explicitExcludeList;
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

			void* proc = (void*) proc_info_new(0, FLAG_MARK_OK | FLAG_MARK_NEW);
			void* v = trie_insert(parentList[i], &proc);

			// Failure at inserting
			if(v != proc){
				free(proc);
				v->flags |= FLAG_MARK_OK;
			}
			// Success at inserting
			else if(!explicitExcludeList || !explicitParentList){
				ppid = get_ppid_of(pid);
				proc->ppid = ppid;
			}
			num_pids++;
		}
	}
	closedir(procDir);

	// Reallocate the stoppable list if necessary.
	if(num_pids > ptree_info->stoppableListSize){
		free(ptree_info->stoppableList);
		ptree_info->stoppableList = (int*) malloc(sizeof(int)*num_pids);
	}

	// Iterate through the PIDs
	trie_iterate(update_proc_tree_item, ptree);
	ptree_info->stoppableList[ptree_info->stoppableListIndex] = NULL;
	
	return 0;
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
