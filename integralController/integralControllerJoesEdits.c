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
double setpoint;
double input;
double update_interval;
double maximum_output;
double minimum_output;

void usage(){
	fprintf(stdout, "\nIntegral controller prints a value proportional to the integral of the error.\n");
	fprintf(stdout, "  The setpoint is changed by typing s[setpoint] on a line by itself.\n");
	fprintf(stdout, "  The k value is changed by typing k[kvalue] on a line by itself.\n");
	fprintf(stdout, "  The t value is changed by typing t[tvalue] on a line by itself.\n");
	fprintf(stdout, "  The d value is changed by typing d[dvalue] on a line by itself.\n");
	fprintf(stdout, "  The input is changed by typing [input] on a line by itself.\n");
	fprintf(stdout, "\nUsage: \n");
	fprintf(stdout, "%s [option]\n",progname);
	fprintf(stdout, "   -s [initial setpoint (default: 0.0)]\n");
	fprintf(stdout, "   -i [initial input (default: 0.0)]\n");
	fprintf(stdout, "   -u [update interval in seconds (default: 1.0)]\n");
	fprintf(stdout, "   -k [k is the proportional gain (default: 0.0)]\n");
	fprintf(stdout, "   -t [t is the integral gain (default: 1.0]\n");
	fprintf(stdout, "   -d [d is the derivative gain (default: 0.0)]\n");
	fprintf(stdout, "   -h [this help]\n");
	fprintf(stdout, "   -v [verbose]\n");
	fprintf(stdout, "   -n [minimum integral value (default:  1e37)]\n");
	fprintf(stdout, "   -x [maximum integral value (default: -1e37)]\n");
	fprintf(stdout, "   -p [prefix (default: (none) )]\n");
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

	while ((opt = getopt(argc, argv, "p:t:d:n:x:u:s:i:k:vh")) != -1) {
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
	FILE* fp;
	fp = fopen("/home/powerserver/joe/serverpower/transcoders/videos/iCstderr.log", "w");
	setbuf(fp,NULL);
	char* str;

	double prevInput = input;
	double prevKValue = kvalue;
	double prevTValue = tvalue;
	double prevDValue = dvalue;
	double prevSetpoint = NAN;

double newOutput = 0.0;//Joe
double limitedOutput = 0.0;//Joe

	double prevTime;
	double currentTime;

	double integral = 0.0;

	if(cmdline(argc,argv)){
		exit(EXIT_FAILURE);
	}

	if(verbose){
		fprintf(stderr,"Initial setpoint: %lf\n",setpoint);
		fprintf(stderr,"Initial input: %lf\n",input);
		fprintf(stderr,"Update interval: %lf\n",update_interval);
	}

	while(1) {
		if(checkStdin(update_interval) > 0){
			str = readLine();
			if(str != NULL){
				if(str[0] == 'q'){
					exit(EXIT_SUCCESS);
				}
				else if(str[0] == 's'){
					if(sscanf(str+1,"%lf",&setpoint) > 0 && prevSetpoint != setpoint){
						if(verbose)
							fprintf(stderr, "Setpoint updated from %g to %g\n",prevSetpoint, setpoint);
					}
				}
				else if(str[0] == 'd'){
					prevDValue = dvalue;
					if(sscanf(str+1,"%lf",&dvalue) > 0 && prevDValue != dvalue){
						if(verbose)
							fprintf(stderr, "dvalue updated from %g to %g\n",prevDValue, dvalue);
					}
				}
				else if(str[0] == 't'){
					prevTValue = tvalue;
					if(sscanf(str+1,"%lf",&tvalue) > 0 && prevTValue != tvalue){
						if(verbose)
							fprintf(stderr, "tvalue updated from %g to %g\n",prevTValue, tvalue);
					}
				}
				else if(str[0] == 'k'){
					prevKValue = kvalue;
					if(sscanf(str+1,"%lf",&kvalue) > 0 && prevKValue != kvalue){
						if(verbose)
							fprintf(stderr, "kvalue updated from %g to %g\n",prevKValue, kvalue);
					}
				}
				else{
					if(sscanf(str,"%lf",&input) > 0 && prevInput != input){
						if(verbose)
							fprintf(stderr, "Input updated from %g to %g\n",prevInput, input);
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
//Joe	double integralDelta = timeDelta*((currentError + prevError)/2.0);
double integralDelta = timeDelta*(currentError/2.0)+((limitedOutput-newOutput)/tvalue/2.0);//Joe
// limitedOutput-newOutput adds an "antiwindup" term, see p310 in Astrom's Computer Controlled Systems //Joe 

//Joe	if(prevSetpoint == setpoint){
//Joe		if(tvalue*(integral + integralDelta) > maximum_output)
//Joe			integral = maximum_output/tvalue;
//Joe		else if(tvalue*(integral + integralDelta) < minimum_output)
//Joe			integral = minimum_output/tvalue;
//Joe		else
				integral += integralDelta;

			newOutput = kvalue*currentError + integral*tvalue + dvalue*derivative;
limitedOutput = newOutput;//Joe

			if(prefix != NULL)
				fprintf(stdout,"%s",prefix);

			if(newOutput > maximum_output){
				fprintf(stdout,"%lf\n",maximum_output);
limitedOutput = maximum_output;//Joe
				if(verbose)
					fprintf(stderr, "output capped (max)\n");
			}
			else if(newOutput < minimum_output){
				fprintf(stdout,"%lf\n",minimum_output);
limitedOutput = minimum_output;//Joe
				if(verbose)
					fprintf(stderr, "output capped (min)\n");
			}
			else {
				fprintf(stdout,"%lf\n",newOutput);
				if(verbose)
					fprintf(stderr, "output: %lf\n",newOutput);
			}
fprintf(fp,"limited output: %.2f newOutput: %.2f\n",limitedOutput,newOutput);
//		}
//		else{
			//integral = 0.0;
//		}

		prevSetpoint = setpoint;
		prevInput = input;
	}
	fclose(fp);
	exit(EXIT_SUCCESS);
	return 0;
}
