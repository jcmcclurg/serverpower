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

FILE 	     *fp = NULL;  
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

void get_CPU_temperature(char *thz0,char *thz1) // get temperature of thermal zones 0 & 1 -- code added by Joe Hall 4/19/15
{
    FILE *f; 
    char c; 
    //char thz0[5] = "xxxxx";
    //char thz1[5] = "xxxxx";
    int i0 = 0;
    int i1 = 0;
    f=fopen("/sys/class/thermal/thermal_zone0/temp","rt"); 
    while((c=fgetc(f))!=EOF){ 
        //printf("%c",c);
        thz0[i0]=c;
        i0++; 
        
    } 
    thz0[5]='\0';
    fclose(f); 
/*
    f=fopen("/sys/class/thermal/thermal_zone1/temp","rt"); 
    while((c=fgetc(f))!=EOF){ 
        //printf("%c",c);
        thz1[i1]=c;
        i1++; 
    } 
    thz1[5]='\0';
    fclose(f); 
*/
    //printf("thermal zone0: %s\nthermal zone1: %s\n\n",thz0,thz1);
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
		,pkg_plc.limit_enabled_1,pkg_plc.limit_enabled_2
		,pkg_plc.clamp_enabled_1,pkg_plc.clamp_enabled_2,pkg_plc.lock_enabled);	

	   printf("RAPL Package Parameters:\n"	
		"ThermalSpecPower(W) = %f\tMaxWindow(sec) = %f\nMinimumPower(W) = %f\tMaximumPower(W) = %f\n\n"
       		,pkg_param.thermal_spec_power_watts,pkg_param.maximum_limit_time_window_seconds
		,pkg_param.minimum_power_watts,pkg_param.maximum_power_watts);

	   printf("RAPL PowerPlane0 (Core) Power Limit Control:\n"
		"PowerLimit(W) =\t%f\tWindow(sec) =%f\nLimitON = %d\tClampON = %d\n"
		"LockON = %d\tPolicyLevel = %d\n\n"
		,pp0_plc.power_limit_watts,pp0_plc.limit_time_window_seconds,pp0_plc.limit_enabled
		,pp0_plc.clamp_enabled,pp0_plc.lock_enabled,pp0_priority_level);
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
		,dram_plc.power_limit_watts,dram_plc.limit_time_window_seconds,dram_plc.limit_enabled
		,dram_plc.clamp_enabled,dram_plc.lock_enabled);

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

double
convert_time_to_sec(struct timeval tv)
{
    double elapsed_time = (double)(tv.tv_sec) + ((double)(tv.tv_usec)/1000000);
    return elapsed_time;
}


