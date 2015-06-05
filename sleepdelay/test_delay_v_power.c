#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>

void convert_time_to_string(struct timespec tv, char* time_buf);

int main (void) {

	double delay_sec = 0.1;
	int delay_us = delay_sec*1000000;
	double num_steps = 1000; // number of step inputs
	double dmax = 0.1;
	double dmin = 0.0;
    double step_height = (dmax-dmin)/num_steps;
	int i;

	for (i=0;i<num_steps+1;i++){
		fprintf(stdout, "%.2f\n",(double)i*step_height+dmin); 
		usleep(delay_us);
	}
	fprintf(stdout, "quit pwrtest.c\n"); 

	// close output file
//    if (fp!=NULL)
//        fclose(fp);

	return EXIT_SUCCESS;
}
