#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM1 20000000

double d = 7.0;
int got_sigcont;

void sig_handler(int signo)
{
	if(signo == SIGCONT){
		if(!got_sigcont){
			fprintf(stdout,"\n");
			got_sigcont = 1;
		}
	}
	else if(signo == SIGINT){
		fprintf(stdout,"%f\n",d);
		exit(EXIT_SUCCESS);
	}
}

int main(void) {
	long l = 0;
	if(signal(SIGCONT,sig_handler) == SIG_ERR){
		fprintf(stderr,"Can't catch SIGCONT.\n");
		exit(EXIT_FAILURE);
	}
	else if(signal(SIGINT,sig_handler) == SIG_ERR){
		fprintf(stderr,"Can't catch SIGINT.\n");
		exit(EXIT_FAILURE);
	}
	got_sigcont = 0;

	setbuf(stdout, NULL);

	fprintf(stderr,"PID: %d\n",getpid());
	// A long long wait so that we can easily issue a signal to this process
	while(1){
		for(l = 0; l < NUM1; l++){
			d = 7.0*((double) l);
		}
		fprintf(stdout,".");

		if(got_sigcont)
			got_sigcont = 0;
	}
	exit(EXIT_SUCCESS);
	return 0;
}
