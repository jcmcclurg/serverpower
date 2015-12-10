/*
Copyright (c) 2012, Intel Corporation

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

	* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
	* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
	* Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* Written by Martin Dimitrov, Carl Strickland
Modified summer 2015 by Josiah McClurg
*/

#include <aio.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>

#include "trie.h"
#include "get_children.h"
#include "cmdline.h"

#define CLOCKID CLOCK_REALTIME

#define STATE_FINISHED 2 
#define STATE_WORKING 1
#define STATE_SLEEPING 0

#define errExit(msg)	do { prepare_for_close(); perror(msg); exit(EXIT_FAILURE); } while (0)
#define LOG(args...) if(global_options->verbose){ fprintf(stderr,args); } if(global_options->logfp != NULL){ fprintf(global_options->logfp,args); }

#define SIG_INT SIGINT
#define SIG_UPDATE (SIGRTMIN+2)
#define SIG_TIMER SIGRTMIN
#define SIG_AIO (SIGRTMIN+1)
#define BUFSIZE 128

// This holds the global options that were specified at the command line
cmdline_opts* global_options;
char state;


void prepare_for_close(void){
   if(global_options->proc_tree != NULL){
      if(global_options->verbose)
         fprintf(stderr,"Restarting PIDs: ");

      if(global_options->ptree_info->stoppableList != NULL){
         int i = 0;
         while(global_options->ptree_info->stoppableList[i] > 0){
            LOG("%d ",global_options->ptree_info->stoppableList[i]);
            if(kill(global_options->ptree_info->stoppableList[i++],SIGCONT) == -1)
					fprintf(stderr,"Couldn't send SIGCONT to %d\n",global_options->ptree_info->stoppableList[i-1]);
         }
      }
      LOG("\n");

      trie_deallocate(global_options->proc_tree);
   }
}


// These are for catching the interrupt signal
struct sigaction sigint_action;
char sigint_count;

static void sigint_handler(int sig, siginfo_t *si, void *uc) {
	fprintf(stderr,"sigint\n");
	if(sigint_count < 2){
		state = STATE_FINISHED;
		sigint_count++;
	}
	else
		errExit("sigintcount");
}

void start_sigint(void){
	sigint_count = 0;
	sigint_action.sa_flags = SA_SIGINFO;
	sigint_action.sa_sigaction = sigint_handler;
	sigemptyset(&sigint_action.sa_mask);
	if (sigaction(SIG_INT, &sigint_action, NULL) == -1)
		errExit("sigint");
}

void _update_proc_tree(void){
	if(update_proc_tree(global_options->proc_tree))
		errExit("Couldn't update process tree.");
	else if(global_options->verbose){
		LOG("Updated process tree. ");
		LOG("Stoppable list: ");
		if(global_options->ptree_info->stoppableList != NULL){
			int i = 0;
			while(global_options->ptree_info->stoppableList[i] > 0){
				LOG("%d ",global_options->ptree_info->stoppableList[i++]);
			}
		}
		LOG("\n");
	}
}

// These are for periodically updating the process tree
struct sigaction updatetimer_action;
struct itimerspec updatetimerspec;
timer_t updatetimerid;

void set_update_interval(double ud){
	if(ud >= 0.0){
		global_options->update_interval = ud;
		updatetimerspec.it_value.tv_sec = (time_t) global_options->update_interval;
		updatetimerspec.it_value.tv_nsec = (long) ((global_options->update_interval - ((double) updatetimerspec.it_value.tv_sec))*1.0e9);

		/* Start the timer */
		updatetimerspec.it_interval.tv_sec = updatetimerspec.it_value.tv_sec;
		updatetimerspec.it_interval.tv_nsec = updatetimerspec.it_value.tv_nsec;
		if (timer_settime(updatetimerid, 0, &updatetimerspec, NULL) == -1)
			errExit("timer_settime");
	}
}

static void sigupdate_handler(int sig, siginfo_t *si, void *uc) {
	//fprintf(stderr,"sigupdate\n");
	_update_proc_tree();
}

