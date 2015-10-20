#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <math.h>
#include <float.h>

#include "commonFunctions.h"


char verbose;
double kvalue;
double tvalue;
double dvalue;
char* progname;
char* prefix;
FILE* fp;
char* logfile;
double setpoint;
double input;
double update_interval;
double maximum_output;
double minimum_output;

#define LOG(args...) if(verbose){ fprintf(stderr,args); } if(fp != NULL){ fprintf(fp,args); }

void usage(){
	fprintf(stdout, "\nIntegral controller prints a value according to the following formula:\n");
	fprintf(stdout, "      currentError = setpoint - input\n");
	fprintf(stdout, "      dt = currentTime - previousTime\n");
	fprintf(stdout, "      de = currentError - previousError\n");
	fprintf(stdout, "      di = currentError*dt\n");
	fprintf(stdout, "      output = k*currentError + t*(integral + di) + d*de/dt\n");
	fprintf(stdout, "      integral = (output - (k*currentError - d*de/dt))/t\n");
	fprintf(stdout, "      Note that the integral term is back-calculated to prevent integral windup.\n\n");
	fprintf(stdout, "  The setpoint is changed by typing s[setpoint] on a line by itself.\n");
	fprintf(stdout, "  The k value is changed by typing k[kvalue] on a line by itself.\n");
	fprintf(stdout, "  The t value is changed by typing t[tvalue] on a line by itself.\n");
	fprintf(stdout, "  The d value is changed by typing d[dvalue] on a line by itself.\n");
	fprintf(stdout, "  The input is changed by typing [input] on a line by itself.\n");
	fprintf(stdout, "\nUsage: \n");
	fprintf(stdout, "%s [options]\n",progname);
	fprintf(stdout, "   -s [initial setpoint (default: 0.0)]\n");
	fprintf(stdout, "   -i [initial input (default: 0.0)]\n");
	fprintf(stdout, "   -u [update interval in seconds (default: 1.0)]\n");
	fprintf(stdout, "   -k [k is the proportional gain (default: 0.0)]\n");
	fprintf(stdout, "   -t [t is the integral gain (default: 1.0]\n");
	fprintf(stdout, "   -d [d is the derivative gain (default: 0.0)]\n");
	fprintf(stdout, "   -h [this help]\n");
	fprintf(stdout, "   -v [verbose]\n");
	fprintf(stdout, "   -n [minimum output value (default:  1e37)]\n");
	fprintf(stdout, "   -x [maximum output value (default: -1e37)]\n");
	fprintf(stdout, "   -p [prefix (default: (none) )]\n");
	fprintf(stdout, "   -o [log file (default: (none))]\n");
	fprintf(stdout, "\n");
}

int cmdline(int argc, char **argv){
	int opt;
	kvalue = 0.0;
	tvalue = 1.0;
	dvalue = 0.0;

	verbose = 0;
	progname = argv[0];
	setpoint = 0.0;
	input = 0.0;
	update_interval = 1.0;
	minimum_output = -DBL_MAX;
	maximum_output = DBL_MAX;
	prefix = NULL;
	logfile = NULL;
	fp = NULL;

	while ((opt = getopt(argc, argv, "p:t:d:n:x:u:s:i:k:vho:")) != -1) {
		switch (opt) {
			case 'n':
				minimum_output = atof(optarg);
				break;
			case 'x':
				maximum_output = atof(optarg);
				break;
			case 'u':
				update_interval = atof(optarg);
				break;
			case 's':
				setpoint = atof(optarg);
				break;
			case 'i':
				input = atof(optarg);
				break;
			case 't':
				tvalue = atof(optarg);
				break;
			case 'd':
				dvalue = atof(optarg);
				break;
			case 'k':
				kvalue = atof(optarg);
				break;
			case 'h':
				usage();
				exit(EXIT_SUCCESS);
				break;
			case 'v':
				verbose = 1;
				break;
			case 'p':
				prefix = (char*) optarg;
				break;
			case 'o':
				logfile = (char*) optarg;
				fp = fopen(logfile, "w");
				if(!fp){
					exit(EXIT_FAILURE);
				}
				setbuf(fp,NULL);
				break;
			default:
				usage();
				return -1;
		}
	}

	return 0;
}

