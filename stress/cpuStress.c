#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#define NUM_THREADS 4

struct features
{
    bool running;
    int sleeplen;
    int fieldsRead;
    int threadNum;
};

struct features ns_array[NUM_THREADS];
int threadNum;

float worker(void);


void *threadFunc(void *arg)
{
    struct features *my_data;
    bool my_running;
    int my_sleeplen;
    int my_fieldsRead;
    int my_threadNum;

    my_data = (struct features *) arg;
    my_running = my_data->running;
    my_sleeplen = my_data->sleeplen;
    my_fieldsRead = my_data->fieldsRead;
    my_threadNum = my_data->threadNum;

    printf("Fields read: %d, Number read by thread #%d: %d\n", my_fieldsRead, my_threadNum, my_sleeplen);
	 while(my_running)
	 {
		 worker();
		 sleep(my_sleeplen);
	 }
    return EXIT_SUCCESS;
}


int main (void)
{
    struct features ns;
    int n = 4;
    int i;
    fd_set readfds;
    int    num_readable;
    struct timeval tv;
    int    fd_stdin;
    pthread_t threads[NUM_THREADS];
	 threadNum = 0;

    ns.sleeplen = 1000;
    ns.running = true;

    fd_stdin = fileno(stdin);



    while(ns.running)
    {
        FD_ZERO(&readfds);
        FD_SET(fileno(stdin), &readfds);
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        printf("please insert the time delay \n ");
        fflush(stdout);
        num_readable = select(fd_stdin +1, &readfds, NULL, NULL, &tv);
        if (num_readable == 0)
        {
            printf("\nPerforming default action after %d seconds\n",5);
            //exit(1);
        }

        if (num_readable == 1)
        {
            ns.running = true;

            while(ns.running)
            {
                ns.fieldsRead = scanf("%d",&ns.sleeplen);
                if(ns.fieldsRead == 0)
                {
                    printf("Don't type things that aren't numbers!\n");
                    return EXIT_FAILURE;
                }

                if(ns.sleeplen<0)
                {
                    printf("please only type positive numbers\n");
                    return EXIT_FAILURE;
                }

                else
                {
                    for(i=0; i<NUM_THREADS; i++)
                    {
                        ns_array[i].running = ns.running;
                        ns_array[i].sleeplen = ns.sleeplen;
                        ns_array[i].fieldsRead = ns.fieldsRead;
                        ns_array[i].threadNum = threadNum++;

                        pthread_create(&threads[i], NULL, threadFunc,(void *) &ns_array[i]);
                    }
                    printf("Fields read: %d, Number read by main: %d\n", ns.fieldsRead, ns.sleeplen);
                    break;
                }
            }
        }
        //worker();
        //sleep(ns.sleeplen);
    }

    return EXIT_SUCCESS;
}



float worker(void)
{
    int t = 0;
    float r;
    int i;

    for(i=0; i<100000; i++)
    {
        r = rand() %100;
        r = sqrt(r);
        t = r;


    }
    return t;
}
