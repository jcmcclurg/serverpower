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

char *progname;
double packagePower;
double cpuPower;
double dramPower;
const char *version = "2.2";

int do_set_power_limit(int num_node, double packageWatts, double cpuWatts, double dramWatts) {
	int i;
	int ret;

	pkg_rapl_power_limit_control_t pkg_plc;
	pp0_rapl_power_limit_control_t pp0_plc;
	dram_rapl_power_limit_control_t dram_plc;

	// set RAPL Power Limits: (added by Joe Hall 5/27/15)
	ret = 0;
	fprintf(stderr, "Setting RAPL power limits to (pkg=%g W,cpu=%g W,dram=%g W)\n",packageWatts,cpuWatts,dramWatts);
	for (i=0;i<num_node;i++) { // added by Joe Hall 4/25/15

		// The Package
		get_pkg_rapl_power_limit_control(i,&pkg_plc);
		if(packageWatts > 0.0){
			// Enable power clamping, and make sure the lock is disabled.
			pkg_plc.power_limit_watts_1 = packageWatts;
			pkg_plc.limit_enabled_1 = 1;

			// I'm not sure what these do, so I'm not messing with them.
			//pkg_plc.power_limit_watts_2 = packageWatts;
			//pkg_plc.limit_enabled_2 = 1;
			//pkg_plc.clamp_enabled_2 = 1;
		} else {
			pkg_plc.clamp_enabled_1 = 0;
		}
		pkg_plc.clamp_enabled_1 = 1;
		pkg_plc.lock_enabled = 0;
		ret += set_pkg_rapl_power_limit_control(i,&pkg_plc);

		// The CPU
		get_pp0_rapl_power_limit_control(i,&pp0_plc);
		if(cpuWatts > 0.0) {
			pp0_plc.power_limit_watts = cpuWatts;
			pp0_plc.clamp_enabled = 1;
		} else {
			pp0_plc.clamp_enabled = 0;
		}
		pp0_plc.limit_enabled = 1;
		pp0_plc.lock_enabled = 0;
		ret += set_pp0_rapl_power_limit_control(i, &pp0_plc);

		// The DRAM
		get_dram_rapl_power_limit_control(i,&pp0_plc);
		if(dramWatts > 0.0){
			dram_plc.power_limit_watts = dramWatts;
			dram_plc.clamp_enabled = 1;
		} else {
			dram_plc.clamp_enabled = 0;
		}
		dram_plc.limit_enabled = 1;
		dram_plc.lock_enabled = 0;
		ret += set_dram_rapl_power_limit_control(i, &dram_plc);
	}

	if (ret != 0)
		fprintf(stderr, "Error setting RAPL power limit controls\n");
	return ret
}

void usage()
{
		fprintf(stdout, "\nSets RAPL power capping MSRs %s\n", version);
		fprintf(stdout, "\nUsage: \n");
		fprintf(stdout, "%s -p packagePower -c cpuPower -d dramPower\n", progname);
		fprintf(stdout, "\nExample: %s -p 28.5\n", progname);
		fprintf(stdout, "\n");
}


int cmdline(int argc, char **argv)
{
	int opt;
	progname = argv[0];
	packagePower = -1.0;
	cpuPower = -1.0;
	dramPower = -1.0;

	while ((opt = getopt(argc, argv, "p:c:d:")) != -1) {
			switch (opt) {
			case 'p':
					packagePower = atof(optarg);
					break;
			case 'c':
					cpuPower = atof(optarg);
					break;
			case 'd':
					dramPower = atof(optarg);
					break;
			default:
					usage();
					return -1;
			}
	}
	return 0;
}

int main(int argc, char **argv) {
	int i = 0;
	int ret = 0;
	int num_node;

	// First init the RAPL library
	ret = init_rapl();
	if (ret) {
		fprintf(stdout, "Init failed (%d)!\n",ret);
		terminate_rapl();
		return ret;
	}
	num_node = get_num_rapl_nodes_pkg();

	ret = cmdline(argc, argv);
	if (ret) {
		terminate_rapl();
		return ret;
	}

	ret = do_set_power_limit();
	terminate_rapl();
	
	return ret;
}

