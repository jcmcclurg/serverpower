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

	double delay_sec = 2;
	int delay_us = delay_sec*1000000;
	double num_steps = 10; // number of step inputs
	//double dur_sec = 60; // test duration	
	double pmax = 35;
	double pmin = 20;
    double step_height = (pmax-pmin)/num_steps;
	int i;
	struct timespec tv;
	char time_buffer[32];

	//open output file
	FILE	*fp = NULL; 
	fp=fopen("data/setpoint.csv", "w");

	for (i=0;i<num_steps+1;i++){
		fprintf(stdout, "p%.2f\n",pmin);
		clock_gettime(CLOCK_MONOTONIC, &tv);
		convert_time_to_string(tv, time_buffer);
		fprintf(fp, "%s,%.2f,\n",time_buffer,pmin);
		usleep(delay_us); 
		fprintf(stdout, "p%.2f\n",(double)i*step_height+pmin); 
		clock_gettime(CLOCK_MONOTONIC, &tv);
		convert_time_to_string(tv, time_buffer);
		fprintf(fp, "%s,%.2f,\n",time_buffer,(double)i*step_height+pmin);
		usleep(delay_us);
	}
	fprintf(stdout, "q\n"); 

	// close output file
    if (fp!=NULL)
        fclose(fp);

	return EXIT_SUCCESS;
}


void
convert_time_to_string(struct timespec tv, char* time_buf)
{
    time_t sec;
    int msec;
    struct tm *timeinfo;
    char tmp_buf[9];

    sec = tv.tv_sec;
    timeinfo = localtime(&sec);
    msec = tv.tv_nsec/1000000;

    strftime(tmp_buf, 9, "%H:%M:%S", timeinfo);
    sprintf(time_buf, "%s:%d",tmp_buf,msec);
}