void
do_print_energy_info()
{
    int i = 0;
    int domain = 0;
    uint64_t node = 0;
    double new_sample;
    double delta;
    double power;

    double prev_sample[num_node][RAPL_NR_DOMAIN];
    double power_watt[num_node][RAPL_NR_DOMAIN];
    double cum_energy_J[num_node][RAPL_NR_DOMAIN];
    double cum_energy_mWh[num_node][RAPL_NR_DOMAIN];

    char time_buffer[32];
    struct timeval tv;
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
	pkg_plc.limit_time_window_seconds_1 = 8.8;
	pkg_plc.limit_time_window_seconds_2 = 0.007812;
	pkg_plc.limit_enabled_1 = 1;
	pkg_plc.limit_enabled_2 = 1;
	pkg_plc.clamp_enabled_1 = 1;
	pkg_plc.clamp_enabled_2 = 1;
	pkg_plc.lock_enabled = 0;  

    pp0_rapl_power_limit_control_t pp0_plc;
    pp0_plc.power_limit_watts = 50.0;
    pp0_plc.limit_time_window_seconds = 0.001;
    pp0_plc.limit_enabled = 1;
    pp0_plc.clamp_enabled = 1;
    pp0_plc.lock_enabled = 0;

	dram_rapl_power_limit_control_t dram_plc;
	dram_plc.power_limit_watts = 35.0;
    dram_plc.limit_time_window_seconds = 0.001;
    dram_plc.limit_enabled = 1;
    dram_plc.clamp_enabled = 1;
    dram_plc.lock_enabled = 0;
	

    /* if output FILE pointer is not initiated in main(), let it be standard output (Joe)*/
    if (fp==NULL) {
		fp = stdout;
    }
	
    /* don't buffer if piped */
    setbuf(fp, NULL);

    /* Print header */
    fprintf(fp, "System Time,RDTSC,Elapsed Time (sec),");
    for (i = node; i < num_node; i++) {
        fprintf(fp, "IA Frequency_%d (MHz),",i);
        if(is_supported_domain(RAPL_PKG))
            fprintf(fp,"Processor Power_%d (Watt),Cumulative Processor Energy_%d (Joules),Cumulative Processor Energy_%d (mWh),", i,i,i);
        if(is_supported_domain(RAPL_PP0))
            fprintf(fp, "IA Power_%d (Watt),Cumulative IA Energy_%d (Joules),Cumulative IA Energy_%d(mWh),", i,i,i);
        if(is_supported_domain(RAPL_PP1))
            fprintf(fp, "GT Power_%d (Watt),Cumulative GT Energy_%d (Joules),Cumulative GT Energy_%d(mWh)", i,i,i);
        if(is_supported_domain(RAPL_DRAM))
            fprintf(fp, "DRAM Power_%d (Watt),Cumulative DRAM Energy_%d (Joules),Cumulative DRAM Energy_%d(mWh),", i,i,i);
    }
 	//   fprintf(fp,"ThermalZone0(Celsius),ThermalZone1(Celsius),"); // Added by Joe Hall 4/19/15
    fprintf(fp,"ThermalZone0(Celsius),");
    fprintf(fp, "\n");
    
    /* Read initial values */
    for (i = node; i < num_node; i++) {
        for (domain = 0; domain < RAPL_NR_DOMAIN; ++domain) {
            if(is_supported_domain(domain)) {
                prev_sample[i][domain] = get_rapl_energy_info(domain, i);
            }
        }
    // debug
    //fprintf(fp, "debug\n");
    }

    gettimeofday(&tv, NULL);
    start = convert_time_to_sec(tv);
    end = start;

    /* Begin sampling */
    while (1) {

        usleep(delay_us);

        gettimeofday(&tv, NULL);
        interval_start = convert_time_to_sec(tv);
        interval_elapsed_time = interval_start - end;

        for (i = node; i < num_node; i++) {
            for (domain = 0; domain < RAPL_NR_DOMAIN; ++domain) {
                if(is_supported_domain(domain)) {
                    new_sample = get_rapl_energy_info(domain, i);
                    delta = new_sample - prev_sample[i][domain];

                    /* Handle wraparound */
                    if (delta < 0) {
                        delta += MAX_ENERGY_STATUS_JOULES;
                    }

                    prev_sample[i][domain] = new_sample;

                    // Use the computed elapsed time between samples (and not
                    // just the sleep delay, in order to more accourately account for
                    // the delay between samples
                    power_watt[i][domain] = delta / interval_elapsed_time;
                    cum_energy_J[i][domain] += delta;
                    cum_energy_mWh[i][domain] = cum_energy_J[i][domain] / 3.6; // mWh
                }
            }
        }

        gettimeofday(&tv, NULL);
        end = convert_time_to_sec(tv);
        total_elapsed_time = end - start;
        convert_time_to_string(tv, time_buffer);

        read_tsc(&tsc);
        fprintf(fp,"%s,%llu,%.4lf,", time_buffer, tsc, total_elapsed_time);
        for (i = node; i < num_node; i++) {
            //get_pp0_freq_mhz(i, &freq);
            get_pp0_freq_mhz(i, freq); //changed by Joe 4/25/15 to get 4 CPU freq
            fprintf(fp, "%u|%u|%u|%u,", freq[0],freq[1],freq[2],freq[3]);
            for (domain = 0; domain < RAPL_NR_DOMAIN; ++domain) {
                if(is_supported_domain(domain)) {
                    fprintf(fp, "%.4lf,%.4lf,%.4lf,",
                            power_watt[i][domain], cum_energy_J[i][domain], cum_energy_mWh[i][domain]);
                }
            }
        }
        get_CPU_temperature(thz0,thz1);
        fprintf(fp, "%s,",thz0);
        fprintf(fp, "\n");

        // check to see if we are done
        if(total_elapsed_time >= duration)
            break;

		// set RAPL Power Limits: (added by Joe Hall 5/27/15)
		rcvd = get_usr_input(buff, &bytes_so_far); // get stdin input
		if (rcvd == 1) {
			pp = interpret_power_limit_command(buff, &setpoint); // interpret stdin input for power limit command
			if (setpoint > 0) {
				for (i=0;i<num_node;i++) { // added by Joe Hall 4/25/15
						if (pp=='p') {
							pkg_plc.power_limit_watts_2 = setpoint;
							ret = set_pkg_rapl_power_limit_control(i,&pkg_plc);
						}
						else if (pp=='c') {
							pp0_plc.power_limit_watts = setpoint;
							ret = set_pp0_rapl_power_limit_control(i, &pp0_plc);
						}
                        else if (pp=='d') {
							dram_plc.power_limit_watts = setpoint;
							ret = set_dram_rapl_power_limit_control(i, &dram_plc);
						}
						//fprintf(stdout, "Setpoint = %f & char = %c\n", setpoint,pp);
						print_rapl_control_info(i);
        			if (ret != 0)
	    				fprintf(stdout, "Error setting RAPL power limit controls\n");
				}					
    		}
			else if (setpoint == -1) // -1 for quit, therefore exit while()
				break;
		}
		// else rcvd=-1 means perror("select") 
		
    }

    end = clock();

    /* Print summary */
    fprintf(stdout, "\nTotal Elapsed Time(sec)=%.4lf\n\n", total_elapsed_time);
    for (i = node; i < num_node; i++) {
        if(is_supported_domain(RAPL_PKG)){
            fprintf(stdout, "Total Processor Energy_%d(Joules)=%.4lf\n", i, cum_energy_J[i][RAPL_PKG]);
            fprintf(stdout, "Total Processor Energy_%d(mWh)=%.4lf\n", i, cum_energy_mWh[i][RAPL_PKG]);
            fprintf(stdout, "Average Processor Power_%d(Watt)=%.4lf\n\n", i, cum_energy_J[i][RAPL_PKG]/total_elapsed_time);
        }
        if(is_supported_domain(RAPL_PP0)){
            fprintf(stdout, "Total IA Energy_%d(Joules)=%.4lf\n", i, cum_energy_J[i][RAPL_PP0]);
            fprintf(stdout, "Total IA Energy_%d(mWh)=%.4lf\n", i, cum_energy_mWh[i][RAPL_PP0]);
            fprintf(stdout, "Average IA Power_%d(Watt)=%.4lf\n\n", i, cum_energy_J[i][RAPL_PP0]/total_elapsed_time);
        }
        if(is_supported_domain(RAPL_PP1)){
            fprintf(stdout, "Total GT Energy_%d(Joules)=%.4lf\n", i, cum_energy_J[i][RAPL_PP1]);
            fprintf(stdout, "Total GT Energy_%d(mWh)=%.4lf\n", i, cum_energy_mWh[i][RAPL_PP1]);
            fprintf(stdout, "Average GT Power_%d(Watt)=%.4lf\n\n", i, cum_energy_J[i][RAPL_PP1]/total_elapsed_time);
        }
        if(is_supported_domain(RAPL_DRAM)){
            fprintf(stdout, "Total DRAM Energy_%d(Joules)=%.4lf\n", i, cum_energy_J[i][RAPL_DRAM]);
            fprintf(stdout, "Total DRAM Energy_%d(mWh)=%.4lf\n", i, cum_energy_mWh[i][RAPL_DRAM]);
            fprintf(stdout, "Average DRAM Power_%d(Watt)=%.4lf\n\n", i, cum_energy_J[i][RAPL_DRAM]/total_elapsed_time);
        }
    }
    read_tsc(&tsc);
    fprintf(stdout,"TSC=%llu\n", tsc);
}

