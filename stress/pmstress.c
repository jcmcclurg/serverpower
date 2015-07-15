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

int get_usr_input(char *buff, int *bytes_so_far); // get stdin input
char interpret_command(char *input, long double *output); // interpret stdin
float cpu_worker(int num_iter, struct timespec *delay_ptr);
char mem_worker(long num_bytes, long touch, struct timespec *delay_ptr);
double convert_time_to_sec(struct timespec tv);

int
main(int argc, char **argv)
{
	char input[256];
    char *buff = &input[0];
	int	bytes_so_far = 0;
    long bytes = 0;
	int delay_us = 1000000.0;
	char cmd_type;
	long double cmd_data;
	double retval;
	int rcvd = 0;
	int running = 1;
	struct timespec tv_start, tv_stop;
	struct timespec delay;
	double t_start, t_stop;
	int num_iter = 100000;
    long touch_bytes = 0;
	long counter = 0;
	
	delay.tv_sec = 0;
	delay.tv_nsec = 100000000;
	/* don't buffer if piped */
    setbuf(stdin, NULL);
	
	while (running == 1) {
		
		//clock_gettime(CLOCK_MONOTONIC, &tv_start);
		retval = cpu_worker(num_iter, &delay);
		//retval = mem_worker(bytes,touch_bytes, &delay);
		//clock_gettime(CLOCK_MONOTONIC, &tv_stop);
		//usleep(delay_us);
		//t_start = convert_time_to_sec(tv_start);
		//t_stop = convert_time_to_sec(tv_stop);
		//printf("seconds elapsed while running worker: %.18f\n",(t_stop-t_start));
		//usleep(10000);
		rcvd = get_usr_input(buff, &bytes_so_far); // get stdin input
		if (rcvd==1) {
			cmd_type = interpret_command(buff, &cmd_data);
			switch (cmd_type) {
				case ('m'):
					bytes = (long)cmd_data;
					break;
				case ('d'):
					delay.tv_sec = (int)(cmd_data/1000000000);
					delay.tv_nsec = (long)((cmd_data/1000000000-delay.tv_sec)*1000000000);
					break;
				case ('i'):
					num_iter = (int)cmd_data;
					break;
				case ('t'):
					touch_bytes = (long)cmd_data;
					break;
				case ('q'):
					running = 0;
					break;
				default:
					// ignore
					break;
			}
		}
	}
	return EXIT_SUCCESS;
}


float cpu_worker(int num_iter, struct timespec *delay_ptr)
{
    int t = 0;
    float r;
    int i;
    for(i=0; i<num_iter; i++)
    {
        r = rand() %100;
        r = sqrt(r);
        t = r;
    }

	nanosleep(delay_ptr, NULL);   
	return 0;
}

char mem_worker(long num_bytes, long touch, struct timespec *delay_ptr)
{
	char *str;
	str = (char*)malloc(num_bytes);
	if (num_bytes >= touch) {
		memset(str,0xFF,touch);
		nanosleep(delay_ptr, NULL);
		memset(str,0x00,touch);
	}
	else {
		memset(str,0xFF,num_bytes);
		nanosleep(delay_ptr, NULL);
		memset(str,0x00,num_bytes);
	}
	free(str);
	return EXIT_SUCCESS;
}

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

char interpret_command(char *input, long double *output) {	
	switch (input[0]) {
		case ('m'):
			*output = atof(input+1);
			break;
		case ('d'):
			*output = atof(input+1);
			break;
		case ('i'):
			*output = atof(input+1);
			break;
		case ('t'):
			*output = atof(input+1);
			break;
		case ('q'):
			*output = -1;
			break;
		default:
			//input = "ignore";
			*output = 0;
			break;
	}
	return input[0];
}

double
convert_time_to_sec(struct timespec tv) {
	double elapsed_time = (double)(tv.tv_sec) + ((double)(tv.tv_nsec)/1000000000);
	return elapsed_time;
}


