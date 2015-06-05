#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define DEFMAXV 1.0
#define DEFMINV 0.0
#define DEFRES 25
#define DEFFREQ 0.5

double period;
double res;
double minv;
double maxv;
double deltav;
double deltat;
double dir;
double v;
useconds_t deltat_us;

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
	fprintf(stdout,"%f\n",v);
	usleep(deltat_us);
	v += deltav;
	if(v > maxv || v < minv){
		flip_dir();
		v += deltav;
	}
}

int main(int argc, char* argv[]){
	v = 0.0;
	dir = 1.0;
	setbuf(stdout, NULL);

	double f = DEFFREQ;
	int r = DEFRES;
	double mnv = DEFMINV;
	double mxv = DEFMAXV;

	if(argc > 1){
		f = atof(argv[1]);
		//fprintf(stderr,"%f,%s.\n",f,argv[1]);
		if(argc > 2){
			r = atoi(argv[2]);
			if(argc > 3){
				mnv = atof(argv[3]);
				if(argc > 4){
					mxv = atof(argv[4]);
				}
			}
		}
	}
	update_params(f,r,mnv,mxv);

	while(1){
		do_sleep();
	}
	return 0;
}
