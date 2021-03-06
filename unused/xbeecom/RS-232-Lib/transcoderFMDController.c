/**************************************************
File: transcoderFMDController.c 
Author: Joe Hall
Date: 7/23/15
Description: 
	opens virtual COM port at specified port 
	establishes communication with FMD using cmdline args	
	polls any bytes received from COM port
	converts received freq deviation and transcoder frames to a power setpoint and prints (stdout) result
	prints freq and timestamp to file	
	transmits user entered (stdin) bytes
	
compile with the command: gcc transcoderFMDController.c rs232.c -Wall -Wextra -o2 -o transcoderFMDController -lm
example run to only get freq: sudo ./transcoderFMDController -r 500 -p 16 -b 115200 -o test.csv
example run to get freq & setpoint: sudo ./transcoderFMDController -r 500 -d 1 -p 16 -b 115200 -M 35 -m 24 -a 1 -B 10000 -o test.csv

**************************************************/
#include <stdlib.h>
#include <stdio.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include <time.h>
#include "rs232.h"
#include <math.h>

char* progname;
int cport_nr=16,        /* ie /dev/ttyUSB0 (see doc.txt) */
    bdrate=115200,      /* ie 115200 baud */
	rate_ms = 1000,		/* milliseconds between transmissions of freq from FMD */
	min_power=-1,			/* maximum/minimum power setpoints to output to server */
	max_power=-1,
	max_freq;			/* maximum expected magnitude freq deviation */
char *filename;
int pwr_algorithm = 0;
long frame_buf_len = 1;
int deadlines = 0;		/* use deadlines (trancoder thru-put) in algorithm: 1=yes 0=no */

int get_usr_input(char *buff); // get stdin input
int RS232_GetComCommand(int cport_nr, unsigned char *com_buf); // get COM input
double convert_time_to_sec(struct timespec tv);
void convert_time_to_string(struct timespec tv, char* time_buf);

void usage(){
	fprintf(stdout, "\nOpens virtual COM port for RS232 at specified port & baudrate.\n");
	fprintf(stdout, "Program is hardcoded to 8N1, but can be changed. See RS232-Lib/doc.txt.\n");
	fprintf(stdout, "Prints input from COM port to stdout and sends stdin to COM port.\n");
	fprintf(stdout, "Type 'q <rtn>' to exit program. (any stdin with leading 'q' will quit program).\n");
	fprintf(stdout, "\nUsage: \n");
	fprintf(stdout, "%s [options]\n\nOptions:\n",progname);
	fprintf(stdout, "-a [0,1] turns power algorithm output on (1) or off (0)\n");
	fprintf(stdout, "-d [0,1] algorithm uses (d)eadlines: 1=yes 0=no\n");
	fprintf(stdout, "-r [milliseconds] (milliseconds b/n transmission of average freq by FMD)\n");
	fprintf(stdout, "-M [max_power] (maximum allowed output setpoint)\n");
	fprintf(stdout, "-m [min_power] (minimum allowed output setpoint)\n");
	fprintf(stdout, "-B [max frame buffer length (# frames)]\n");
	fprintf(stdout, "\n-p [port_number]\n");
	fprintf(stdout, "Available port numbers:\n##  Linux   Windows\n");
	fprintf(stdout, 
		"0   ttyS0   COM1\n"
		"1   ttyS1   COM2\n"
		"2   ttyS2   COM3\n"
		"3   ttyS3   COM4\n"
		"4   ttyS4   COM5\n"
		"5   ttyS5   COM6\n"
		"6   ttyS6   COM7\n"
		"7   ttyS7   COM8\n"
		"8   ttyS8   COM9\n"
		"9   ttyS9   COM10\n"
		"10  ttyS10  COM11\n"
		"11  ttyS11  COM12\n"
		"12  ttyS12  COM13\n"
		"13  ttyS13  COM14\n"
		"14  ttyS14  COM15\n"
		"15  ttyS15  COM16\n"
		"16  ttyUSB0   n.a.\n"
		"17  ttyUSB1   n.a.\n"
		"18  ttyUSB2   n.a.\n"
		"19  ttyUSB3   n.a.\n"
		"20  ttyUSB4   n.a.\n"
		"21  ttyUSB5   n.a.\n"
		"22  ttyAMA0   n.a.\n"
		"23  ttyAMA1   n.a.\n"
		"24  ttyACM0   n.a.\n"
		"25  ttyACM1   n.a.\n"
		"26  rfcomm0   n.a.\n"
		"27  rfcomm1   n.a.\n"
		"28  ircomm0   n.a.\n"
		"29  ircomm1   n.a.\n");
	fprintf(stdout, "\n-b [baudrate]\n");
	fprintf(stdout, "Available baudrates:\nLinux\tWindows\n");
	fprintf(stdout,
		"50  n.a.\n"
		"75  n.a.\n"
		"110   110\n"
		"134   n.a.\n"
		"150   n.a.\n"
		"200   n.a.\n"
		"300   300\n"
		"600   600\n"
		"1200  1200\n"
		"1800  n.a.\n"
		"2400  2400\n"
		"4800  4800\n"
		"9600  9600\n"
		"19200   19200\n"
		"38400   38400\n"
		"57600   57600\n"
		"115200  115200\n"
		"230400  128000\n"
		"460800  256000\n"
		"500000  500000\n"
		"576000  n.a.\n"
		"921600  n.a.\n"
		"1000000   1000000\n"
		"1152000   n.a.\n"
		"1500000   n.a.\n"
		"2000000   n.a.\n"
		"2500000   n.a.\n"
		"3000000   n.a.\n"
		"3500000   n.a.\n"
		"4000000   n.a.\n");
	fprintf(stdout, "\n-h [this help]\n");
	fprintf(stdout, "Example usage:\n"
		"    sudo ./fmd2server -r 500 -p 16 -b 115200 -o test.csv\n"
		"	 sudo ./fmd2server -r 500 -p 16 -b 115200 -M 35 -m 24 -a 1 -B 10000 -o test.csv\n");
	fprintf(stdout, "\n");
}

