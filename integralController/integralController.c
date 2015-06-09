#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <math.h>

#include "commonFunctions.h"

char verbose;
double gain;
char* progname;

void usage(){
	fprintf(stdout, "\nIntegral controller\n");
	fprintf(stdout, "\nUsage: \n");
	fprintf(stdout, "%s [-f [frequency (Hz) ] optional] -r [samples per period] -n [minimum value] -x [maximum value] -p [prefix]\n", progname);
	fprintf(stdout, "\nExample: %s -e 1000 -d 10\n", progname);
	fprintf(stdout, "\n");
}

int cmdline(int argc, char **argv){
	int opt;
	gain = 1.0;
	verbose = 0;
	progname = argv[0];

	while ((opt = getopt(argc, argv, "k:v")) != -1) {
		switch (opt) {
		case 'k':
			gain = atof(optarg);
			break;
		case 'h':
			usage();
			exit(EXIT_SUCCESS);
			break;
		case 'v':
			verbose = 1;
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

	char* str;
	char verbose = 0;
	double setpoint = NAN;
	double input = NAN;
	double error = NAN;
	double integral = 0.0;
	double currentTime = NAN;

	if(cmdline(argc,argv)){
		exit(EXIT_FAILURE);
	}

	do {
		str = readLine();
		if(str != NULL){
			double n;
			double newError;
			if(str[0] == 's'){
				n = atof(str+1);
				newError = n - input;
				if(isnormal(n))
					setpoint = n;
			}
			if(str[0] == 'q'){
				exit(EXIT_SUCCESS);
			}
			else{
				n = atof(str);
				newError = setpoint - n;
				if(isnormal(n))
					input = n;
			}

			if(!isnan(newError)){
				double prevTime = currentTime;
				currentTime = getCurrentTime();
				double timeDelta = currentTime - prevTime;
				double integralDelta = timeDelta*error;
				error = newError;

				if(verbose)
					fprintf(stderr,"Error is %f at time %f",error,currentTime);

				if(isnormal(timeDelta)){
					integral += integralDelta;
					if(verbose)
						fprintf(stderr," => integral + %f = %f\n",integralDelta,integral);
					fprintf(stdout,"%f\n",gain*integral);
				}
				else{
					if(verbose)
						fprintf(stderr,"\n");
				}
			}
		}
	} while(str != NULL);

	exit(EXIT_SUCCESS);
	return 0;
}
