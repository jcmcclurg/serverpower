# -*- coding: utf-8 -*-
"""
Created on Fri Apr 25 03:04:57 2014

this is based very loosely on example code from the PyDAQmx website

@author: Josiah McClurg
"""

import scipy.io
import time
import math
from PyDAQmx import *
from PyDAQmx.DAQmxCallBack import *
import numpy as np
from RingBuffer import *

class AIChannelSpec(object):
	def __init__(self, deviceName, portNum, name=None, termConf=DAQmx_Val_Cfg_Default, rangemin=-10, rangemax=10, units=None):
		self.port = 'ai%d'%(int(portNum))
		self.deviceName = str(deviceName)
		self.device = self.deviceName+'/'+self.port

		self.rangemin = float(rangemin)
		self.rangemax = float(rangemax)

		if name is None:
			self.name = 'v%d'%(int(portNum))
		else:
			self.name = str(name)

		self.termConf = termConf

		if not ((units is None) or (units.lower() == "volts") or (units.lower() == "volt") or (units.lower() == "v")):
			self.units = str(units)
			self.valType = DAQmx_Val_FromCustomScale
		else:
			self.units = None
			self.valType = DAQmx_Val_Volts

class MultiChannelAITask(Task):
	"""Class to create a multi-channel analog input

	The channels must be a list of AIChannelSpec objects.
	"""
	def __init__(self, channels, dataWindowLength=None, sampleRate=10000.0, bufferSize=None, sampleEvery=None, timeout=None,  data_updated_callback=None, debug=0):
		Task.__init__(self)

		self.debug = debug
		self.numChans = len(channels)
		self.timeStarted = -1
		self.timeStopped = -1
		self.data_updated_callback = data_updated_callback
		devicesReset = []

		for chan in channels:
			if chan.deviceName not in devicesReset:
				if self.debug > 0: print "Resetting device %s" % (chan.deviceName)
				DAQmxResetDevice(chan.deviceName)
				devicesReset.append(chan.deviceName)

			self.CreateAIVoltageChan(chan.device,
						chan.name,
						chan.termConf,
						chan.rangemin,
						chan.rangemax,
						chan.valType,
						chan.units)

		self.sampleRate = float(sampleRate)

		# The small memory buffer in which the samples go when they are read from the device.
		# The default is to make the buffer large enough to store a half second of data
		if bufferSize  is None:
			self.bufferSize = int(self.numChans*np.round(self.sampleRate/2.0))
		else:
			self.bufferSize = int(bufferSize)
		self.buffer = np.zeros(self.bufferSize)

		# The larger memory buffer to which the samples are added, for later viewing.
		# The default is large enough to store ten seconds of a data
		if dataWindowLength is None:
			self.dataWindow = RingBuffer(np.round(self.sampleRate*10.0),len(channels)+1);
		else:
			self.dataWindow = RingBuffer(dataWindowLength,len(channels)+1);

		# The default is chosen such that the buffer is half full when it is emptied
		if sampleEvery is None:
			self.sampleEvery = int(max(math.floor(math.floor(self.bufferSize/(self.numChans))/2.0),1))
		else:
			self.sampleEvery = int(sampleEvery)

		if timeout is None:
			self.timeout = float(self.sampleEvery)/self.sampleRate
		else:
			self.timeout = float(timeout)

		self.CfgSampClkTiming("OnboardClock",
				self.sampleRate,
				DAQmx_Val_Rising,
				DAQmx_Val_ContSamps,
				self.bufferSize)

		self.AutoRegisterEveryNSamplesEvent(DAQmx_Val_Acquired_Into_Buffer,
				self.sampleEvery,
				0)

		self.AutoRegisterDoneEvent(0)

	def EveryNCallback(self):
		read = int32()
		self.ReadAnalogF64(self.sampleEvery,
				self.timeout,
				DAQmx_Val_GroupByScanNumber,
				self.buffer,
				self.bufferSize,
				byref(read),
				None)

		endTime = time.time()
		numRead = int(read.value)
		if(numRead != self.sampleEvery):
			print '%d is not %d'%(numRead, self.sampleEvery)
			return -1

		startTime = endTime - float(numRead - 1)/self.sampleRate
		timeVector = np.linspace(startTime,endTime,numRead).reshape(numRead,1)
		readPortion = self.buffer[0:numRead*self.numChans].reshape(numRead,self.numChans)

		self.dataWindow.extend(np.concatenate((timeVector,readPortion),axis=1))
		if not (self.data_updated_callback is None):
			self.data_updated_callback(startTime,endTime,numRead)

		return 0

	def DoneCallback(self, status):
		return 0

	def StopTask(self):
		self.timeStopped = time.time()
		read = int32()
		self.ReadAnalogF64(-1,
				self.timeout,
				DAQmx_Val_GroupByScanNumber,
				self.buffer,
				self.bufferSize,
				byref(read),
				None)
		numRead = int(read.value)
		if numRead > 0:
			endTime = time.time()
			startTime = endTime - float(numRead)/self.sampleRate
			timeVector = np.linspace(startTime,endTime,numRead).reshape(numRead,1)
			readPortion = self.buffer[0:numRead*self.numChans].reshape(numRead,self.numChans)
			self.dataWindow.extend(np.concatenate((timeVector,readPortion),axis=1))

		Task.StopTask(self)

	def StartTask(self):
		self.timeStarted = time.time()
		Task.StartTask(self)

if __name__ == '__main__':
	import sys

	class flushfile():
		def __init__(self, f):
			self.f = f

		def __getattr__(self,name):
			return object.__getattribute__(self.f, name)

		def write(self, x):
			self.f.write(x)
			self.f.flush()

	# Make the output unbuffered
	sys.stdout = flushfile(sys.stdout)

	channels = []
	portNums = np.concatenate((np.arange(8), np.arange(8)+16));
	for i in portNums:
		c = AIChannelSpec('JDAQ', i, termConf=DAQmx_Val_Diff, rangemin=-10, rangemax=10)
		channels.append(c)

	m = MultiChannelAITask(channels,sampleRate=10000)

	raw_input("Press enter to start measurement")
	m.StartTask()

	raw_input("Measuring. Press enter to stop")
	#print "Measuring for 2min"
	#time.sleep(120);

	m.StopTask()
	m.ClearTask()

	dat = {"startTime": m.timeStarted, "stopTime": m.timeStopped, "data": m.dataWindow.getFIFO()}
	dat['time'] = dat['data'][:,0] - dat['data'][0,0]
	for i in range(0,len(channels)):
		dat[channels[i].name] = dat['data'][:,i+1]

	print "Saving data to data.mat"
	scipy.io.savemat('data.mat',dat,False,do_compression=True,oned_as='column')
