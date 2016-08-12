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

from defaultGetPower import readPowerFile, rawFileToPowerFile

def generateJobEnergyFile(expDir, filename, verbose=False):
	f = expDir+"/powerlog.log"

	blocklen = 2000
	powerData = readPowerFile(f,blocklen,verbose)
	#powerData = rawFileToPowerFile(f,f+"_%d.gz"%(blocklen),blocklen,verbose)
	powerData = powerData[:, [0,3,4,5,6,7] ]
	powerStartTime = powerData[0,0]
	powerEndTime = powerData[-1,0]
	#powerData[:,0] -= powerStartTime

	fp = gzip.open(expDir+"/sparkEvents.json.gz",'rb')
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

	assert(jobEndTime != -float("inf"))
	assert(jobStartTime != float("inf"))


	if(jobStartTime <= powerStartTime):
		raise ValueError("Job %s/%s started before power collection (%.0f,%.0f)"%(expDir,filename,jobStartTime,powerStartTime))
	if(jobEndTime >= powerEndTime):
		raise ValueError("Job %s/%s ended after power collection (%.0f,%.0f)"%(expDir,filename,jobEndTime,powerEndTime))

	powerRange = np.logical_and(powerData[:,0] >= jobStartTime,powerData[:,0] <= jobEndTime)

	data = {}
	data["jobStartTime"] = jobStartTime
	data["jobEndTime"] = jobEndTime
	data["jobDuration"] = jobEndTime-jobStartTime
	data["meanPower"] = np.mean(np.sum(powerData[powerRange,1:], axis=1))

	fp = open(expDir+"/"+filename,"wb")
	pickle.dump(data,fp,-1)
	fp.close()

	return data

def readJobEnergy(expDir,filename,verbose=False):
	try:
		if verbose:
			print "Loading data from "+expDir+"/"+filename+"..."

		fp = open(expDir+"/"+filename,"rb")
		data = pickle.load(fp)
		fp.close()

	except IOError as err:
		if verbose:
			print "Does not exist (%s). Attempting to create..."%(err)
		data = generateJobEnergyFile(expDir,filename, verbose)

	return data

