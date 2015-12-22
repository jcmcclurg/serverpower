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

char		 *progname;

const char   *version = "2.2";
uint64_t	  num_node = 0;
char		  thz0[6] = "xxxxx"; //thermalzone0 temperature (Celsius) -- code added by Joe Hall 4/19/15
char		  thz1[6] = "xxxxx"; //thermalzone1 temperature (Celsius) -- code added by Joe Hall 4/19/15

int get_usr_input(char *buff, int *bytes_so_far); // get stdin input
char interpret_power_limit_command(char *command, double *str); // interpret stdin input for power limit command

void get_CPU_temperature(char *thz0) // get temperature of thermal zones 0  -- code added by Joe Hall 4/19/15
{
	FILE *f; 
	char c; 
	//char thz0[5] = "xxxxx";
	int i0 = 0;
	f=fopen("/sys/class/thermal/thermal_zone0/temp","rt"); 
	while((c=fgetc(f))!=EOF){ 
		//printf("%c",c);
		thz0[i0]=c;
		i0++; 
		
	} 
	thz0[5]='\0';
	fclose(f); 
}

void print_rapl_control_info(uint64_t node)  // added by Joe Hall 4/25/15
{
   int err = 0;
   pkg_rapl_power_limit_control_t pkg_plc;
   pkg_rapl_parameters_t pkg_param;
   pp0_rapl_power_limit_control_t pp0_plc;
   //pp1_rapl_power_limit_control_t pp1_plc;
   dram_rapl_power_limit_control_t dram_plc;
   dram_rapl_parameters_t dram_param;
   
   uint64_t pp0_priority_level;
   uint64_t pp1_priority_level;

   err = get_pkg_rapl_power_limit_control(node, &pkg_plc);
   err += get_pkg_rapl_parameters(node, &pkg_param);
   err += get_pp0_rapl_power_limit_control(node, &pp0_plc);
   err += get_pp0_balance_policy(node, &pp0_priority_level);
   //err += get_pp1_rapl_power_limit_control(node, &pp1_plc);  //pp1 only on client systems
   //err += get_pp1_balance_policy(node, &pp1_priority_level);
   err += get_dram_rapl_power_limit_control(node, &dram_plc);  // dram only on server systems
   err += get_dram_rapl_parameters(node, &dram_param);

   if (err > 0){
   		printf("%d error(s) getting RAPL info.\n", err);
   }
   else {
	   printf("RAPL power limit control structure:\n"
		"PowerLimit1(W)=\t%f\tPowerLimit2(W)=\t%f\nWindow1(sec)=\t%f\tWindow2(sec)=\t%f\n"
		"Limit1ON=\t%d\tLimit2ON=\t%d\nClamp1ON=\t%d\tClamp2ON=\t%d\nLockON=\t%d\n\n"
		,pkg_plc.power_limit_watts_1,pkg_plc.power_limit_watts_2
		,pkg_plc.limit_time_window_seconds_1,pkg_plc.limit_time_window_seconds_2
		,(int) pkg_plc.limit_enabled_1, (int) pkg_plc.limit_enabled_2
		,(int) pkg_plc.clamp_enabled_1, (int) pkg_plc.clamp_enabled_2, (int) pkg_plc.lock_enabled);	

	   printf("RAPL Package Parameters:\n"	
		"ThermalSpecPower(W) = %f\tMaxWindow(sec) = %f\nMinimumPower(W) = %f\tMaximumPower(W) = %f\n\n"
	   		,pkg_param.thermal_spec_power_watts,pkg_param.maximum_limit_time_window_seconds
		,pkg_param.minimum_power_watts,pkg_param.maximum_power_watts);

	   printf("RAPL PowerPlane0 (Core) Power Limit Control:\n"
		"PowerLimit(W) =\t%f\tWindow(sec) =%f\nLimitON = %d\tClampON = %d\n"
		"LockON = %d\tPolicyLevel = %d\n\n"
		,pp0_plc.power_limit_watts, pp0_plc.limit_time_window_seconds, (int) pp0_plc.limit_enabled
		,(int) pp0_plc.clamp_enabled, (int) pp0_plc.lock_enabled, (int) pp0_priority_level);
/*
	   printf("RAPL PP1 Power Limit Control:\n"
		"PowerLimit(W) =\t%f\tWindow(sec) =%f\nLimitON = %d\tClampON = %d\n"
		"LockON = %d\tPolicyLevel = %d\n\n"
		,pp1_plc.power_limit_watts,pp1_plc.limit_time_window_seconds,pp1_plc.limit_enabled
		,pp1_plc.clamp_enabled,pp1_plc.lock_enabled,pp1_priority_level);	
*/  
	   printf("RAPL DRAM Power Limit Control:\n"
		"PowerLimit(W) =\t%f\tWindow(sec) =%f\nLimitON = %d\tClampON = %d\n"
		"LockON = %d\n\n"
		,dram_plc.power_limit_watts,dram_plc.limit_time_window_seconds, (int) dram_plc.limit_enabled
		,(int) dram_plc.clamp_enabled, (int) dram_plc.lock_enabled);

	   printf("RAPL DRAM Parameters:\nThermalSpecPower(W) = %f\tMaxWindow(sec) = %f\n" 
		 "MinimumPower(W) = %f\tMaximumPower(W) = %f\n\n"
	   	 ,dram_param.thermal_spec_power_watts,dram_param.maximum_limit_time_window_seconds
		 ,dram_param.minimum_power_watts,dram_param.maximum_power_watts);

	}

}

