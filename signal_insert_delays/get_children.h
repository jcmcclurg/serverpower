#ifndef get_children_h
#define get_children_h

#include <sys/vfs.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define UNKNOWN_CLASS 0
#define CHILD_CLASS 1
#define NONCHILD_CLASS 2

typedef struct process_tree_struct {
	int class;
	int parent;
	int pid;
	int ppid;
} process_tree;


/*
Gets the children of rootPID as a list.
Also returns the entire process tree.
Uses and re-uses buffers which get cleaned up in close_children.
*/
int get_children(int** childNodes, int* numChildren, process_tree** procTree, int* numProcs, int rootPID);


/*
Takes process_tree structure pointer (created by get_children), and a
list of child PIDs.

The children array is repopulated with the root nodes.
Returns the number of root nodes found.
*/
int get_roots(process_tree* procTree, int numProcs, int* children, int numChildren);

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
