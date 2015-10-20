#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <math.h>

void convert_time_to_string(struct timespec tv, char* time_buf);

int main (void) {

	double delay_sec = 3;
	int delay_us = delay_sec*1000000;
	double num_steps = 60; // number of step inputs
	double dmax = 500;
	double dmin = 0;
    double step_height = (dmax-dmin)/num_steps;
	double i;
	int j;
	double d_array[]=
		{1000.0,750.0,500.0,400.0,300.0,250.0,200.0,150.0,125.00,100.00,
		80.0,60.0,50.0,40.0,35.0,30.0,25.0,20.0,15.0,13.0,
		11.0,10.0,9.0,8.0,7.0,6.0,5.0,4.0,3.0,2.0,
		1.5,1.0,0.5,0.0};
	//fprintf(stdout, "minimum delay set as: %.18f\n",dmin); 
	//for (i=0;i<num_steps+1;i++){
	for (j=0;j<(sizeof(d_array)/sizeof(double));j++){
		//fprintf(stdout, "%.18f\n",(double)i*step_height+dmin); 
		fprintf(stdout, "%.2f\n",d_array[j]); 
		//fprintf(stdout, "%.18f\n",pow(2.0,i)*dmin); 
		usleep(delay_us);
	}
	fprintf(stdout, "q\n"); 

	// close output file
//    if (fp!=NULL)
//        fclose(fp);

	return EXIT_SUCCESS;
}
