/*
Copyright (c) 2012, Intel Corporation

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

	* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
	* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
	* Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* Written by Martin Dimitrov, Carl Strickland */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>

#include "rapl.h"
#define RETRY_US 100
#define NUM_RETRIES 10

char		 *progname;
const char   *version = "2.2_josiah";
uint64_t	  num_node = 0;
uint64_t	  delay_us = 1000000;
char prefix_buffer[64];
char display_time;
char display_power;
char display_energy;

double convert_time_to_sec(struct timespec tv){
	double elapsed_time = ((double)(tv.tv_sec)) + (((double)(tv.tv_nsec))/1.0e9);
	return elapsed_time;
}

void print_timespec(char* msg, struct timespec ts){
	fprintf(stderr,"%s:\n",msg);
	fprintf(stderr,"\ttv_sec: %ld\n",ts.tv_sec);
	fprintf(stderr,"\ttv_nsec: %ld\n",ts.tv_nsec);
}

void do_print_energy_info(){
	double currentEnergy;
	double prevEnergy;
	double energyConsumed;

	double duration_sec;
	double prevTime_sec;
	double currentTime_sec;
	int err;
	int node = 0;

	struct timespec clockres;
	struct timespec currentTime;

	/* don't buffer if piped */
	setbuf(stdout, NULL);
	
	// CLOCK_BOOTTIME is CLOCK_MONOTONIC, but is suspend-aware.
	if(clock_getres(CLOCK_BOOTTIME, &clockres) || clock_gettime(CLOCK_BOOTTIME, &currentTime)){
		perror("Problem with time.");
		exit(EXIT_FAILURE);
	}
	print_timespec("Clock resolution",clockres);
	print_timespec("Current time",currentTime);
	currentTime_sec = convert_time_to_sec(currentTime);
	fprintf(stderr,"Current time (s): %f\n",currentTime_sec);

	err = 0;
	while(get_pkg_total_energy_consumed(node, &currentEnergy)){
		usleep(RETRY_US);
		err++;
		if(err > NUM_RETRIES){
			perror("Problem with reading energy consumed");
			exit(EXIT_FAILURE);
		}
	}
	fprintf(stderr,"Total energy consumed: %f\n", currentEnergy);
	if(display_time)
		fprintf(stderr,"Time (s) ");
	if(display_energy)
		fprintf(stderr,"Energy Buffer (J) ");
	if(display_power)
		fprintf(stderr,"Power (W)");
	fprintf(stderr,"\n");

	// Timed period
	while(1){
		usleep(delay_us);
		prevEnergy = currentEnergy;
		err = 0;
		while(get_pkg_total_energy_consumed(node, &currentEnergy)){
			usleep(RETRY_US);
			err++;
			if(err > NUM_RETRIES){
				perror("Problem with reading energy consumed");
				exit(EXIT_FAILURE);
			}
		}
		clock_gettime(CLOCK_BOOTTIME, &currentTime);

		prevTime_sec = currentTime_sec;
		currentTime_sec = convert_time_to_sec(currentTime);
		duration_sec = currentTime_sec - prevTime_sec;

		energyConsumed = currentEnergy - prevEnergy;
		if(energyConsumed < 0){
			energyConsumed += MAX_ENERGY_STATUS_JOULES;
		}

		if(prefix_buffer != NULL)
			fprintf(stdout,"%s",prefix_buffer)
		if(display_time)
			fprintf(stdout,"%f ",currentTime_sec);
		if(display_energy)
			fprintf(stdout,"%f ",currentEnergy);
		if(display_power)
			fprintf(stdout,"%f",energyConsumed/duration_sec);
		fprintf(stdout,"\n");
	}
}

void usage(){
	fprintf(stdout, "\nIntel(r) Power Gadget %s (Josiah version)\n", version);
	fprintf(stdout, "\nUsage: \n");
	fprintf(stdout, "%s\n", progname);
	fprintf(stdout, " -e [sampling delay (ms) ]\n");
	fprintf(stdout, " -p [prefix string]\n");
	fprintf(stdout, " -c [column indicators, which can be: \n");
	fprintf(stdout, "     t (time)\n");
	fprintf(stdout, "     e (energy)\n");
	fprintf(stdout, "     p (power)\n");
	fprintf(stdout, "     For example: -c tp ]\n");
	fprintf(stdout, "\n");
}


int cmdline(int argc, char **argv){
	int			 opt;
	int i = -1;
	uint64_t	delay_ms_temp = 1000;
	display_time = 0;
	display_power = 0;
	display_energy = 0;
	
	progname = argv[0];

	while ((opt = getopt(argc, argv, "e:p:c:")) != -1) {
		switch (opt) {
		case 'e':
			delay_ms_temp = atoi(optarg);
			if(delay_ms_temp >= 0) {
				delay_us = delay_ms_temp * 1000;
			} else {
				fprintf(stdout, "Sampling delay must be positive.\n");
				return -1;
			}
			break;
		case 'h':
			usage();
			exit(0);
			break;
		case 'p':
			sprintf(prefix_buffer,"%s",optarg);
			break;
		case 'c':
			i = 0;
			while(optarg[i] != 0){
				if(optarg[i] == 't')
					display_time = 1;
				else if(optarg[i] == 'e')
					display_energy = 1;
				else if(optarg[i] == 'p')
					display_power = 1;
				i++;
			}
			break;
		default:
			usage();
			return -1;
		}
	}
	if(i == -1){
		display_power = 1;
	}
	return 0;
}

void sigint_handler(int signum){
	terminate_rapl();
	exit(0);
}

void print_power_limit(pkg_rapl_power_limit_control_t* s){
		fprintf(stdout, "power limit 1: %f\n",s->power_limit_watts_1);
		fprintf(stdout, "limit time window seconds 1: %f\n",s->limit_time_window_seconds_1);
		fprintf(stdout, "limit enabled 1: %ld\n",s->limit_enabled_1);
		fprintf(stdout, "clamp enabled 1: %ld\n",s->clamp_enabled_1);
		fprintf(stdout, "power limit 2: %f\n",s->power_limit_watts_2);
		fprintf(stdout, "limit time window seconds 2: %f\n",s->limit_time_window_seconds_2);
		fprintf(stdout, "limit enabled 2: %ld\n",s->limit_enabled_2);
		fprintf(stdout, "clamp enabled 2: %ld\n",s->clamp_enabled_2);
		fprintf(stdout, "lock enabled: %ld\n",s->lock_enabled);
}

int main(int argc, char **argv){
	int i = 0;
	int ret = 0;
	pkg_rapl_power_limit_control_t s;

	/* Clean up if we're told to exit */
	signal(SIGINT, sigint_handler);

	if (argc < 2) {
		usage();
		terminate_rapl();
		return 0;
	}

	// First init the RAPL library
	if (0 != init_rapl()) {
		fprintf(stderr, "Init failed!\n");
		terminate_rapl();
		return MY_ERROR;
	}
	num_node = get_num_rapl_nodes_pkg();

	ret = cmdline(argc, argv);
	if (ret) {
		terminate_rapl();
		return ret;
	}

	do_print_energy_info();
	terminate_rapl();
}
