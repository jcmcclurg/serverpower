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

char         *progname;

const char   *version = "2.2";
uint64_t      num_node = 0;
uint64_t      delay_us = 1000000;
double        duration = 3600.0;
double        delay_unit = 1000000.0;
char	      thz0[6] = "xxxxx"; //thermalzone0 temperature (Celsius) -- code added by Joe Hall 4/19/15
char          thz1[6] = "xxxxx"; //thermalzone1 temperature (Celsius) -- code added by Joe Hall 4/19/15

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
    int          err;
    double       total_energy_consumed;

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
convert_time_to_string(struct timespec tv, char* time_buf)
{
    time_t sec;
    long nsec;
    struct tm *timeinfo;
    char tmp_buf[15];

    sec = tv.tv_sec;
    timeinfo = localtime(&sec);
    nsec = tv.tv_nsec;

    strftime(tmp_buf, 15, "%H:%M:%S", timeinfo);
    sprintf(time_buf, "%s:%9ld",tmp_buf,nsec);
}

/*
void
convert_time_to_string(struct timeval tv, char* time_buf)
{
    time_t sec;
    int msec;
    struct tm *timeinfo;
    char tmp_buf[9];

    sec = tv.tv_sec;
    timeinfo = localtime(&sec);
    msec = tv.tv_usec/1000;

    strftime(tmp_buf, 9, "%H:%M:%S", timeinfo);
    sprintf(time_buf, "%s:%d",tmp_buf,msec);
}
*/
double
convert_time_to_sec(struct timespec tv) {
	double elapsed_time = (double)(tv.tv_sec) + ((double)(tv.tv_nsec)/1000000000);
	return elapsed_time;
}
/*
double
convert_time_to_sec(struct timeval tv)
{
    double elapsed_time = (double)(tv.tv_sec) + ((double)(tv.tv_usec)/1000000);
    return elapsed_time;
}
*/


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

    char time_buffer[32];
    //struct timeval tv;
	struct timespec tv, t_setpoint, t_realtime;    
	int msec;
    uint64_t tsc;
    uint64_t freq[4];
    double start, end, interval_start;
    double total_elapsed_time;
    double interval_elapsed_time;

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


	clock_gettime(CLOCK_REALTIME, &tv);    
	//gettimeofday(&tv, NULL);
    start = convert_time_to_sec(tv);
    end = start;

    gettimeofday((struct timeval *__restrict__) &tv, NULL);
    start = convert_time_to_sec(tv);
    end = start;
	 total_elapsed_time = end-start;

    while (total_elapsed_time < duration) {
        gettimeofday((struct timeval *__restrict__) &tv, NULL);
        end = convert_time_to_sec(tv);
        total_elapsed_time = end - start;
		  
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
							printf("setpoint = %.1f\n",setpoint);
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

                // timestamp setpoint using real-time clock
				clock_gettime(CLOCK_REALTIME, &t_setpoint);
				convert_time_to_string(t_setpoint, time_buffer);
    		} // end if(setpoint)
			else if (setpoint == -1) { // -1 for quit, therefore exit while()
				fprintf(stderr, "quit power_gadget\n"); // forward quit-command to pipe via stdout
				break;
			}
		}
    }
    
    end = clock();
}

void
usage()
{
    fprintf(stdout, "\nRAPL Power Limiter %s\n", version);
    fprintf(stdout, "\nUsage: \n");
    fprintf(stdout, "%s -d [duration (sec)]\n", progname);
    fprintf(stdout, "\nType p to set the package power limit.");
    fprintf(stdout, "\n");
}


int
cmdline(int argc, char **argv)
{
    int             opt;
    uint64_t    delay_ms_temp = 1000;
    progname = argv[0];

    while ((opt = getopt(argc, argv, "d:q")) != -1) {
        switch (opt) {
        case 'd':
            duration = atof(optarg);
            if(duration <= 0.0){
                fprintf(stdout, "Duration must be greater than 0 seconds.\n");
                return -1;
            }
            break;
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

//////////////////////////////////////////////////////////////////
// function: get_usr_input() 
// author: Joseph Hall 
// date: 5/27/15
// description: checks stdin buffer and reads if there are contents
// 		checks if '\n' recieved
//			if so, returns 1 and replaces '\n' with '\0' in buff
//			if not, returns 0 and shifts buff up by number of bytes rxd
// passes: 	buff - input char array
//	  		bytes_so_far - pointer to number of input bytes collected from stdin since last '\n'
// returns:	-1 error
// 			 0 no input or no '\n'
//			 1 input complete, '\n' received
// requires libraries: 
//       #include <stdio.h>
//       #include <stdlib.h>
//       #include <sys/time.h>
//       #include <sys/types.h>
//       #include <unistd.h>
//
// source: modified from select.2 manpage
// source license: 
/* 	This manpage is copyright (C) 1992 Drew Eckhardt, copyright (C) 1995 Michael Shields.
	Permission is granted to make and distribute verbatim copies of this
	manual provided the copyright notice and this permission notice are
	preserved on all copies.

	Permission is granted to copy and distribute modified versions of this
	manual under the conditions for verbatim copying, provided that the
	entire resulting derived work is distributed under the terms of a
	permission notice identical to this one.

	Since the Linux kernel and libraries are constantly changing, this
	manual page may be incorrect or out-of-date.  The author(s) assume no
	responsibility for errors or omissions, or for damages resulting from
	the use of the information contained herein.  The author(s) may not
	have taken the same level of care in the production of this manual,
	which is licensed free of charge, as they might when working
	professionally.

	Formatted or processed versions of this manual, if unaccompanied by
	the source, must acknowledge the copyright and authors of this work.
*/
////////////////////////////////////////////////////////////

int get_usr_input(char *buff, int *bytes_so_far) {
    
	fd_set rfds;
    struct timeval tv;
    int retval, len;

    /* Watch stdin (fd 0) to see when it has input. */
    FD_ZERO(&rfds);
    FD_SET(0, &rfds);
	/* Wait up to 1 useconds. */
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    /* check if stdin has input */
    retval = select(1, &rfds, NULL, NULL, &tv);

    if (retval == -1)
    	return -1;
    
    else if (retval) {
    	len = read(0,buff,255);
        if (buff[len-1]=='\n') {
            if (*bytes_so_far == 0) {
		    	buff[len-1]='\0'; // replace '\n' with '\0'
		    	//printf("Data received: %s\n",buff);
				}		
			else {
		    	//printf("Data received: %s\n",buff-bytes_so_far);		    	
				*bytes_so_far = 0;	
		    	buff -= *bytes_so_far;	
			}
			return 1;    	 
	   	}
		else {
			*bytes_so_far += len;
            buff += len;
	    }
	}        
	else {
		return 0;
   		//printf("No data within five seconds.\n");
	}
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

