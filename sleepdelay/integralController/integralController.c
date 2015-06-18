#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <math.h>

#include "commonFunctions.h"

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
	double gain = 1.0;

	if(argc > 1 && argv[1][0] == 'v'){
		verbose = 1;
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

	return EXIT_SUCCESS;
}
