# -*- coding: utf-8 -*-
"""
Created on Wed Mar  4 14:46:47 2015

@author: josiah
"""
import multiprocessing
import numpy as np
import time
#import fuzzy_ctrl

def controlLoop(n):
    propFactor = 0.00
    derivFactor = 0.00000
    intFactor = 0.001
    
    lastTime = time.time() - n.sleepTime
    totalTime = 0
    
    err = 0.0
    lastErr = 0.0
    intErr = 0.0

    #delay = 1.0
    print "Worker started"
    while n.running:
       
        err = n.setPoint - n.actualPower
        t = time.time()
        dt = t - lastTime
        totalTime += dt
        intErr += err*dt
        
        controlVal = propFactor*err + derivFactor*(err - lastErr)/dt + intFactor*intErr
        lastErr = err
        lastTime = t
        #print "ControlVal: %g"%(-controlVal)
        print "%g"%(max(min(-controlVal,1),0))

#	#delay = delay*fuzzy_ctrl.fuzzy_ctrl(n)
#        delay = delay*10**((n.actualPower-n.setPoint)/100)
#        
#	delay = (max(min(delay,1),0)) 
#        #print "Delay: %g"%delay     
#        print "%g"%(max(min(delay,1),0))

        time.sleep(n.sleepTime)

mgr = multiprocessing.Manager()
ns = mgr.Namespace()
ns.running = True
ns.actualPower = 0
ns.setPoint = 7
ns.sleepTime = 0.1
ns.pMax=25.
ns.pMin=3.

np.random.seed()


w = multiprocessing.Process(name="w", target=controlLoop, args=(ns,))

print "Started worker."
w.start()

try:
    a = " "
    while a:
        a = raw_input()
        try:
            if(float(a) >= 0):
                ns.actualPower = float(a)
                print "Updated power: %s"%(a)

        except ValueError:
            if a and (a[0] == 's'):
                ns.setPoint = float(a[1:])
                print "Updated setpoint: %s"%(a[1:])
            elif (a[0] == 'q'):
                print "quit powerControlLoop.py"
                a = 0
#            else:
#                pass

except EOFError:
    print "input is done"
        
except KeyboardInterrupt:
    print "Exiting."

finally:
    ns.running = False
    w.join()
