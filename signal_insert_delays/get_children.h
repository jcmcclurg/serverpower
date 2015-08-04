#ifndef get_children_h
#define get_children_h

#include <sys/vfs.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

#define FLAG_PARENT		1
#define FLAG_CHILD		2
#define FLAG_EXCLUDE		4
#define FLAG_STOPPABLE	8
#define FLAG_MARK_OK		16
#define FLAG_MARK_NEW	32

typedef struct proc_info_struct {
	pid_t ppid;
	char flags;
} proc_info;

typedef struct proc_tree_info_struct {
	char explicitParentList;
	char explicitExclusionList;
	int* stoppableList;
	int stoppableListIndex;
	int stoppableListSize;
} proc_tree_info;

proc_tree_info* proc_tree_info_new(char explicitParentList, char explicitExclusionList);

proc_info* proc_info_new(pid_t ppid, char flags);

/*
1. Iterate through parentList:
	Insert the node, marking it with FLAG_PARENT
2. Iterate through exclusionList:
	Insert the node, clear FLAG_PARENT, and set FLAG_EXCLUDE

returns the process tree
*/
trie_root* create_proc_tree(int* parentList, char explicitParentList, int* exclusionList, char explicitExcludeList);
void deallocate_proc_tree(trie_root* ptree);

/*
1. Iterate through /proc:
	If a node was inserted, mark it with FLAG_MARK_OK | FLAG_MARK_NEW
	If a node already exists, mark it with FLAG_MARK_OK
	If stoppablePIDs is not NULL, and stoppablePIDsSize < number of PIDs in /proc, free stoppablePIDs, and re-allocate it to be larger.
2. Iterate through ptree:
	If a node is not marked with FLAG_MARK_OK, remove the node
	If a node is marked with FLAG_MARK_OK, unset the flag.
	If a node is marked with FLAG_MARK_NEW, unset the flag and:
		If explicitExcludeList is unset, navigate up until reaching a node with FLAG_EXCLUDE. If you find one, set FLAG_EXCLUDE. If not and if explicitParentList is unset, navigate up until reaching a node with FLAG_PARENT or FLAG_CHILD. If you find one, set FLAG_CHILD and set FLAG_STOPPABLE if it's stoppable.
	If a node is marked with FLAG_STOPPABLE, add it to stoppablePIDs.
Return 0 on success, -1 on error.
*/
int update_proc_tree(trie_root* ptree);

// Gets the status of the pid
char get_status_of(pid_t pid);

// Gets the parent PID of the pid
pid_t get_ppid_of(pid_t pid);

// Gets the name of the pid
char* get_name_of(pid_t pid, char* buff, int size);

// Used to check whether the signal actually was accepted.
#define CHECK_EXTERNAL_RESTART_WAITLEN_US 5000
#define CHECK_CONT_WAITLEN_US 5000

// Return values for get_stoppable_status
#define INSUFFICIENT_PERMISSIONS_STOP 1
#define EXTERNALLY_RESTARTED 2
#define INSUFFICIENT_PERMISSIONS_CONT 4
#define DOES_NOT_CONTINUE 8
#define STOPPABLE 0
#define IS_PARENT 16
#define IS_SELF 32

// Checks whether pid is stoppable and restartable. See #defines above for return values.
int get_stoppable_status(pid_t pid);

#endif
