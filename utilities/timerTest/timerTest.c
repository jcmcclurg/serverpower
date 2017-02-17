#include <time.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>

#define MODE_SEC 0
#define MODE_NANO 1
#define MODE_USEC 2

#define DEFPERIOD 3.0
#define DEFMODE MODE_USEC
#define CLOCK CLOCK_REALTIME
#define NUM_HIST_BINS 7

unsigned char sleepMode;
double period_double_seconds;

struct timespec prevTime;
useconds_t period_us;
struct timespec period_nanoseconds;
struct timespec period_nanoseconds_rem;
unsigned int period_seconds;
unsigned long minDiff = ULONG_MAX;
unsigned long maxDiff = 0;
unsigned long histogramBorders[] = {1000L,
												10000L,
												100000L,
												1000000L,
												10000000L,
												100000000L};
unsigned long histogramCounts[NUM_HIST_BINS] = {0};

char* progname;

void setupSleepFuncUSec(double period){
	period_us = (useconds_t) (period*1.0e6);
	period_nanoseconds.tv_sec = (time_t)(period_us / 1000000);
	period_nanoseconds.tv_nsec = ((long) (period_us % 1000000))*1000L;
}
void sleepFuncUSec(){
	usleep(period_us);
}
void setupSleepFuncSec(double period){
	period_seconds = (unsigned int) period;
	period_nanoseconds.tv_sec = (time_t) period_seconds;
	period_nanoseconds.tv_nsec = 0L;
}
void sleepFuncSec(){
	sleep(period_seconds);
}
void setupSleepFuncNano(double period){
	period_nanoseconds.tv_sec = (time_t) period;
	period_nanoseconds.tv_nsec = (long) ((period - ((double) period_nanoseconds.tv_sec))*1.0e9);
}
void sleepFuncNano(){
	nanosleep(&period_nanoseconds,&period_nanoseconds_rem);
}
void do_sleep( void (*sleepFunc)(void) ){
	struct timespec currentTime;

	clock_gettime(CLOCK, &prevTime);
	(*sleepFunc)();
	clock_gettime(CLOCK, &currentTime);

	long secDur = (long) (currentTime.tv_sec) - (long)(prevTime.tv_sec);
	long nsecDur = currentTime.tv_nsec - prevTime.tv_nsec;
	//if(((long) period_nanoseconds.tv_sec) > secDur && period_nanoseconds.tv_nsec < nsecDur){
		// Do the carry
	//	fprintf(stdout, "Timing error: %lds, %ldns\n", ((long) period_nanoseconds.tv_sec) - secDur - 1, period_nanoseconds.tv_nsec + 1000000000L - nsecDur);
	//} else {
	//	// No carry needed
	long secDiff = ((long) period_nanoseconds.tv_sec) - secDur;
	long nsecDiff = period_nanoseconds.tv_nsec - nsecDur;

	unsigned long diff = labs(secDiff*1000000000L + nsecDiff);
	char changed = 0;
	if(diff > maxDiff){
		maxDiff = diff;
		changed = 1;
	}
	if(diff < minDiff){
		minDiff = diff;
		changed = 1;
	}

	int i = 0;
	char histNeedsPrinting = 1;
	while( diff > histogramBorders[i] ){
		if(histogramCounts[i] > 0){
			histNeedsPrinting = 0;
		}
		i++;
	}
	int index = i;
	histogramCounts[index]++;
	for(i = index+1; i < NUM_HIST_BINS; i++){
		if(histogramCounts[i] > 0){
			histNeedsPrinting = 0;
		}
	}
	if(changed){
		fprintf(stdout,"Max: %ld, Min: %ld\n", maxDiff, minDiff);
	}
	if(changed || histNeedsPrinting){
		for(i = 0; i< NUM_HIST_BINS-1; i++){
			fprintf(stdout,"#%ld<%ld\t",histogramCounts[i],histogramBorders[i]);
		}
		fprintf(stdout,"#%ld>%ld\n",histogramCounts[NUM_HIST_BINS-1],histogramBorders[NUM_HIST_BINS-2]);
	}
	//}
}

void usage(){
	fprintf(stdout, "\nTriangle wave generator\n");
	fprintf(stdout, "\nUsage: \n");
	fprintf(stdout, "%s -p [period]\n", progname);
	fprintf(stdout, "\n");
}

int cmdline(int argc, char **argv){
	int			 opt;
	double p = DEFPERIOD;
	unsigned char m = DEFMODE;

	progname = argv[0];

	while ((opt = getopt(argc, argv, "p:m:")) != -1) {
		switch (opt) {
		case 'm':
			if( strcmp("SEC",optarg) == 0 ) {
				m = MODE_SEC;
			} else if( strcmp("USEC",optarg) == 0){
				m = MODE_USEC;
			} else if( strcmp("NANO",optarg) == 0){
				m = MODE_NANO;
			} else {
				usage();
				return -1;
			}
			break;
		case 'p':
			p = atof(optarg);
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

	period_double_seconds = p;
	sleepMode = m;
	return 0;
}

int main(int argc, char* argv[]){
	setbuf(stdout, NULL);

	if(cmdline(argc,argv)){
		exit(EXIT_FAILURE);
	}

	struct timespec resTime;
	clock_getres(CLOCK,&resTime);
	fprintf(stderr,"The period is %lf seconds with clock resolution: %lds, %ldns\n",period_double_seconds,(long) resTime.tv_sec,resTime.tv_nsec);
	void (*sleepFunc)(void);

	switch(sleepMode){
		case MODE_SEC:
			fprintf(stderr,"The timing mode is SEC\n");
			setupSleepFuncSec(period_double_seconds);
			sleepFunc = sleepFuncSec;
			break;
		case MODE_USEC:
			fprintf(stderr,"The timing mode is USEC\n");
			setupSleepFuncUSec(period_double_seconds);
			sleepFunc = sleepFuncUSec;
			break;
		case MODE_NANO:
			fprintf(stderr,"The timing mode is NANO\n");
			setupSleepFuncNano(period_double_seconds);
			sleepFunc = sleepFuncNano;
			break;
		default:
			exit(EXIT_FAILURE);
	}
	fprintf(stderr, "Actual period: %lds, %ldns\n", ((long) period_nanoseconds.tv_sec), period_nanoseconds.tv_nsec);

	while(1){
		do_sleep(sleepFunc);
	}
	return 0;
}
