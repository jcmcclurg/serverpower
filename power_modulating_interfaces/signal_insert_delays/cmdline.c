#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include "trie.h"
#include "get_children.h"
#include "cmdline.h"

cmdline_opts _opts;

// Turn stoppable children of init (pid = 1) into parents.
// Clear all the child flags, since this only gets called with explicitParentList set.
int update_snapshot_pid(int pid, void* value, trie_root* root){
	proc_info* pinfo = (proc_info*) value;
	if(pinfo->flags & FLAG_STOPPABLE){
		pinfo->flags |= FLAG_PARENT;
	}
	pinfo->flags &= ~(FLAG_CHILD);
	return 1;
}

cmdline_opts* cmdline(int argc, char **argv){
	_opts.progname = argv[0];

	int* parentList;
	char explicitParentList;
	int* exclusionList;
	char explicitExclusionList;

	int i,j;
	char p_flag_pos = -1;
	int parentListLen = -1;
	char e_flag_pos = -1;
	int exclusionListLen = -1;

	_opts.update_interval = DEFAULT_UPDATE_INTERVAL;
	_opts.duty = DEFAULT_DUTY;
	_opts.verbose = 0;
	_opts.update_on_error = 0;

	explicitParentList = 0;
	explicitExclusionList = 0;
	parentList = NULL;
	exclusionList = NULL;

	_opts.proc_tree = NULL;
	_opts.logfp = NULL;
	pid_t my_pid = getpid();

	char opt = 0;
	j = 0;
	for(i = 1; i < argc; i++){
		if(argv[i][0] == '-'){
			opt = argv[i][1];

			if(p_flag_pos > 0){
				parentListLen = i - p_flag_pos - 1;
			}
			if(e_flag_pos > 0){
				exclusionListLen = i - e_flag_pos - 1;
			}

			if(opt == 'p'){
				p_flag_pos = i;
			}
			else if(opt == 'o'){
				explicitParentList = 1;
			}
			else if(opt == 'e'){
				e_flag_pos = i;
			}
			else if(opt == 'x'){
				explicitExclusionList = 1;
			}
			else if(opt == 'l'){
				if(i+1 >= argc ){
					fprintf(stderr,"Please specify filename for logfile.\n");
					usage();
					return NULL;
				}
				else{
					_opts.logfp = fopen(argv[i+1],"w");
					if(_opts.logfp == NULL){
						fprintf(stderr,"Couldn't open %s.\n",argv[i+1]);
						return NULL;
					}
				}
			}
			else if(opt == 'v'){
				_opts.verbose = 1;
			}
			else if(opt == 'U'){
				_opts.update_on_error = 1;
			}
			else if(opt == 'h'){
				usage();
				exit(EXIT_SUCCESS);
			}
			else if(opt == 'u'){
				if(i+1 >= argc || sscanf(argv[i+1],"%lf",&_opts.update_interval) != 1) {
					fprintf(stderr,"update: %d, %lf\n",i+1,_opts.update_interval);
					usage();
					return NULL;
				}
			}
			else if(opt == 'd'){
				if(i+1 >= argc || sscanf(argv[i+1],"%lf",&_opts.duty) != 1 ){
					fprintf(stderr,"duty: %d, %lf\n",i+1,_opts.duty);
					usage();
					return NULL;
				}
			}
			else{
				fprintf(stderr,"Invalid command line argument -%c\n",opt);
				usage();
				return NULL;
			}
			j++;
		}
	}
	//trie_set_verbose(_opts.verbose);
	proc_tree_set_verbose(_opts.verbose);

	if(e_flag_pos > 0){
		if(exclusionListLen < 0){
			exclusionListLen = i - e_flag_pos - 1;
		}

		if(exclusionListLen == 0){
			fprintf(stderr,"You have to specify an exclusion list.\n");
			usage();
			return NULL;
		}
	}

	if(p_flag_pos > 0){
		if(parentListLen < 0){
			parentListLen = i - p_flag_pos - 1;
		}

		if(parentListLen == 0){
			fprintf(stderr,"You have to specify a parent list.\n");
			usage();
			return NULL;
		}
	}

	if(e_flag_pos < 0 && explicitExclusionList){
		fprintf(stderr,"You have to specify an exclusion list to use the -x option.\n");
		usage();
		return NULL;
	}

	if(e_flag_pos > 0){
		exclusionList = (int*) malloc(sizeof(int)*(exclusionListLen + 2));
		for(i = 0; i < exclusionListLen; i++){
			exclusionList[i] = atoi(argv[i + e_flag_pos + 1]);
		}
		exclusionList[i] = 0;
	}
	else{
		exclusionList = (int*) malloc(sizeof(int)*2);
		exclusionList[0] = my_pid;
		exclusionList[1] = 0;
	}

	if(p_flag_pos > 0){
		parentList = (int*) malloc(sizeof(int)*(parentListLen + 1));
		for(i = 0; i < parentListLen; i++){
			parentList[i] = atoi(argv[i + p_flag_pos + 1]);
		}
		parentList[i] = 0;
	}

	else{
		parentList = (int*) malloc(sizeof(int)*2);
		parentList[0] = 1;
		parentList[1] = 0;
		// First, get a tree with all the children of init
		_opts.proc_tree = create_proc_tree(parentList,0,exclusionList,explicitExclusionList);
		_opts.ptree_info = (proc_tree_info*) _opts.proc_tree->data;
		if(update_proc_tree(_opts.proc_tree)){
			perror("Failed to update proc tree (1).");
			free(parentList);
			free(exclusionList);
			return NULL;
		}

		// Take a snapshot of the currently modifyable processes, and make that the parent list.
		if(explicitParentList){
			// Then, update all the stoppable nodes to be parent nodes
			trie_iterate(update_snapshot_pid, _opts.proc_tree);

			// Then, reset the info of the tree to be correct.
			_opts.ptree_info->explicitParentList = explicitParentList;
			free(parentList);
			parentList = (int*) malloc(sizeof(int)*(_opts.ptree_info->stoppableListSize));
			memcpy(parentList,_opts.ptree_info->stoppableList,sizeof(int)*(_opts.ptree_info->stoppableListSize));
		}
	}

	LOG("Update interval: %g\n",_opts.update_interval);
	LOG("Duty cycle: %g\n",_opts.duty);
	LOG("Period: %g\n",_opts.period);
	LOG("Parent list (");
	if(explicitParentList){
		LOG("these and only these):\n");
	}
	else{
		LOG("these and their children):\n");
	}
	i = 0;
	while(parentList[i] > 0){
		LOG("   %d\n",parentList[i++]);
	}

	LOG("Exclusion list (");
	if(explicitExclusionList){
		LOG("these and only these):\n");
	}
	else{
		LOG("these and their children):\n");
	}
	i = 0;
	while(exclusionList[i] > 0){
		LOG("   %d\n",exclusionList[i++]);
	}

	if(_opts.proc_tree == NULL){
		_opts.proc_tree = create_proc_tree(parentList, explicitParentList, exclusionList, explicitExclusionList);
		_opts.ptree_info = (proc_tree_info*) _opts.proc_tree->data;
		if(update_proc_tree(_opts.proc_tree)){
			perror("Failed to update proc tree (2).");
			free(parentList);
			free(exclusionList);
			return NULL;
		}
	}

	LOG("Stoppable list (buffer size: %d, index: %d): ", _opts.ptree_info->stoppableListSize, _opts.ptree_info->stoppableListIndex);
	if(_opts.ptree_info->stoppableList != NULL){
		i = 0;
		while(_opts.ptree_info->stoppableList[i] > 0){
			LOG("%d ",_opts.ptree_info->stoppableList[i++]);
		}
	}
	LOG("\n");

	free(parentList);
	free(exclusionList);

	if(_opts.verbose)
		print_proc_tree(_opts.proc_tree);

	return &_opts;
}
