#!/usr/bin/python

import numpy
import re
import cPickle as pickle
import gzip
import time

def getSetpointsFromRaw(filename,verbose=False):
	printEvery = 1
	if(verbose):
		startTime = time.time()
		print "Opened raw file %s."%(filename)
	else:
		startTime = 0

	f=open(filename,'rb')
	data = []
	for line in f:
		if re.search('^[0-9]+(\.[0-9]*)?,[0-9]+(\.[0-9]*)?$', line) != None:
			v = [ float(i) for i in line.strip().split(',') ]
			data.append(v)

			if verbose and (time.time() - startTime > printEvery):
					startTime = time.time()
					print "The list has %d blocks."%(len(data))

	return numpy.array(data)

def rawFileToSetpointsFile(oldFilename,newFilename,verbose=False):
	if verbose:
		print "Loading data from raw..."
	data = getSetpointsFromRaw(oldFilename,verbose)
	if verbose:
		print "Writing data (%d blocks) to setpoints file..."%(data.shape[0])
	fp = gzip.open(newFilename,'wb')
	pickle.dump(data,fp,-1)
	fp.close()

	return data

def readSetpointsFile(filename,verbose=False):
	try:
		if verbose:
			print "Loading data from setpoints file..."
		fp = gzip.open(filename+"_cache.gz","rb")
		data = pickle.load(fp)
		fp.close()

	except IOError as err:
		if verbose:
			print "Does not exist (%s). Attempting to create..."%(err)
		data = rawFileToSetpointsFile(filename, filename+"_cache.gz", verbose)

	if verbose:
		print "Got %d blocks."%(data.shape[0])

	return data

if __name__ == "__main__":
	exps = { 'stress': '1452722752.651508100', 'signal_insert_delays':'1452732970.201413700', 'rapl':'1452743186.881235700','powerclamp':'1452753403.717082000','cpufreq':'1452796934.955382300' }
	for exp in exps:
		date = exps[exp]
		print exp+": "+date
		d = "experiments/"+exp+"/"+date

		for i in [1,2,3,4]:
			print " server "+str(i)
			data = readSetpointsFile(d+"/server"+str(i)+"/"+date+".testlog",True)
		print ""