double
get_rapl_energy_info(uint64_t power_domain, uint64_t node)
{
	int		  err;
	double	   total_energy_consumed;

	switch (power_domain) {
	case PKG:
		err = get_pkg_total_energy_consumed(node, &total_energy_consumed);	
		break;
	case PP0:
		err = get_pp0_total_energy_consumed(node, &total_energy_consumed);
		break;
	case PP1:
		err = get_pp1_total_energy_consumed(node, &total_energy_consumed);
		break;
	case DRAM:
		err = get_dram_total_energy_consumed(node, &total_energy_consumed);
		break;
	default:
		err = MY_ERROR;
		break;
	}

	return total_energy_consumed;
}


void
do_print_energy_info()
{
	int i = 0;
	int domain = 0;
	uint64_t node = 0;
	double delta;
	double power;

	double new_sample[num_node][RAPL_NR_DOMAIN];;
	double prev_sample[num_node][RAPL_NR_DOMAIN];
	double power_watt[num_node][RAPL_NR_DOMAIN];
	double cum_energy_J[num_node][RAPL_NR_DOMAIN];
	double cum_energy_mWh[num_node][RAPL_NR_DOMAIN];

	uint64_t freq[4];

	// setup input command variables:
	int	rcvd = 0;
	char input[256];
	char *buff = &input[0];
	int	bytes_so_far = 0; 
	int ret;
	double setpoint = 0.0;
	char pp;

	pkg_rapl_power_limit_control_t pkg_plc;
	pkg_plc.power_limit_watts_1 = 80.0;
	pkg_plc.power_limit_watts_2 = 96.0;
	// printed rapl info: MaxWindow(sec) = 0.045898
	pkg_plc.limit_time_window_seconds_1 = 8.8;
	//pkg_plc.limit_time_window_seconds_2 = 0.007812;
	pkg_plc.limit_time_window_seconds_2 = 0.0001;
	pkg_plc.limit_enabled_1 = 1;
	pkg_plc.limit_enabled_2 = 1;
	pkg_plc.clamp_enabled_1 = 1;
	pkg_plc.clamp_enabled_2 = 1;
	pkg_plc.lock_enabled = 0;  

	pp0_rapl_power_limit_control_t pp0_plc;
	pp0_plc.power_limit_watts = 50.0;
	pp0_plc.limit_time_window_seconds = 0.0001;
	pp0_plc.limit_enabled = 1;
	pp0_plc.clamp_enabled = 1;
	pp0_plc.lock_enabled = 0;

	dram_rapl_power_limit_control_t dram_plc;
	dram_plc.power_limit_watts = 30.0;
	dram_plc.limit_time_window_seconds = 0.03;
	dram_plc.limit_enabled = 1;
	dram_plc.clamp_enabled = 1;
	dram_plc.lock_enabled = 0;

	/* don't buffer if piped */
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);
	setbuf(stdin, NULL);


	while (1) {
		rcvd = fscanf(stdin,"%s",buff); // get stdin input
		fprintf(stderr,"Got: (%s)\n",buff);

		if (rcvd == 1) {
			pp = interpret_power_limit_command(buff, &setpoint); // interpret stdin input for power limit command
			if (setpoint > 0) {
				if (pp=='s') {
					fprintf(stdout, "s%f\n", setpoint); // forward setpoint to pipe via stdout
				}
				else {
					for (i=0;i<num_node;i++) { // added by Joe Hall 4/25/15
						if (pp=='p') {
							pkg_plc.power_limit_watts_2 = setpoint;
							ret = set_pkg_rapl_power_limit_control(i,&pkg_plc);
							//fprintf(stderr, "setpoint = %0.1f, ret = %d\n",setpoint, ret);
						}
						else if (pp=='c') {
							pp0_plc.power_limit_watts = setpoint;
							ret = set_pp0_rapl_power_limit_control(i, &pp0_plc);
						}
						else if (pp=='d') {
							dram_plc.power_limit_watts = setpoint;
							ret = set_dram_rapl_power_limit_control(i, &dram_plc);
						}
						//print_rapl_control_info(i);
						if (ret != 0)
							fprintf(stderr, "Error setting RAPL power limit controls\n");
					} // end for(numnode)
				} //end if-else(pp)	

			} // end if(setpoint)
			else if (setpoint == -1) { // -1 for quit, therefore exit while()
				fprintf(stderr, "quit power_gadget\n"); // forward quit-command to pipe via stdout
				break;
			}
		}
	}
	
}

