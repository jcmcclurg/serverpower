#!/usr/bin/python

# -*- coding: utf-8 -*-
"""
Created on Fri Jan 08 09:57:53 2016

@author: Josiah
"""

import numpy as np
import json
import gzip
import sys
import cPickle as pickle
import matplotlib.pyplot as plt

from defaultGetPower import readTotalPowerFile
from scipy.interpolate import interp1d

def generateJobEnergyFile(rawJobFile, rawPowerFile, newJobFile, newPowerFile, verbose=False):
	#powerData[:,0] -= powerStartTime

	# Load the job events first
	fp = gzip.open(rawJobFile,'rb')
	j = json.load(fp)
	fp.close()

	jobEndTime = -float("inf")
	jobStartTime = float("inf")
	for event in j["events"]:
		if ("eventinfo" in event) and ("Event" in event["eventinfo"]) and ("timestamp" in event):
			if event["eventinfo"]["Event"] == "SparkListenerJobEnd":
				jobEndTime = np.max([jobEndTime, float(event["timestamp"])/1000.0])
			elif event["eventinfo"]["Event"] == "SparkListenerJobStart":
				jobStartTime = np.min([jobStartTime, float(event["timestamp"])/1000.0])

	if jobStartTime == float("inf"):
		raise ValueError("Job %s has no start."%(rawJobFile))
	if jobEndTime == -float("inf"):
		raise ValueError("Job %s has no end."%(rawJobFile))

	# Only now do we care about the power.

	#blocklen = 2000
	#powerData = readPowerFile(f,blocklen,verbose)
	power,time = readTotalPowerFile(rawPowerFile,[0,1,2,3,4],jobStartTime,jobEndTime,newPowerFile,verbose)
	if not (np.all(np.diff(time) >= 0)):
		problemIndices, = np.where(np.diff(time) < 0)

		if len(problemIndices) > 100:
			problemIndices = problemIndices[0:100]
		problemDiffs = time[problemIndices]

		raise ValueError("Time is not monotonically increasing for %s/powerLog. Specifically, at indices %s, there are diffs %s."%(expDir, ",".join(["%d"%(s) for s in problemIndices]), ",".join(["%g"%(s) for s in problemDiffs])))

	#powerData = rawFileToPowerFile(f,f+"_%d.gz"%(blocklen),blocklen,verbose)
	#powerData = powerData[:, [0,3,4,5,6,7] ]
	#powerStartTime = powerData[0,0]
	#powerEndTime = powerData[-1,0]
	powerStartTime = time[0]
	powerEndTime = time[-1]


	if(jobStartTime < powerStartTime):
		raise ValueError("Job %s started before power collection (%.4f,%.4f)"%(rawJobFile,jobStartTime,powerStartTime))
	if(jobEndTime > powerEndTime):
		raise ValueError("Job %s ended after power collection (%.4f,%.4f)"%(rawJobFile,jobEndTime,powerEndTime))

	#print "Time:%s Power:%s"%(time.shape,power.shape)
	interpolatorFunc = interp1d(time,power,copy=False,assume_sorted=True)

	powerEndpoints = interpolatorFunc( [jobStartTime,jobEndTime] )
	if time[0] <= jobStartTime:
		pwrStartI = 1
	else:
		pwrStartI = 0
	if time[-1] >= jobEndTime:
		pwrEndI = -2
	else:
		pwrEndI = -1

	power = np.concatenate( ([powerEndpoints[0]],power[pwrStartI:pwrEndI],[powerEndpoints[1]]) )
	time = np.concatenate( ([jobStartTime],time[pwrStartI:pwrEndI], [jobEndTime]) )
	meanPower = np.trapz( power, time )/(jobEndTime-jobStartTime)

	if verbose:
		print "Writing to file:: meanPower:%g,jobStartTime:%g,jobEndTime:%g"%(meanPower,jobStartTime,jobEndTime)

	fp = open(newJobFile,"wb")
	pickle.dump(meanPower,fp,-1)
	pickle.dump(jobStartTime,fp,-1)
	pickle.dump(jobEndTime,fp,-1)
	fp.close()

	return (meanPower,jobStartTime,jobEndTime)

def readJobEnergy(rawJobFile,rawPowerFile,newJobFile=None,newPowerFile=None,verbose=False,needsReload=False):
	if newJobFile == None:
		newJobFile = rawJobFile+"_energy.gz"
	if not needsReload:
		try:
			if verbose:
				print "Loading job energy from "+newJobFile+"..."

			fp = open(newJobFile,"rb")
			meanPower = pickle.load(fp)
			jobStartTime = pickle.load(fp)
			jobEndTime = pickle.load(fp)
			fp.close()

		except IOError as err:
			if verbose:
				print "Does not exist (%s). Attempting to create..."%(err)
			needsReload = True

	if needsReload:
		meanPower,jobStartTime,jobEndTime = generateJobEnergyFile(rawJobFile,rawPowerFile, newJobFile, newPowerFile, verbose)

	return (meanPower,jobStartTime,jobEndTime)

if __name__ == "__main__":
	verbose = True


	expDir = "experiments/Aug12_monitored1/exp22/Aggregation-Arrays-Higgs"

	#data = generateJobEnergyFile(expDir,"job.energy",verbose)
	meanPower,jobStartTime,jobEndTime = readJobEnergy(expDir+"/sparkEvents.json.gz",expDir+"/powerlog.log",verbose=verbose)