int cmdline(int argc, char *argv[]){
	int opt;
	progname = argv[0];
	while ((opt = getopt(argc, argv, "a:d:p:b:r:M:m:B:o:h")) != -1) {
		switch (opt) {
			case 'a':
				pwr_algorithm = (int)atof(optarg);
				break;
			case 'd':
				deadlines = (int)atof(optarg);
				break;
			case 'p':
				cport_nr = (int)atof(optarg);
				break;
			case 'b':
				bdrate = (int)atof(optarg);
				break;
			case 'r':
				rate_ms = (int)atof(optarg);
				break;
			case 'M':
				max_power = (int)atof(optarg);
				break;
			case 'm':
				min_power = (int)atof(optarg);
				break;
			case 'B':
				frame_buf_len = (long)atof(optarg);
				break;
			case 'o':
				filename=optarg;
				break;
			case 'h':
				usage();
				exit(EXIT_SUCCESS);
				break;
			default:
				usage();
				return -1;
		}
	}

	return 0;
}

int main(int argc, char *argv[])
{	
	if(cmdline(argc,argv)){
		exit(EXIT_FAILURE);
	}
	unsigned char msb = (rate_ms >> 8),
				  lsb = (rate_ms & 0x00FF);
	unsigned char setup_buf[]={'S','R',msb,lsb,'B'}; 
	/* Stop, Rate=, (MSB), (LSB)ms, Begin */	

	if (pwr_algorithm && (min_power==-1 ||max_power==-1)) {
		printf("must specify power range with -M & -m\n");
		usage();
		return EXIT_SUCCESS;
	}

	int i,n;
  	char com_buf[4096]; /* COM port buffer */
  	char mode[]={'8','N','1',0}; /* RS232 mode */
	char stdin_buf[256]; /* stdin buffer for user input or pipe */
	double setpoint;
	double last_freq, freq, freq_change = 0;
    long frames_in_buf = frame_buf_len/2;
	long buf_slope = frame_buf_len/10;

	struct timespec tv_time;
	char quit = 0;
	setbuf(stdin, NULL);
	setbuf(stderr, NULL);
	setbuf(stdout, NULL);

	double time_sec;	
	char time_str[256];
	static char last_str[6] = {0};

	FILE *fp, *fdp;
	if (filename!=NULL) {
		fp = fopen(filename, "w");
		if (fp!=NULL) {
			setbuf(fp, NULL);
		}	
	}
	else {
		fprintf(stdout,"No output file path specified.\n");
	}

	if (fp==NULL) {
		fprintf(stdout,"Error opening output file\n");
	}

	if(RS232_OpenComport(cport_nr, bdrate, mode)) /* Setup COM port */
  	{
   		printf("Can not open comport\n");
		return(0);
	}
	fdp = RS232_OpenPortFILE(cport_nr);
	
	/* setup FMD */
	for (i=0;i<(int)(sizeof(setup_buf)/sizeof(setup_buf[0]));i++) {
		RS232_SendByte(cport_nr, setup_buf[i]);

#ifdef _WIN32
    		Sleep(1000);
#else
    		usleep(1000000);  /* sleep for 100 milliSecond */
#endif

		//n=RS232_GetComCommand(cport_nr, com_buf);
		n = RS232_ReadComPortLine(cport_nr,com_buf,4095,fdp);		
		if (n>0) {
			if ((int)atof(com_buf)!=setup_buf[i]) {
				fprintf(stdout,"Failed to receive '%c' ACK from FMD\n"
				"Received '%s' instead\n",setup_buf[i], com_buf);
				i--;
			}
		}
		else {
			fprintf(stdout, "Failed to receive '%c' ACK from FMD\n"
			"Received no response from FMD\n",setup_buf[i]);
			i--;
		}
	}
	
	while (quit!=1) {
		/* get input from COM port, timestamp & print*/
		//n = RS232_GetComCommand(cport_nr, com_buf);
		n = RS232_ReadComPortLine(cport_nr,com_buf,4095,fdp);
		if (n == 6) {
			clock_gettime(CLOCK_REALTIME, &tv_time);
			convert_time_to_string(tv_time, time_str);
			//time_sec = convert_time_to_sec(tv_time);
			
			/* add hysteresis */		
			if (!pwr_algorithm) 
				fprintf(stdout,"%s\n",com_buf);	
			else {
				freq = atof(com_buf);
				freq_change = freq-last_freq;
				if ((freq_change > 1.0) || (freq_change < -1.0)) {


					//setpoint = (double)(max_power+min_power)/2+((max_power-min_power)/(60.02-59.98))*(freq-60000)/1000+30.0/frames_in_buf+30.0/(frames_in_buf-frame_buf_len);

					//setpoint = (double)(max_power+min_power)/2+((max_power-min_power)/(60.03-59.97))*(freq-60000)/1000+30*(exp(-1*frames_in_buf)-exp(frames_in_buf-frame_buf_len));
/*  
	y = A*{exp(-x)-exp(x-B)}
  
	   x=0
		 |
		 |	
	_ _ _\______ _ _ _ _y=0
				\
		 		 \
		         |
				 |
				 x=B

*/		

					setpoint = (double)(max_power+min_power)/2+((max_power-min_power)/(60.03-59.97))*(freq-60000)/1000;
				}
				else {
					setpoint = (double)(max_power+min_power)/2+((max_power-min_power)/(60.03-59.97))*(last_freq-60000)/1000;
				}
				if (deadlines==1) { // adapt alg if considering deadline constraints
					if (frames_in_buf > (frame_buf_len-buf_slope)) {
						setpoint -= (buf_slope-(frame_buf_len-frames_in_buf))*(max_power-min_power)/buf_slope;
					}
					else if (frames_in_buf < buf_slope) {
						setpoint += (buf_slope-frames_in_buf)*(max_power-min_power)/buf_slope;
					}
				}	
				if (setpoint > (double)max_power)
					setpoint = max_power;
				if (setpoint < (double)min_power)
					setpoint = min_power;
									
				//fprintf(stdout,"p%.1f\n%s\n",setpoint,com_buf);
				fprintf(stdout,"s%.1f\n",setpoint);
				last_freq = freq;	
			}

			if (fp != NULL) {
				/* only print data to file if it is different from last time */
				//if (strcmp(last_str,com_buf)!=0) {	
				if (1) {				
					//fprintf(fp,"%f,%s,\n",time_sec,(char *)com_buf);
					if (pwr_algorithm) {
						fprintf(fp,"%s,%s,%.1f,%ld,\n",time_str,com_buf,setpoint,frames_in_buf);
					}
					else {
						fprintf(fp,"%s,%s,\n",time_str,com_buf);
					}
				}		
			}
			strcpy(last_str,com_buf);			
		}
		/* get stdin or pipe input from user and transmit via COM port */
		n = get_usr_input(stdin_buf);
		if (n==1) {
			frames_in_buf = (int)atof(stdin_buf);
			/* if frames_in_buf received, save and do not send to COM port */
			if (frames_in_buf!=0) {
				n=0;
				//printf("frames in buf: %ld\n",frames_in_buf);
			}
			if (n==1) {
				RS232_SendByte(cport_nr, (unsigned char)stdin_buf[0]);
				if (stdin_buf[0]!='q') {
					//printf("sent: '%c'\n", stdin_buf[0]);
				}
				if (stdin_buf[0]=='q') {			
					RS232_SendByte(cport_nr, 'S'); /* send 's' for stop FMD*/
					//printf("sent: 'S' to stop FMD\nExiting\n");
#ifdef _WIN32
					Sleep(rate_ms);
#else
   					usleep(rate_ms*1000);  /* sleep for 10 milliSecond */
#endif
					//n = RS232_GetComCommand(cport_nr, com_buf);
					n = RS232_ReadComPortLine(cport_nr,com_buf,4095,fdp);

					RS232_CloseComport(cport_nr);
					if (fp!=NULL)
						fclose(fp);
					if (fdp!=NULL)
						RS232_ClosePortFILE(fdp);
					quit = 1;
				}				
			}
		}

#ifdef _WIN32
    		Sleep(rate_ms);
#else
    		usleep(rate_ms*1000);  /* sleep for 10 milliSecond */
#endif
	}

	return EXIT_SUCCESS;
}


