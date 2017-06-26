#!/usr/bin/python

# -*- coding: utf-8 -*-
"""
Created on Fri Jan 08 09:57:53 2016

@author: Josiah
"""

import defaultGetPower
import sys
import numpy as np

def blockAvg(x,by):
	y = x.reshape(x.shape[0],-1)
	leftover = y.shape[0] % by
	if leftover > 0:
		y = y[:-leftover,:]
		
	return np.mean(y.reshape(-1,by,y.shape[1]),axis=1)


if __name__ == "__main__":
	stage = int(sys.argv[1])
	filename = sys.argv[2]
	blockLen = int(sys.argv[3])

	if len(sys.argv) > 4:
		secs = float(sys.argv[4])
	else:
		secs = 1.0
	
	if ((len(sys.argv) > 5) and (sys.argv[5] != "")):
		startTime = float(sys.argv[5])
	else:
		startTime = -float("inf")
	if ((len(sys.argv) > 6) and (sys.argv[6] != "")):
		endTime = float(sys.argv[6])
	else:
		endTime = float("inf")

	try:
		data = defaultGetPower.rawFileToPowerFile(filename,filename+"_%d.gz"%(blockLen), blockLen)
	except IOError as err:
		data = defaultGetPower.readPowerFile(filename,blockLen)
	dataRng = (data[:,1] >= startTime) & (data[:,1] <= endTime)
	serverPower = np.sum(data[dataRng,4:],1)

	# We want it to be in 1s blocks.
	secsPerSample = float(blockLen)/10000.0;
	serverPower = blockAvg(serverPower,int(round(secs/secsPerSample)))
	
	minServerPower = np.min(serverPower)
	maxServerPower = np.max(serverPower)
	meanServerPower = np.mean(serverPower)
	medServerPower = np.median(serverPower)
	stddevServerPower = np.std(serverPower)
	precisionScore = 1.0 - np.mean(np.abs((serverPower - meanServerPower) / meanServerPower))

	print "stage %d\nminPower %g\nmaxPower %g\nmeanPower %g\nmedPower %g\nstdPower %g\nprecisionScore %g"%(stage,minServerPower,maxServerPower,meanServerPower,medServerPower,stddevServerPower,precisionScore)