void
usage()
{
    fprintf(stdout, "\nIntel(r) Power Gadget %s\n", version);
    fprintf(stdout, "\nUsage: \n");
    fprintf(stdout, "%s [-e [sampling delay (ms) ] optional] -d [duration (sec)]\n", progname);
    fprintf(stdout, "\nExample: %s -e 1000 -d 10\n", progname);
    fprintf(stdout, "\n");
}


int
cmdline(int argc, char **argv)
{
    int             opt;
    uint64_t    delay_ms_temp = 1000;

    progname = argv[0];

    while ((opt = getopt(argc, argv, "e:d:")) != -1) {
        switch (opt) {
        case 'e':
            delay_ms_temp = atoi(optarg);
            if(delay_ms_temp > 50) {
                delay_us = delay_ms_temp * 1000;
            } else {
                fprintf(stdout, "Sampling delay must be greater than 50 ms.\n");
                return -1;
            }
            break;
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

    if (argc < 2) {
        usage();
        terminate_rapl();
        return 0;
    }

    
    /* ********************************
    /  open file to output data (Joe)
    / 	******************************/
    //fp=fopen("data.csv", "w");

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
        ret = set_pp0_rapl_power_limit_control(i,&pp0_plc_orig);
        if (ret > 0)
	    	fprintf(stdout, "Error setting PP0 power limit controls\n");
		ret = set_pkg_rapl_power_limit_control(i,&pkg_plc_orig);
        if (ret > 0)
	    	fprintf(stdout, "Error setting PKG power limit controls\n");
		ret = set_dram_rapl_power_limit_control(i,&dram_plc_orig);
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
		ret = set_dram_rapl_power_limit_control(i,&dram_plc_orig);
		if (ret > 0)
	    	fprintf(stdout, "Error setting DRAM power limit controls\n");
		//print_rapl_control_info(i);
    }

    terminate_rapl();

    /* close output file if FILE pointer was initialized */
    if (fp!=stdout)
        fclose(fp);
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
		case ('q'):
			*str = -1;
			break;
		default:
			break;
	}
	return command[0];
}