void start_updatetimer(void){
	struct sigevent sev;

	updatetimer_action.sa_flags = SA_SIGINFO;
	updatetimer_action.sa_sigaction = sigupdate_handler;
	sigemptyset(&updatetimer_action.sa_mask);
	if (sigaction(SIG_UPDATE, &updatetimer_action, NULL) == -1)
		errExit("sigupdate");

	/* Create the timer */
	sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_signo = SIG_UPDATE;
	sev.sigev_value.sival_ptr = &updatetimerid;
	if (timer_create(CLOCKID, &sev, &updatetimerid) == -1)
		errExit("timer_create");

	set_update_interval(global_options->update_interval);
}

// These are for alternating back and forth between STATE_WORKING and STATE_SLEEPING
struct sigaction timer_action;
timer_t timerid;
struct itimerspec timer_on;
struct itimerspec timer_off;
char update_on_error_count;

void _update_timer(void){
	double ont = global_options->duty*global_options->period;
	double offt = (1.0-global_options->duty)*global_options->period;

	timer_on.it_value.tv_sec = (time_t) ont;
	timer_on.it_value.tv_nsec = (long) ((ont - ((double) timer_on.it_value.tv_sec))*1.0e9);
	timer_off.it_value.tv_sec = (time_t) offt;
	timer_off.it_value.tv_nsec = (long) ((offt - ((double) timer_off.it_value.tv_sec))*1.0e9);

	// Make sure the timer actually goes, even if the duty/period calculations don't work out right.
	if (timer_on.it_value.tv_sec == 0 && timer_on.it_value.tv_nsec <= 0){
		timer_on.it_value.tv_nsec = 1;
	}
	if (timer_off.it_value.tv_sec == 0 && timer_off.it_value.tv_nsec <= 0){
		timer_off.it_value.tv_nsec = 1;
	}
}

// Returns 0 on success, -1 on clipping at zero.
char set_period(double p){
	char r = 0;
	if(p < 0.0){
		p = 0.0;
		r = -1;
	}
	global_options->period = p;
	_update_timer();
	return r;
}

// Returns 0 on success, -1 on clipping at zero, 1 on clipping at 1.
char set_duty(double d){
	char r = 0;
	if(d < 0.0){
		d = 0.0;
		r = -1;
	}
	else if(d > 1.0){
		d = 1.0;
		r = 1;
	}
	global_options->duty = d;
	_update_timer();
	return r;
}

static void sigtimer_handler(int sig, siginfo_t *si, void *uc) {
	//fprintf(stderr,"sigtimer\n");
	int f = 0;
	int i = 0;
	if(state == STATE_WORKING){
		state = STATE_SLEEPING;
		// Tell the other process to stop working.
		if(global_options->ptree_info->stoppableList != NULL){
			while(global_options->ptree_info->stoppableList[i] > 0){
				f -= kill(global_options->ptree_info->stoppableList[i++],SIGSTOP);
				//LOG("STOP %d\n",global_options->ptree_info->stoppableList[i-1]);
			}
		}

	}
	else if(state == STATE_SLEEPING){
		state = STATE_WORKING;
		// Allow the other processes to do work.
		if(global_options->ptree_info->stoppableList != NULL){
			while(global_options->ptree_info->stoppableList[i] > 0){
				f -= kill(global_options->ptree_info->stoppableList[i++],SIGCONT);
				//LOG("CONT %d\n",global_options->ptree_info->stoppableList[i-1]);
			}
		}

	}
	
	if(global_options->update_on_error && f > 0 && update_on_error_count == 0){
		if(f > 0){
			if(update_on_error_count == 0){
				update_on_error_count = 1;
				_update_proc_tree();
			}
		}
		else{
			update_on_error_count = 0;
		}
	}

	if(state == STATE_WORKING){
		if (timer_settime(timerid, 0, &timer_on, NULL) == -1)
			errExit("timer_settime_on");
	}
	else if(state == STATE_SLEEPING){
		if (timer_settime(timerid, 0, &timer_off, NULL) == -1)
			errExit("timer_settime_off");
	}
}

