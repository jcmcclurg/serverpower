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
from numpy import zeros

class ChannelSpec(object):
	def __init__(self, device, rangemin=-10, rangemax=10, units=None, filtCutoff=None, termConf=None, name=None):
		self.device = str(device)
		self.rangemin = float(rangemin)
		self.rangemax = float(rangemax)

		if name is None:
			self.name = None
		else:
			self.name = str(name)

		if filtCutoff is None:
			self.filtCutoff = None
		else:
			self.filtCutoff = float(filtCutoff)

		if termConf.lower() == "diff" or termConf.lower() == "differential":
			self.termConf = "diff"
		elif termConf.lower() == "rse" or termConf.lower() == "referenced":
			self.termConf = "rse"
		elif termConf.lower() == "nrse" or termConf.lower() == "nonreferenced" or termConf.lower() == "non-referenced":
			self.termConf = "nrse"
		elif termConf.lower() == "pseudo" or termConf.lower() == "pseudodifferential":
			self.termConf = "pseudo"
		else:
			self.termConf = None

		if not ((units is None) or (units.lower() == "volts") or (units.lower() == "volt") or (units.lower() == "v")):
			self.units = str(units)
		else:
			self.units = None

class MultiChannelAnalogInput(Task):
	"""Class to create a multi-channel analog input

	Usage: AI = MultiChannelInput(physicalChannel)
		physicalChannel: a string or a list of strings
	optional parameter: limit: tuple or list of tuples, the AI limit values
						reset: Boolean
	Methods:
		read(name), return the value of the input name
		readAll(), return a dictionary name:value
	"""
	def __init__(self, channelSpecs, sampleRate=10000.0, bufferSize=20000, sampleEvery=None, timeout=None, debug=0):
		Task.__init__(self)

		self.timeout = 0
		self.debug = debug
		self.lastPrintTime = 0

		if type(channelSpecs) != type([]):
			self.channelSpecs = [channelSpecs]
		else:
			self.channelSpecs = channelSpecs

		# The number of channels from which to sample
		self.numChans = len(self.channelSpecs)

		for i in range(0,self.numChans):
			if type(self.channelSpecs[i]) == type(""):
				self.channelSpecs[i] = ChannelSpec(self.channelSpecs[i])
			if self.channelSpecs[i].name is None:
				self.channelSpecs[i].name = str("voltage_%d" % i)

		nameCounts = {}
		for i in range(0,self.numChans):
			if(nameCounts.has_key(self.channelSpecs[i].name)):
				nameCounts[self.channelSpecs[i].name] += 1
				self.channelSpecs[i].name += ("_%d"%(nameCounts[self.channelSpecs[i].name] - 1))
			else:
				nameCounts[self.channelSpecs[i].name] = 0

		# Make sure all resources are released
		ports = {}
		for chan in self.channelSpecs:
			ports[chan.device.split('/')[0]] = 1

		for dev in ports.keys():
			if self.debug > 0: print "Resetting device %s" % (dev)
			DAQmxResetDevice(dev)

		# The buffer in which the samples go, and the ID of that buffer
		self.bufferSize = int(bufferSize)
		self.data = zeros(self.bufferSize)
		self.samples = {}
		self.raw_samples = []
		self.sampleRate = float(sampleRate)
		if sampleEvery is None:
			self.sampleEvery = int(max(math.floor(math.floor(self.bufferSize/(self.numChans))/2.0),1))
		else:
			self.sampleEvery = int(sampleEvery)

		if timeout is None:
			self.timeout = float(self.sampleEvery)/sampleRate
		else:
			self.timeout = float(timeout)

		for chan in self.channelSpecs:
			self.samples[chan.name] = {"samples":[],
					"device":chan.device,
					"rangemin":chan.rangemin,
					"rangemax":chan.rangemax}

			if chan.termConf == "diff":
				termConf = DAQmx_Val_Diff
			elif chan.termConf == "rse":
				termConf = DAQmx_Val_RSE
			elif chan.termConf == "nrse":
				termConf = DAQmx_Val_NRSE
			elif chan.termConf == "pseudo":
				termConf = DAQmx_Val_PseudoDiff
			else:
				termConf = DAQmx_Val_Cfg_Default

			if chan.units is None:
				self.CreateAIVoltageChan(chan.device,
						chan.name,
						termConf,
						chan.rangemin,
						chan.rangemax,
						DAQmx_Val_Volts,
						None)
				units = "Volts"
				self.samples[chan.name]["units"] = "Volts"
			else:
				self.CreateAIVoltageChan(chan.device,
						chan.name,
						termConf,
						chan.rangemin,
						chan.rangemax,
						DAQmx_Val_FromCustomScale,
						chan.units)
				units = chan.units
				self.samples[chan.name]["units"] = chan.units

			if chan.filtCutoff is None:
				DAQmxSetAILowpassEnable(self.taskHandle, chan.name, False);
				filt = "without filter"
				self.samples[chan.name]["filtcutoff"] = float("inf")
			else:
				DAQmxSetAILowpassEnable(self.taskHandle, chan.name, True);
				DAQmxSetAILowpassCutoffFreq(self.taskHandle, chan.name, chan.filtCutoff)
				if (chan.filtCutoff < 1e3) or (chan.filtCutoff > 1e12):
					filt = "%g Hz" % (chan.filtCutoff)
				elif chan.filtCutoff < 1e6:
					filt = "%g kHz" % (chan.filtCutoff/1e3)
				elif chan.filtCutoff < 1e9:
					filt = "%g MHz" % (chan.filtCutoff/1e6)
				elif chan.filtCutoff < 1e12:
					filt = "%g GHz" % (chan.filtCutoff/1e9)
				filt = "with %s filter" % (filt)
				self.samples[chan.name]["filtcutoff"] = chan.filtCutoff

			self.samples[chan.name]["termconf"] = str(chan.termConf)

			if self.debug > 0: print "Added channel %s (%s, %g to %g, %s %s %s-mode)" % (chan.name, chan.device, chan.rangemin, chan.rangemax, units, filt, chan.termConf)

		self.CfgSampClkTiming("OnboardClock",
				self.sampleRate,
				DAQmx_Val_Rising,
				DAQmx_Val_ContSamps,
				self.bufferSize)

		self.AutoRegisterEveryNSamplesEvent(DAQmx_Val_Acquired_Into_Buffer,
				self.sampleEvery,
				0)
		if debug > 10:
			print "SAMPLES: %s" % (self.samples)

		self.AutoRegisterDoneEvent(0)

	def updateSamples(self,dat):
		self.raw_samples.extend(dat)
		i = 0
		p = (self.debug > 0) and (self.lastPrintTime < (time.time() - 1))

		if p:
			s = ""

		for chan in self.channelSpecs:
			d = dat[i:len(dat):self.numChans];
			self.samples[chan.name]["samples"].extend(d)
			if p:
				s += ("%s:%g " % (chan.name, sum(d)/float(len(d))))
			i += 1
		if p:
			print s
			self.lastPrintTime = time.time()

	def EveryNCallback(self):
		read = int32()
		self.ReadAnalogF64(-1,
				self.timeout,
				#DAQmx_Val_GroupByChannel,
				DAQmx_Val_GroupByScanNumber,
				self.data,
				self.bufferSize,
				byref(read),
				None)

		dat = self.data.tolist()
		self.updateSamples(dat[0:read.value*self.numChans])

		return 0

	def DoneCallback(self, status):
		print "Status %s" % (status.value)
		return 0

	def getDataLen(self):
		lens = []
		for samp in self.samples.keys():
			if self.debug > 10:
				print "getting length of samples: %s" % (samp)
			lens.append(len(self.samples[samp]["samples"]))

		return min(lens)

	def getData(self):
		return self.samples

	def StopTask(self):
		read = int32()
		self.ReadAnalogF64(-1,
				self.timeout,
				#DAQmx_Val_GroupByChannel,
				DAQmx_Val_GroupByScanNumber,
				self.data,
				self.bufferSize,
				byref(read),
				None)

		dat = self.data.tolist()
		self.updateSamples(dat[0:read.value*self.numChans])
		Task.StopTask(self)

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
