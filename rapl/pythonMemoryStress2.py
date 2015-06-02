# -*- coding: utf-8 -*-
"""
Created on Tue Mar  3 23:34:35 2015

@author: josiah

See http://pymotw.com/2/multiprocessing/communication.html


Program flow is: 
    Start four worker threads
    For each worker thread, do the following
        Allocate an floating point array of size N (usage of ~8N bytes)
        Wait M seconds
        De-allocate array and repeat
    N and M are synchronized objects that the threads read from. 
    User can set M by typing a number (Ex: 1.5 or 0.001). 
    User can set N by typing "s" followed by a number (Ex: s12500000 or s500000)
"""

import multiprocessing
import numpy as np
import time

def worker(n,i):
    while n.running:
        a = np.ones(n.allocsize)
        time.sleep(n.sleeplen)
        del a

if __name__ == '__main__':
    mgr = multiprocessing.Manager()
    ns = mgr.Namespace()
    ns.running = True
    ns.sleeplen = 0.0001
    ns.allocsize = 8000
    n = 4
    
    np.random.seed()
    
    
    w = []
    for i in range(0,n):
        w.append(multiprocessing.Process(name="w%d"%(i), target=worker, args=(ns,i)))
    
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
                
            except ValueError:
                if a and (a[0] == 's'):
                    ns.allocsize = float(a[1:])
                    print "Updated allocate size: %s"%(a[1:])

    
    except EOFError:
        print "input is done"
            
    except KeyboardInterrupt:
        print "Exiting."
        
    finally:
        ns.running = False
        
        for i in range(0,n):
            w[i].join()