int get_usr_input(char *buff) {
    
	fd_set rfds;
    struct timeval tv;
    int retval, len;
	static int bytes_so_far = 0;

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
            if (bytes_so_far == 0) {
		    	buff[len-1]='\0'; // replace '\n' with '\0'
		    	//printf("Data received: %s\n",buff);
				}		
			else {
		    	//printf("Data received: %s\n",buff-bytes_so_far);		    		
		    	buff -= bytes_so_far;
				bytes_so_far = 0;	
			}
			return 1;    	 
	   	}
		else {
			bytes_so_far += len;
            buff += len;
			return 0;
	    }
	}        
	else {
		return 0;
   		//printf("No data within five seconds.\n");
	}
}

double convert_time_to_sec(struct timespec tv) {
	double elapsed_time = (double)(tv.tv_sec) + ((double)(tv.tv_nsec)/1000000000);
	return elapsed_time;
}

void convert_time_to_string(struct timespec tv, char* time_buf)
{
    time_t sec;
    long nsec;
    struct tm *timeinfo;
    char tmp_buf[15];

    sec = tv.tv_sec;
    timeinfo = localtime(&sec);
    nsec = tv.tv_nsec;

    strftime(tmp_buf, 15, "%H:%M:%S", timeinfo);
    sprintf(time_buf, "%s.%9ld",tmp_buf,nsec);
}

int RS232_GetComCommand(int cport_nr, unsigned char *com_buf) {
	int n;
	int ret_val;
	static int bytes_so_far=0;	
	n = RS232_PollComport(cport_nr, com_buf, 4095);
    if(n > 0)
    {
		if (com_buf[n-1]=='\n'){
			ret_val = bytes_so_far+n;
			if (bytes_so_far == 0) {
				com_buf[n-1]='\0'; // replace '\n' with '\0'
			}
			else {
				com_buf -= bytes_so_far;
				bytes_so_far = 0;
			}
		}
		else {
			bytes_so_far += n;
			com_buf += n;
			ret_val = 0;
		}
	}
	else {
		ret_val = 0;
	}
	return ret_val;
}

