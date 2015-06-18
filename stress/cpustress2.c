#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>


struct features
{
    bool running;
    int sleeplen;
    int fieldsRead;
    
};


float worker(void);

int main (void)
{
    struct features ns;
    int n = 4;
    int i;
    fd_set readfds;
    int    num_readable;
    struct timeval tv;
    int    fd_stdin;
 

    ns.sleeplen = 0;
    ns.running = true;

    fd_stdin = fileno(stdin);

    printf("please insert the time delay \n ");
    while(ns.running)
    {
        FD_ZERO(&readfds);
        FD_SET(fileno(stdin), &readfds);
        tv.tv_sec = 0;
        tv.tv_usec = 0;
        fflush(stdout);
        num_readable = select(fd_stdin +1, &readfds, NULL, NULL, &tv);
        if (num_readable == 0)
        {
            //printf("\nPerforming default action after %d seconds\n",5);
            //exit(1);
            worker();
            sleep(ns.sleeplen);
        }
        else if (num_readable == 1)
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
                    
                    printf("Fields read: %d, Number read by main: %d\n", ns.fieldsRead, ns.sleeplen);
                    printf("please insert the time delay \n ");
                    break;
                }
		
            }
        }
       
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