void
usage()
{
	fprintf(stdout, "\nRAPL Power Limiter %s\n", version);
	fprintf(stdout, "\nUsage: \n");
	fprintf(stdout, "\nType p to set the package power limit.");
	fprintf(stdout, "\n");
}


int
cmdline(int argc, char **argv)
{
	int			 opt;
	uint64_t	delay_ms_temp = 1000;
	progname = argv[0];

	while ((opt = getopt(argc, argv, "d:q")) != -1) {
		switch (opt) {
		case 'h':
			usage();
			exit(0);
			break;
		default:
			usage();
			return -1;
		}
	}
	return 0;
}

void sigint_handler(int signum)
{
	terminate_rapl();
	exit(0);
}

int
main(int argc, char **argv)
{
	int i = 0;
	int ret = 0;

	// original factory RAPL settings:
	pp0_rapl_power_limit_control_t pp0_plc_orig;
	pp0_plc_orig.power_limit_watts = 0.0;
	pp0_plc_orig.limit_time_window_seconds = 0.000977;
	pp0_plc_orig.limit_enabled = 0;
	pp0_plc_orig.clamp_enabled = 0;
	pp0_plc_orig.lock_enabled = 0;
	
	// original factory RAPL settings:
	pkg_rapl_power_limit_control_t pkg_plc_orig;
	pkg_plc_orig.power_limit_watts_1 = 80.0;
	pkg_plc_orig.power_limit_watts_2 = 96.0;
	pkg_plc_orig.limit_time_window_seconds_1 = 8.8;
	pkg_plc_orig.limit_time_window_seconds_2 = 0.007812;
	pkg_plc_orig.limit_enabled_1 = 1;
	pkg_plc_orig.limit_enabled_2 = 1;
	pkg_plc_orig.clamp_enabled_1 = 0;
	pkg_plc_orig.clamp_enabled_2 = 0;
	pkg_plc_orig.lock_enabled = 0;

	// original factory RAPL settings:
	dram_rapl_power_limit_control_t dram_plc_orig;
	dram_plc_orig.power_limit_watts = 0.0;
	dram_plc_orig.limit_time_window_seconds = 1.0;
	dram_plc_orig.limit_enabled = 0;
	dram_plc_orig.clamp_enabled = 0;
	dram_plc_orig.lock_enabled = 0;

	/* Clean up if we're told to exit */
	signal(SIGINT, sigint_handler);

	
	// First init the RAPL library
	if (0 != init_rapl()) {
		fprintf(stdout, "Init failed!\n");
		terminate_rapl();
		return MY_ERROR;
	}
	num_node = get_num_rapl_nodes_pkg();

	ret = cmdline(argc, argv);
	if (ret) {
		terminate_rapl();
		return ret;
	}

	// reset rapl power_limits to factory settings before do_print_energy_info()
	for (i=0;i<num_node;i++) { // added by Joe Hall 4/25/15
		//print_rapl_control_info(i);
		// reset rapl power_limits to factory settings
		//ret = set_pp0_rapl_power_limit_control(i,&pp0_plc_orig);
		if (ret > 0)
			fprintf(stdout, "Error setting PP0 power limit controls\n");
		//ret = set_pkg_rapl_power_limit_control(i,&pkg_plc_orig);
		if (ret > 0)
			fprintf(stdout, "Error setting PKG power limit controls\n");
	//	ret = set_dram_rapl_power_limit_control(i,&dram_plc_orig);
		if (ret > 0)
			fprintf(stdout, "Error setting DRAM power limit controls\n");
		//print_rapl_control_info(i);
	}
	
	// prints energy info from msr registers and also polls stdin for power_limiting commands
	 do_print_energy_info();

	
	// reset rapl power_limits to factory settings after do_print_energy_info()
	for (i=0;i<num_node;i++) { // added by Joe Hall 4/25/15
		//print_rapl_control_info(i);
		// reset rapl power_limits to factory settings
		ret = set_pp0_rapl_power_limit_control(i,&pp0_plc_orig);
		if (ret > 0)
			fprintf(stdout, "Error setting PP0 power limit controls\n");
		ret = set_pkg_rapl_power_limit_control(i,&pkg_plc_orig);
		if (ret > 0)
			fprintf(stdout, "Error setting PKG power limit controls\n");
	//	ret = set_dram_rapl_power_limit_control(i,&dram_plc_orig);
		if (ret > 0)
			fprintf(stdout, "Error setting DRAM power limit controls\n");
		//print_rapl_control_info(i);
	}

	terminate_rapl();

	/* close output file if FILE pointer was initialized */
}


char interpret_power_limit_command(char *command, double *str) {	
	switch (command[0]) {
		case ('p'):
			*str = atof(command+1);
			break;
		case ('c'):
			*str = atof(command+1);
			break;
		case ('d'):
			*str = atof(command+1);
			break;
		case ('s'):
			*str = atof(command+1);
			break;
		case ('q'):
			*str = -1;
			break;
		default:
			command = "ignore";
			*str = 0;
			break;
	}
	return command[0];
}

