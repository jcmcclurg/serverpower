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
import os
import cPickle as pickle

from defaultJobEnergy import readJobEnergy
from defaultGetPower import readTotalPowerFile
from scipy.interpolate import interp1d

def generateEnsemblePowerAverageFile(rawJobAndPowerFileTuples, newEnsembleFile, newJobAndPowerFileTuples=None, verbose=False):

	jobTimes = []
	maxDuration = -float("inf")

	if newJobAndPowerFileTuples == None:
		newJobAndPowerFileTuples = [ (None,None) for i in rawJobAndPowerFileTuples ]

	# Get the maximum duration of any job in the group
	counter = 0
	for (rawJobFile,rawPowerFile) in rawJobAndPowerFileTuples:
		newJobFile,newPowerFile = newJobAndPowerFileTuples[counter]
		meanPower,jobStartTime,jobEndTime = readJobEnergy(rawJobFile,rawPowerFile,newJobFile,newPowerFile,verbose)
		duration = jobEndTime-jobStartTime
		jobTimes.append( (jobStartTime,jobEndTime) )
		if duration > maxDuration:
			maxDuration = duration
		counter += 1

	sampleRate = 10000.0
	ensembleTime = np.arange(0,maxDuration,1.0/sampleRate)
	ensemblePower = np.zeros( ensembleTime.size )

	weights = np.zeros(ensembleTime.size)
	counter = 0
	for (rawJobFile,rawPowerFile) in rawJobAndPowerFileTuples:
		jobStartTime,jobEndTime = jobTimes[counter]
		duration = jobEndTime-jobStartTime
		newJobFile,newPowerFile = newJobAndPowerFileTuples[counter]

		power,time = readTotalPowerFile(rawPowerFile,[0,1,2,3,4],jobStartTime,jobEndTime,newPowerFile,verbose)
		powerStartTime = time[0]
		powerEndTime = time[-1]

		if(jobStartTime < powerStartTime):
			raise ValueError("Job %s started before power collection %s (%.4f,%.4f)"%(rawJobFile,rawPowerFile,jobStartTime,powerStartTime))
		if(jobEndTime > powerEndTime):
			raise ValueError("Job %s ended after power collection %s (%.4f,%.4f)"%(rawJobFile,rawPowerFile,jobEndTime,powerEndTime))

		interpolatorFunc = interp1d(time,power,bounds_error=True,copy=False,assume_sorted=True)
		ensembleRange = np.logical_and(ensembleTime >= 0,ensembleTime <= duration)
		itotalPower = interpolatorFunc(ensembleTime[ensembleRange] + jobStartTime)
		#print itotalPower
		#print "totalPower: %s, ensemblePower: %s, weights: %s"%(totalPower.shape, ensemblePower.shape, weights.shape)
		ensemblePower[ensembleRange] += itotalPower
		weights[ensembleRange] += 1
		counter += 1

	ensemblePower = ensemblePower / weights
	#ensembleTime = ensembleTime / weights

	fp = open(newEnsembleFile,"wb")
	pickle.dump(ensemblePower,fp,-1)
	pickle.dump(ensembleTime,fp,-1)
	pickle.dump(weights,fp,-1)
	fp.close()

	return (ensemblePower,ensembleTime,weights)

# https://en.wikibooks.org/wiki/Algorithm_Implementation/Strings/Longest_common_subsequence#Usage_example
def LCS(X, Y):
	m = len(X)
	n = len(Y)
	# An (m+1) times (n+1) matrix
	C = [[0] * (n + 1) for _ in range(m + 1)]
	for i in range(1, m+1):
		for j in range(1, n+1):
			if X[i-1] == Y[j-1]:
				C[i][j] = C[i-1][j-1] + 1
			else:
				C[i][j] = max(C[i][j-1], C[i-1][j])
	return C

def backTrack(C, X, Y, i, j):
	if i == 0 or j == 0:
		return ""
	elif X[i-1] == Y[j-1]:
		return backTrack(C, X, Y, i-1, j-1) + X[i-1]
	else:
		if C[i][j-1] > C[i-1][j]:
			return backTrack(C, X, Y, i, j-1)
		else:
			return backTrack(C, X, Y, i-1, j)


def readEnsemblePowerAverage(rawJobAndPowerFileTuples,newEnsembleFile=None,newJobAndPowerFileTuples=None,verbose=False,needsReload=False):
	if newEnsembleFile == None:
		# flatten the list of files
		fileList = [ f for pair in rawJobAndPowerFileTuples for f in pair ]
		prefix = os.path.commonprefix(fileList)

		longestSubsequence = fileList[0][len(prefix):]
		for f in fileList[1:]:
			testSequence = f[len(prefix):]
			C = LCS(longestSubsequence, testSequence)
			longestSubsequence = "%s"%( backTrack(C, longestSubsequence, testSequence, len(longestSubsequence), len(testSequence) ) )

		newEnsembleFile = prefix+longestSubsequence.replace("/","").replace("\\","")+"_ensemblePower.gz"

	if not needsReload:
		try:
			if verbose:
				print "Loading data from "+newEnsembleFile+"..."

			fp = open(newEnsembleFile,"rb")
			ensemblePower = pickle.load(fp)
			ensembleTime = pickle.load(fp)
			weights = pickle.load(fp)
			fp.close()

		except IOError as err:
			if verbose:
				print "Does not exist (%s). Attempting to create..."%(err)
			needsReload = True

	if needsReload:
		ensemblePower,ensembleTime,weights = generateEnsemblePowerAverageFile(rawJobAndPowerFileTuples, newEnsembleFile, newJobAndPowerFileTuples, verbose)

	return (ensemblePower,ensembleTime,weights)

if __name__ == "__main__":
	from my_plot_common import averageAroundMidpoints
	import matplotlib.pyplot as plt

	verbose = True

	powerCap = 35
	expName = "KnnArray28"

	uids = ["Jul28_powercap5",
		"Jul29_powercap",
		"Jul29_powercap2",
		"Jul29_powercap3"]
	tuples = []
	for uid in uids:
		d = "F:/josiah/bigdata_paper/experiments/"+uid+"/exp%d"%(powerCap)+"/"+expName
		powerFile = d+"/powerlog.log"
		jobFile = d+"/sparkEvents.json.gz"
		tuples.append( (jobFile,powerFile) )

	ensemblePower,ensembleTime,weights = readEnsemblePowerAverage(tuples,verbose=verbose)
	ttime = np.arange(ensembleTime[0],ensembleTime[-1],1.0)
	power = averageAroundMidpoints(ensembleTime,ensemblePower,ttime)
	plt.plot(ttime,power)