int main(int argc, char* argv[]) {
	// Register signal and signal handler
	signal(SIGINT, terminateProgram);
	setbuf(stdout,NULL);
	setbuf(stderr,NULL);

	char* str;

	double prevInput = input;
	double prevKValue = kvalue;
	double prevTValue = tvalue;
	double prevDValue = dvalue;
	double prevSetpoint = NAN;

	double prevTime;
	double currentTime;

	double integral = 0.0;
	double newOutput;

	if(cmdline(argc,argv)){
		exit(EXIT_FAILURE);
	}

	LOG("Initial setpoint: %lf\n",setpoint);
	LOG("Initial input: %lf\n",input);
	LOG("Update interval: %lf\n",update_interval);

	while(1) {
		if(checkStdin(update_interval) > 0){
			str = readLine();
			if(str != NULL){
				if(str[0] == 'q'){
					exit(EXIT_SUCCESS);
				}
				else if(str[0] == 's'){
					if(sscanf(str+1,"%lf",&setpoint) > 0 && prevSetpoint != setpoint){
						LOG("Setpoint updated from %g to %g\n",prevSetpoint, setpoint);
					}
				}
				else if(str[0] == 'd'){
					prevDValue = dvalue;
					if(sscanf(str+1,"%lf",&dvalue) > 0 && prevDValue != dvalue){
						LOG("dvalue updated from %g to %g\n",prevDValue, dvalue);
					}
				}
				else if(str[0] == 't'){
					prevTValue = tvalue;
					if(sscanf(str+1,"%lf",&tvalue) > 0 && prevTValue != tvalue){
						LOG("tvalue updated from %g to %g\n",prevTValue, tvalue);
					}
				}
				else if(str[0] == 'k'){
					prevKValue = kvalue;
					if(sscanf(str+1,"%lf",&kvalue) > 0 && prevKValue != kvalue){
						LOG("kvalue updated from %g to %g\n",prevKValue, kvalue);
					}
				}
				else{
					if(sscanf(str,"%lf",&input) > 0 && prevInput != input){
						LOG("Input updated from %g to %g\n",prevInput, input);
					}
				}
			}
		}

		prevTime = currentTime;
		currentTime = getCurrentTime();
		double prevError = setpoint - prevInput;
		double currentError = setpoint - input;
		double timeDelta = currentTime - prevTime;

		double derivative = (currentError - prevError)/timeDelta;
		double integralDelta = timeDelta*((currentError + prevError)/2.0);

		newOutput = kvalue*currentError + tvalue*(integralDelta + integral) + dvalue*derivative;
		if(prefix != NULL)
			fprintf(stdout,"%s",prefix);

		if(newOutput > maximum_output){
			newOutput =  maximum_output;
			LOG("output capped (max)\n");
		}
		else if(newOutput < minimum_output){
			newOutput = minimum_output;
			LOG("output capped (min)\n");
		}
		else {
			LOG("output: %lf\n",newOutput);
		}
		fprintf(stdout,"%lf\n",newOutput);

		// I don't understand the following:
		// integralDelta = (0.0075*currentError+(newOutput-oldOutput))*timeDelta/tvalue;
		// 
		// If you're trying to prevent integral windup, you can do it by limiting the accumulator
		// (as I did previously), or by back-calculating the integral term. That method is shown here:
		integral = (newOutput - kvalue*currentError - dvalue*derivative)/tvalue;

		prevSetpoint = setpoint;
		prevInput = input;
	}
	fclose(fp);
	exit(EXIT_SUCCESS);
	return 0;
}
