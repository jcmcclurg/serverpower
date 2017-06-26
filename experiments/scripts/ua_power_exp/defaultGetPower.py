#!/usr/bin/python

# -*- coding: utf-8 -*-
"""
Created on Fri Jan 08 09:57:53 2016

@author: Josiah
"""

import cPickle as pickle
import numpy as np
import time
import gzip

"""
Returns the power in the format:
	Start time, End time, Cluster power, Server0 power, Server1 power, Server2 power, Server3 power, Server4 power
"""
def getPowerFromRaw(filename,blockLen,verbose=False):
	f = gzip.open(filename,"rb")
	if(verbose):
		startTime = time.time()
		print "Opened raw file %s."%(filename)
	else:
		startTime = 0

	fileEnded = False

	voltageIndex = 1
	rackIndex = 2
	serverIndices = np.array([0, 4, 3, 2, 1]) + 3;
	numLeftover = 0
	rlist = []
	printEvery = 1
	totalBlocks = 0
	while not fileEnded:
		try:
			if numLeftover > 0:
				c = pickle.load(f)
				b = np.concatenate((b[-numLeftover::,:], c), axis=0)
			else:
				b = pickle.load(f)

			numSamples = b.shape[0]
			numLeftover = np.mod(numSamples,blockLen)
			numBlocks = int(np.floor(float(numSamples)/float(blockLen)))
			totalBlocks += numBlocks

			r = np.zeros((numBlocks,8))
			offset = 0
			for i in range(0,numBlocks):
				blockRange = np.arange(0,blockLen) + offset
				r[i,0] = np.min(b[blockRange,0])
				r[i,1] = np.max(b[blockRange,0])
				r[i,rackIndex] = np.mean(b[blockRange,voltageIndex]*b[blockRange,rackIndex],axis=0)
				for j in serverIndices:
					r[i,j] = np.mean(b[blockRange,voltageIndex]*b[blockRange,j],axis=0)
				offset += blockLen

			if verbose and (time.time() - startTime > printEvery):
				startTime = time.time()
				print "The list has %d blocks of length %d."%(totalBlocks,blockLen)

			rlist.append(r)
		except IOError as err:
			print "Whoops: %s"%(err)
			fileEnded = True
		except EOFError:
			fileEnded = True

	f.close()
	return np.concatenate(rlist)

def rawFileToPowerFile(oldFilename,newFilename,blockLen,verbose=False):
	if verbose:
		print "Loading data from raw..."
	data = getPowerFromRaw(oldFilename,blockLen,verbose)
	if verbose:
		print "Writing data (%d blocks of length %d) to power file..."%(data.shape[0],blockLen)
	fp = gzip.open(newFilename,'wb')
	pickle.dump(data,fp,-1)
	fp.close()

	return data

def readPowerFile(filename,blockLen,verbose=False):
	try:
		if verbose:
			print "Loading data from power file..."
		fp = gzip.open(filename+"_%d.gz"%(blockLen),"rb")
		data = pickle.load(fp)
		fp.close()

	except IOError as err:
		if verbose:
			print "Does not exist (%s). Attempting to create..."%(err)
		data = rawFileToPowerFile(filename, filename+"_%d.gz"%(blockLen), blockLen, verbose)

	if verbose:
		print "Got %d blocks."%(data.shape[0])

	return data

if __name__ == "__main__":
	exps = { 'stress': '1452722752.651508100', 'signal_insert_delays':'1452732970.201413700', 'rapl':'1452743186.881235700','powerclamp':'1452753403.717082000','cpufreq':'1452796934.955382300' }
	for exp in exps:
		date = exps[exp]
		print exp+": "+date
		d = "experiments/"+exp+"/"+date
		data = readPowerFile(d+"/powerlog.log",1000,True)
		print ""
