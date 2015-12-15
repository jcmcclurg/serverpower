#ifndef cmdline_h
#define cmdline_h

#include "trie.h"
#include "get_children.h"

#define DEFAULT_DUTY 0.5
#define DEFAULT_PERIOD 0.01
#define DEFAULT_UPDATE_INTERVAL 0.0

typedef struct cmdline_opts_S {
	double duty;
	double period;
	double update_interval;
	char verbose;
	char update_on_error;
	proc_tree_info* ptree_info;
	trie_root* proc_tree;
	FILE* logfp;
	char* progname;
} cmdline_opts;

// Prints the usage to stdout.
void usage(void);

// Returns a reference to the static struct buffer on success. Retursn NULL on error.
cmdline_opts* cmdline(int argc, char** argv);

#endif
