# -*- coding: utf-8 -*-
"""
Created on Tue Mar  3 23:34:35 2015

@author: josiah

See http://pymotw.com/2/multiprocessing/communication.html
"""

import multiprocessing
import numpy as np
import time
import datetime

# Function: worker()
def worker(ns,i,jobs):
    while ns.running:
        time.sleep(ns.sleeplen)
        k += np.sqrt(np.random.random())
	jobs = i

# Main:
if __name__ == '__main__':

    start_datetime = datetime.datetime.now()
    start_sec = time.time()

    mgr = multiprocessing.Manager()
    ns = mgr.Namespace()
    ns.running = True
    ns.sleeplen = 10
    n = 4

    print "Opening delay.csv"
    f1=open('data/delay.csv','w+')
    f1.write("start date & time: %s\n"%(str(start_datetime)))
    f1.write("time,delay (sec)\n")


    np.random.seed()

    w = []
	jobs = list((0,0,0,0))

    for i in range(0,n):
        w.append(multiprocessing.Process(name="w%d"%(i), target=worker, args=(ns,i,jobs[i]))

    print "Main thread starting workers"
    for i in range(0,n):
        w[i].start()

    try:
        a = " "
        while a:
            a = raw_input()
            print a
            try:
                if(float(a) >= 0):
                    ns.sleeplen = float(a)
                    f1.write("%s,%s\n" %(str(time.time()),str(ns.sleeplen)))

            except ValueError:
                pass
                if (a[0] == 'q'):
                    print "quit pythonStress.py"
                    a = 0
 #               else:
 #                   pass


    except EOFError:
        print "input is done"

    except KeyboardInterrupt:
        print "Exiting."

	for i in range(0,n):
        w[i].start()

    finally:
        f1.close()
        print "Closed delay.csv"
        ns.running = False

        for i in range(0,n):
            w[i].join()