void start_timer(void){
	struct sigevent sev;
	update_on_error_count = 0;

	/* Establish handler for timer signal */
	timer_action.sa_flags = SA_SIGINFO;
	timer_action.sa_sigaction = sigtimer_handler;
	sigemptyset(&timer_action.sa_mask);
	if (sigaction(SIG_TIMER, &timer_action, NULL) == -1)
		errExit("sigaction");

	/* Create the timer */
	sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_signo = SIG_TIMER;
	sev.sigev_value.sival_ptr = &timerid;
	if (timer_create(CLOCKID, &sev, &timerid) == -1)
		errExit("timer_create");

	/* Start the timer */
	state = STATE_WORKING;
	timer_on.it_interval.tv_sec = 0;
	timer_on.it_interval.tv_nsec = 0;
	timer_off.it_interval.tv_sec = 0;
	timer_off.it_interval.tv_nsec = 0;
	_update_timer();

	if (timer_settime(timerid, 0, &timer_on, NULL) == -1)
		errExit("timer_settime");
}

// These are for reading user input asynchronously
struct sigaction aio_action;
char stdin_buf[BUFSIZE];
struct aiocb my_aiocb;

void sigaio_handler(int signum, siginfo_t *info, void* uap){
	//fprintf(stderr,"sigaio\n");
	ssize_t r = aio_return(&my_aiocb);

	if(r == -1){
		LOG("I/O error\n");
	}
	else if(r == 0){
		LOG("null I/O");
	}
	else{
		char* b = (char*) my_aiocb.aio_buf;
		if(global_options->verbose)
			fprintf(stderr,"Got input: %s", b);
		double d;
		if(sscanf(b, "%lf", &d) > 0)
			set_duty(d);

		else if(b[0] == 'p' && sscanf(b+1,"%lf", &d) > 0)
			set_period(d);

		else if(b[0] == 'u' && sscanf(b+1,"%lf", &d) > 0)
			set_update_interval(d);

		else if(b[0] == 'q')
			state = STATE_FINISHED;

		else if(global_options->verbose)
			fprintf(stderr,"Could not parse.\n");

		if(aio_read(&my_aiocb) == -1)
			errExit("aio_read");
	}
}

void start_io(void){
	/* Create the handler for aio events */
	aio_action.sa_sigaction = sigaio_handler;
	aio_action.sa_flags = SA_SIGINFO;
	sigemptyset(&aio_action.sa_mask);
	sigaction(SIG_AIO, &aio_action, NULL);

	/* Zero out the structure. */
	memset(&stdin_buf,0,sizeof(stdin_buf));
	memset(&my_aiocb,0,sizeof(my_aiocb));

	/* Allocate a data buffer for the aiocb request */
	my_aiocb.aio_buf = stdin_buf;

	/* Initialize the necessary fields in the aiocb */
	my_aiocb.aio_fildes = fileno(stdin);
	my_aiocb.aio_nbytes = BUFSIZE;
	my_aiocb.aio_offset = 0;
	
	my_aiocb.aio_sigevent.sigev_notify = SIGEV_SIGNAL;
	my_aiocb.aio_sigevent.sigev_signo = SIG_AIO;
	my_aiocb.aio_sigevent.sigev_value.sival_int = 0;
	if(aio_read(&my_aiocb) == -1)
		errExit("aio_read");
}

int main(int argc, char **argv){

	/* don't buffer if piped */
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);

	/* Process command line arguments */
	global_options = cmdline(argc, argv);
	if(global_options == NULL)
		errExit("Invalid options");

	/* Cleanly handle Ctrl+C */
	start_sigint();

	/* Read user input */
	start_io();

	/* Alternate between working and sleeping */
	start_timer();

	/* Update the process tree */
	start_updatetimer();

	while(state != STATE_FINISHED){
		pause();
	}

	LOG("Goodbye.\n");
	prepare_for_close();
	exit(EXIT_SUCCESS);
	return 0;
}
