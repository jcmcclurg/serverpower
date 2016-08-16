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
import matplotlib.pyplot as plt


def generateTotalPowerFile(rawFile,newFile,serverNums,startTime=-float("inf"),endTime=float("inf"),verbose=False):
	f = gzip.open(rawFile,"rb")
	if(verbose):
		printStartTime = time.time()
		print "Opened raw file %s."%(rawFile)
	else:
		printStartTime = 0

	fileEnded = False

	timeIndex = 0
	voltageIndex = 1
	serverIndices = np.array([0, 4, 3, 2, 1]) + 3;

	timeList = []
	rlist = []
	printEvery = 1
	totalBlocks = 0

	# Gather the recorded timing information.
	blockLengths = []
	while not fileEnded:
		try:
			r = pickle.load(f)
			if (len(r.shape) != 2) or (r.shape[1] != 8):
				print "Skipping a block with dimensions %s"%(r.shape)
				blockLengths.append(-1)
			else:
				timeList.append(r[:,timeIndex])
				totalBlocks += r.shape[0]
				if verbose and (time.time() - printStartTime > printEvery):
					printStartTime = time.time()
					print "The list has %d blocks."%(totalBlocks)
				blockLengths.append(r.shape[0])

		except IOError as err:
			print "Whoopsie: %s"%(err)
			fileEnded = True
		except EOFError:
			fileEnded = True
	f.close()
	if totalBlocks == 0:
		raise ValueError("Empty file!")

	ttime = np.concatenate(timeList)
	del timeList

	# Recalculate the time so that it is monotonic with the exact sample rate,
	# but also is a good fit for all the recorded timestamps. We calculate the
	# residuals from a line with the correct slope, and then take the median
	# of those to get the y intercept.
	sampleRate = 10000.0
	x = np.linspace(0,(totalBlocks - 1)/sampleRate, totalBlocks)

	y = ttime -ttime[0] - x
	b = np.median(y)
	if verbose:
		print "Offsets: %g, %g"%(np.min(y),np.max(y))
		#plt.plot(x,ttime - (x + b + ttime[0]),'-')
		#raise Exception("Residuals: %s"%(y-b))

	ttime = x + b + ttime[0]

	# Figure out the exact starting and ending time, and where to look for it
	tRange, = np.where(ttime <= startTime)
	if len(tRange) == 0:
		startIndex = 0
	else:
		startIndex = tRange[-1]

	tRange, = np.where(ttime >= endTime)
	if len(tRange) == 0:
		stopIndex = totalBlocks-1
	else:
		stopIndex = tRange[0]

	# Read in the data
	f = gzip.open(rawFile,"rb")
	totalBlocks = 0
	startOffset = 0
	#stopOffset = 0
	for bl in blockLengths:
		if (bl > 0):
			# The start index should be less than the end of the block we are about to read
			# The end index should be greater than the start of the block we are about to read
			if (startIndex <= totalBlocks + bl-1) and (stopIndex >= totalBlocks):
				r = pickle.load(f)

				rlist.append( np.sum(r[:,serverIndices[serverNums]],axis=1)*r[:,voltageIndex] )

				if startOffset == 0:
					startOffset = totalBlocks

				if verbose and (time.time() - printStartTime > printEvery):
					printStartTime = time.time()
					print "The list has %d blocks in range."%(totalBlocks)

			else:
				if verbose:
					print "Ignoring block %0.4f-%0.4f because it is outside of the range"%(ttime[totalBlocks],ttime[totalBlocks + bl-1])
					print "               %0.4f-%0.4f"%(startTime,endTime)
				pickle.load(f)

			totalBlocks += bl
		elif verbose:
			print "Ignoring a block because of its size."
			pickle.load(f)

	f.close()

	power = np.concatenate(rlist)
	power = power[startIndex-startOffset:stopIndex-startOffset+1]
	ttime = ttime[startIndex:stopIndex+1]

	validRange = np.logical_and(power > -float("inf"), power < float("inf") )
	ttime = ttime[validRange]
	power = power[validRange]

	fp = gzip.open(newFile,'wb')
	pickle.dump(power,fp,-1)
	pickle.dump(ttime,fp,-1)
	fp.close()

	return ( power, ttime )


def readTotalPowerFile(rawFile,serverNums,startTime=-float("inf"),endTime=float("inf"),newFile=None,verbose=False,needsReload=False):
	if newFile == None:
		newFile = rawFile+"_%s_%0.4f-%0.4f.gz"%("".join(["%d"%(s) for s in serverNums]),startTime,endTime)

	if not needsReload:
		try:
			if verbose:
				print "Loading data from power file..."
			fp = gzip.open(newFile,"rb")
			power = pickle.load(fp)
			ttime = pickle.load(fp)
			fp.close()

		except IOError as err:
			needsReload = True
			if verbose:
				print "Does not exist (%s). Attempting to create..."%(err)

	if needsReload:
		power,ttime = generateTotalPowerFile(rawFile, newFile, serverNums,startTime,endTime,verbose)

	if verbose:
		print "Got %d blocks."%(power.shape[0])

	return (power,ttime)


def getPowerFromRaw(filename,blockLen,verbose=False):
	f = gzip.open(filename,"rb")
	if(verbose):
		printStartTime = time.time()
		print "Opened raw file %s."%(filename)
	else:
		printStartTime = 0

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

			if verbose and (time.time() - printStartTime > printEvery):
				printStartTime = time.time()
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
	expDir = "experiments/Aug12_monitored1/exp22/Aggregation-Arrays-Higgs"
	filename = "powerlog.log"

	(power,ttime) = generateTotalPowerFile("test.log","test_totalPower.gz",[0,1,2,3,4],verbose=True)