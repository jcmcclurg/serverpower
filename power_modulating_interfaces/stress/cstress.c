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

#define CLOCKID CLOCK_REALTIME

#define SIG_INT SIGINT
#define SIG_TIMER SIGRTMIN
#define SIG_AIO (SIGRTMIN+1)

#define errExit(msg)	do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define DEFAULT_DUTY 0.5
#define DEFAULT_PERIOD 0.01

#define STATE_INPUT_READY 3
#define STATE_FINISHED 2 
#define STATE_WORKING 1
#define STATE_SLEEPING 0

#define BUFSIZE 128

char* progname;
char* id;
char* defaultId = "iterations:";

struct sigaction aio_action;
char stdin_buf[BUFSIZE];
struct aiocb my_aiocb;

struct sigaction timer_action;
double duty;
double period;
char verbose;

struct itimerspec timer_on;
struct itimerspec timer_off;

struct sigaction sigint_action;
char sigint_count;

timer_t timerid;
char state;


void usage(){
	fprintf(stdout, "\nUsage: %s \n", progname);
	fprintf(stdout, "   -v    verbose\n");
	fprintf(stdout, "   -p    modulation period in seconds default %g\n",DEFAULT_PERIOD);
	fprintf(stdout, "   -d    initial duty cycle (0 - 1) default %g\n",DEFAULT_DUTY);
	fprintf(stdout, "\nType a number to change the duty cycle.\n");
	fprintf(stdout, "Type p followed by a number to change the period.\n");
	fprintf(stdout, "\n");
}

void _update_duty(void){
	double ont = duty*period;
	double offt = (1.0-duty)*period;

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
	period = p;
	_update_duty();
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
	duty = d;
	_update_duty();
	return r;
}

int cmdline(int argc, char **argv){
	duty = DEFAULT_DUTY;
	period = DEFAULT_PERIOD;
	verbose = 0;
	progname = argv[0];
	id = defaultId;
	int opt;

	while ((opt = getopt(argc, argv, "i:p:d:vh")) != -1) {
		switch (opt) {
		case 'i':
			id = optarg;
		case 'p':
			set_period(atof(optarg));
			break;
		case 'd':
			set_duty(atof(optarg));
			break;
		case 'v':
			verbose = 1;
			break;
		case 'h':
			usage();
			exit(EXIT_SUCCESS);
			break;
		default:
			usage();
			return -1;
		}
	}
	_update_duty();
	return 0;
}

static void sigtimer_handler(int sig, siginfo_t *si, void *uc) {
	if(state == STATE_WORKING){
		state = STATE_SLEEPING;
		if (timer_settime(timerid, 0, &timer_off, NULL) == -1)
			errExit("timer_settime1");
	}
	else{ // if state == STATE_SLEEPING
		state = STATE_WORKING;
		if (timer_settime(timerid, 0, &timer_on, NULL) == -1)
			errExit("timer_settime2");
	}
}

void start_timer(void){
	struct sigevent sev;

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

	if (timer_settime(timerid, 0, &timer_on, NULL) == -1)
		errExit("timer_settime");
}

void sigaio_handler(int signum, siginfo_t *info, void* uap){
	if(aio_return(&my_aiocb) == -1){
		//errExit("aio_return");
		if(verbose)
			fprintf(stderr,"I/O error\n");
	}
	else{
		char* b = (char*) my_aiocb.aio_buf;
		if(verbose)
			fprintf(stderr,"Got input: %s", b);
		double d;
		if(sscanf(b, "%lf", &d) > 0)
			set_duty(d);

		else if(b[0] == 'p' && sscanf(b+1,"%lf", &d) > 0)
			set_period(d);

		else if(b[0] == 'q')
			state = STATE_FINISHED;

		else if(verbose)
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

static void sigint_handler(int sig, siginfo_t *si, void *uc) {
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
		errExit("sigaction");
}

int main(int argc, char **argv){

	/* don't buffer if piped */
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);

	/* Process command line arguments */
	cmdline(argc, argv);

	/* Cleanly handle Ctrl+C */
	start_sigint();
	start_io();
	start_timer();

	double r;
	double l;
	int i;
	long long iterations = 0;
	while(state != STATE_FINISHED){
		//if(verbose && state == STATE_WORKING)
		//	fprintf(stderr,"Working\n");
		while(state == STATE_WORKING){
			l = 0;
			for(i = 0; i < 1024; i++){
				r = sqrt(l++);
			}
			iterations++;
		}
		//if(verbose && state == STATE_SLEEPING)
		//	fprintf(stderr,"Sleeping\n");
		while(state == STATE_SLEEPING){
			pause();
		}
		//if(verbose && state == STATE_FINISHED)
		//	fprintf(stderr,"Finished\n");
	}
	if(verbose)
		fprintf(stderr,"Goodbye.\n");
	fprintf(stdout,"%s%lld\n",id,iterations);
	exit(EXIT_SUCCESS);
	return (int) r;
}
