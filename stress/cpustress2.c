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
#include <ctype.h>

struct features
{
    bool running;
    int fieldsRead;
    char sleeplen[256];
};


float worker(void);

int main ()
{
    struct features ns;
    fd_set readfds;
    int    num_readable;
    struct timeval tv;
    int    fd_stdin;
    float val;
    float store[2];
    int i = 0;

    strcpy(ns.sleeplen,"0");
    ns.running = true;
    store[0] = 0;
    store[1] = 0;
    fd_stdin = fileno(stdin);
    val = atof(ns.sleeplen);
    printf("INSTRUCTIONS:\n");
    printf("1. please insert the time delay in a range of 0 to 500 \n");
    printf("2. to quit the program type q and press enter twice \n");
    printf("3. to access the previous delay time you typed press p and enter\n");
    printf("4. to make the CPU achieve its minimum performance press l and enter\n");
    printf("5. to make the CPU achieve its maximum performance press h and enter\n"); 
    printf("6. to show a map that contains increments of 5 percent from 20 to 100 percent,press m and enter\n");
    printf("7. to show the instructions again, press i and enter\n");

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
            usleep(100*store[1]);
		
        }
        else if (num_readable == 1)
        {
          ns.running = true;
		
            while(ns.running)
            {
	      fgets(ns.sleeplen,sizeof(ns.sleeplen),stdin);
		
		if(isalpha(ns.sleeplen[0]))
                {
     		   i = i+1;
		   val = store[0];
                  if(strcmp(ns.sleeplen,"q") == 10)
			{
			ns.fieldsRead = 1;
			 printf("q key pressed,please press enter\n");
			printf("please insert the time delay \n ");
			   return EXIT_SUCCESS;
			}
			
		  else if(strcmp(ns.sleeplen,"p") == 10)
			{
			store[1] = val;
			ns.fieldsRead = 1;
			printf("p key pressed\n");
	printf("Fields read: %d, Number read by main: %f\n", ns.fieldsRead, store[1]);
			printf("please insert the time delay \n ");
			break;
		 	}

			else if(strcmp(ns.sleeplen,"l")==10 )
			{
			ns.fieldsRead = 1;
			store[1] = 10000;
			ns.fieldsRead = 1;
			printf("l key pressed\n");
			printf("please insert the time delay \n ");
			break;
		 	}
			else if(strcmp(ns.sleeplen,"h")==10 )
			{
			ns.fieldsRead = 1;
			  store[1] = 0; 
			ns.fieldsRead = 1;
			printf("h key pressed\n");
			printf("please insert the time delay \n ");
				break;
			}
			else if(strcmp(ns.sleeplen,"m")==10 )
			{
			
			ns.fieldsRead = 1;
			int per[17] ={20,25,30,35,40,45,50,55,60,65,70,75,80,85,90,95,100};
			float time[17] = {280,190,120,98,73,52,42.7,33,28,18,13,10,7,4.2,1.9,0.55,0};
			int k;
			printf("CPU performance(percentage)        time delay\n");
			for(k=0;k<17;k++)
			{
			printf("             %d                        %f\n",per[k],time[k]);
			}	
		printf("Fields read: %d, Number read by main: %f\n", ns.fieldsRead, store[1]);
			printf("please insert the time delay \n ");
			   break;	
			}
		else if(strcmp(ns.sleeplen,"i")==10 )
			{
			store[1] = val;
			ns.fieldsRead = 1;
			 printf("INSTRUCTIONS:\n");
    printf("1. please insert the time delay in a range of 0 to 500 \n");
    printf("2. to quit the program type q and press enter twice \n");
    printf("3. to access the previous delay time you typed press p and enter\n");
    printf("4. to make the CPU achieve its minimum performance press l and enter\n");
    printf("5. to make the CPU achieve its maximum performance press h and enter\n"); 
    printf("6. to show a map that contains increments of 5 percent from 20 to 100 percent\n");
    printf("7. to show the instructions again\n\n");
	printf("Fields read: %d, Number read by main: %f\n", ns.fieldsRead, store[1]);
	printf("please insert the time delay \n ");
			break;
			}
			
		   else
		 {
		ns.fieldsRead = 0;
		printf("please only type positive numbers or any key command\n");
		printf("Fields read: %d, Number read by main: %f\n", ns.fieldsRead, store[1]);
		printf("please insert the time delay \n ");
		 break;
		 }
		
                }


             else if(isdigit(ns.sleeplen[0]))
	      {
		ns.fieldsRead = 1;
		store[0] = val;
		val = atof(ns.sleeplen);
                ns.fieldsRead = 1;
		val = atof(ns.sleeplen);
		store[1] = val;
                printf("Fields read: %d, Number read by main: %f\n", ns.fieldsRead, store[1]);
                printf("please insert the time delay \n ");
                break;
	       }  
  
		else
		{
			ns.fieldsRead = 0;
			printf("please only type positive numbers or any key command\n");
		printf("Fields read: %d, Number read by main: %f\n", ns.fieldsRead, store[1]);
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
