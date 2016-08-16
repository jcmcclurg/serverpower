#!/usr/bin/python

# -*- coding: utf-8 -*-
"""
Created on Fri Jan 08 09:57:53 2016

@author: Josiah
"""

import numpy as np
import matplotlib.pyplot as plt

from defaultGetPower import readPowerFile, rawFileToPowerFile

verbose = True

f = "experiments/idlepower.log"

blocklen = 50000
powerData = readPowerFile(f,blocklen,verbose)
#powerData = rawFileToPowerFile(f,f+"_%d.gz"%(blocklen),blocklen,verbose)
powerData = powerData[:, [0,3,4,5,6,7] ]
powerStartTime = powerData[0,0]
powerEndTime = powerData[-1,0]

total = np.sum(powerData[:,1:], axis=1)
time = powerData[:,0]-powerStartTime

plt.plot(time,total)
plt.title("Idle power")
plt.xlabel("Time (s)")
plt.ylabel("Power (W)")

print "Average idle power over %g seconds: %g"%(powerEndTime-powerStartTime,np.mean(total))