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
	int class;
	int parent;
	int pid;
	int ppid;
} process_tree;


pid_t get_ppid_of(pid_t pid);
char get_status_of(pid_t pid);
int tag_proc_tree_nonchildren(int* nonChildren, int numNonChildren);
int update_proc_tree(process_tree** procTree, int* numProcs);
int update_children(	int** childNodes, int* numChildNodes, int* exclusions, int numExclusions,	char exclude_nonstoppable);
int tag_proc_tree_children(int* rootPIDs, int numRootPIDs);
void init_children(void);

void close_children(void);

char get_status_of(pid_t pid);
pid_t get_ppid_of(pid_t pid);


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
