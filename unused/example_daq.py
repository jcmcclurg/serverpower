# -*- coding: utf-8 -*-
"""
Created on Fri Apr 25 03:04:57 2014

this is based very loosely on example code from the PyDAQmx website

@author: Josiah McClurg
"""

from NI_DAQmx import *
import sys

class flushfile():
	def __init__(self, f):
		self.f = f

	def __getattr__(self,name):
		return object.__getattribute__(self.f, name)

	def write(self, x):
		self.f.write(x)
		self.f.flush()

# Make the output unbuffered. This doesn't seem to work always.
sys.stdout = flushfile(sys.stdout)

channels = []

for i in [0, 1, 2, 3, 4, 5, 6, 7, 16, 17, 18, 19, 20, 21, 22, 23]:
	c = ChannelSpec(device="JDAQ/ai%d"%(i), rangemin=-10, rangemax=10, name="v%d"%(i), termConf="diff")
	channels.append(c)

m = MultiChannelAnalogInput(channels,sampleRate=1000,debug=100)

raw_input("Press enter to start measurement")

startTime = time.time()
m.StartTask()

raw_input("Measuring. Press enter to stop")
#print "Measuring for 2min"
#time.sleep(120);

m.StopTask()
stopTime = time.time()

m.ClearTask()
dat = m.getData()

print "Saving data to data.mat"
dat["data_len"] = m.getDataLen()
dat["raw_samples"] = m.raw_samples
dat["startTime"] = startTime
dat["stopTime"] = stopTime
scipy.io.savemat('data.mat',dat,False,do_compression=True,oned_as='column')