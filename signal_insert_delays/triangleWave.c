#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define DEFMAXV 1.0
#define DEFMINV 0.0
#define DEFRES 25
#define DEFFREQ 0.5

char prefix_buffer[64];

double period;
double res;
double minv;
double maxv;
double deltav;
double deltat;
double dir;
double v;
useconds_t deltat_us;
char* progname;

void flip_dir(void){
	dir *= -1.0;
	deltav = dir*(maxv-minv)/res;
}

void update_params(double f,int r,double mnv,double mxv){
	period = 1.0/f;
	res = (double )r;
	minv = mnv;
	maxv = mxv;

	deltav = dir*(maxv-minv)/res;
	deltat = period/res;
	deltat_us = (useconds_t) (deltat*1000000.0);
	fprintf(stderr,"DeltaV = %f, DeltaT = %f\n",deltav,deltat);
}

void do_sleep(void){
	fprintf(stdout,"%s%f\n",prefix_buffer,v);
	usleep(deltat_us);
	v += deltav;
	if(v > maxv || v < minv){
		flip_dir();
		v += deltav;
	}
}

void usage(){
	fprintf(stdout, "\nTriangle wave generator\n");
	fprintf(stdout, "\nUsage: \n");
	fprintf(stdout, "%s [-f [frequency (Hz) ] optional] -r [samples per period] -n [minimum value] -x [maximum value] -p [prefix]\n", progname);
	fprintf(stdout, "\nExample: %s -e 1000 -d 10\n", progname);
	fprintf(stdout, "\n");
}

int cmdline(int argc, char **argv){
	int			 opt;
	double f = DEFFREQ;
	int r = DEFRES;
	double mnv = DEFMINV;
	double mxv = DEFMAXV;
	prefix_buffer[0] = 0;

	progname = argv[0];

	while ((opt = getopt(argc, argv, "f:r:n:x:p:")) != -1) {
		switch (opt) {
		case 'f':
			f = atof(optarg);
			break;
		case 'r':
			r = atoi(optarg);
			break;
		case 'n':
			mnv = atof(optarg);
			break;
		case 'x':
			mxv = atof(optarg);
			break;
		case 'h':
			usage();
			exit(EXIT_SUCCESS);
			break;
		case 'p':
			fprintf(stdout,"Hi.");
			sprintf(prefix_buffer,"%s",optarg);
			fprintf(stdout,"Hi.");
			break;
		default:
			usage();
			return -1;
		}
	}

	update_params(f,r,mnv,mxv);
	return 0;
}

int main(int argc, char* argv[]){
	v = 0.0;
	dir = 1.0;
	setbuf(stdout, NULL);


	if(cmdline(argc,argv)){
		exit(EXIT_FAILURE);
	}

	while(1){
		do_sleep();
	}
	return 0;
}
