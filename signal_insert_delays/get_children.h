#ifndef get_children_h
#define get_children_h
typedef struct process_tree_struct {
	int class;
	int parent;
	int pid;
	int ppid;
} process_tree;

int get_children(int** childNodes, int rootPID);

void init_children(void);

void close_children(void);
#endif
