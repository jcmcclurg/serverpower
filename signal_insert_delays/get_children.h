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
#include "get_children.h"

#define UNKNOWN_CLASS 0
#define CHILD_CLASS 1
#define NONCHILD_CLASS 2

typedef struct process_tree_struct {
	char marked;
	int  parent;
	int  pid;
	int  ppid;
} process_tree;

// Check if the process tree is up-to-date with /proc.
// Does not check parent-child relationships.
// Does not check whether the marks are valid.
int proc_tree_is_current(process_tree* ptree);

// Rebuild the process tree so that it contains all processes in /proc.
// Parent-child relationships are not linked.
// All processes are unmarked.
int update_proc_tree(process_tree** ptree);

// Link the existing process tree to contain correct parent-child relationships.
// Does not check /proc for new processes. (See update_proc_tree and proc_tree_is_current for that).
// Does not check whether the marks are valid.
int link_proc_tree(process_tree* ptree);

int mark_proc_tree(int* parentList, char explicitParentList,
						 int* excludeList, char explicitExcludeList);

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
