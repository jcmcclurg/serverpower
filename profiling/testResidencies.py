#!/usr/bin/python

import numpy
import re
import cPickle as pickle
import gzip
import time

def getMhzFromRaw(filename,verbose=False):
	f=open(filename,'rb')
	data = []
	foundTimestamp = False
	counter = 0

	printEvery = 1
	if(verbose):
		startTime = time.time()
		print "Opened raw file %s."%(filename)
	else:
		startTime = 0

	for line in f:
		if (not foundTimestamp) and ( re.search('^Time: [0-9]+\.[0-9]+$', line) != None ):
			timestamp = float(line.split()[1])
			foundTimestamp = True

		elif foundTimestamp:
			counter += 1
			#print "%d,%s"%(counter,line.strip())
			if counter == 2:
				v = [timestamp]
			elif counter > 2:
				v.append(float(line.split()[2]))

			if counter == 14:
				counter = 0
				foundTimestamp = False
				data.append(v)

			if verbose and (time.time() - startTime > printEvery):
				startTime = time.time()
				print "The list has %d blocks."%(len(data))

	return numpy.array(data)

def rawFileToMhzFile(oldFilename,newFilename,verbose=False):
	if verbose:
		print "Loading data from raw..."
	data = getMhzFromRaw(oldFilename,verbose)
	if verbose:
		print "Writing data (%d blocks) to mHz file..."%(data.shape[0])
	fp = gzip.open(newFilename,'wb')
	pickle.dump(data,fp,-1)
	fp.close()

	return data

def readMhzFile(filename,verbose=False):
	try:
		if verbose:
			print "Loading data from residencies file..."
		fp = gzip.open(filename+"_mhz_cache.gz","rb")
		data = pickle.load(fp)
		fp.close()

	except IOError as err:
		if verbose:
			print "Does not exist (%s). Attempting to create..."%(err)
		data = rawFileToMhzFile(filename, filename+"_mhz_cache.gz", verbose)

	if verbose:
		print "Got %d blocks."%(data.shape[0])

	return data

if __name__ == "__main__":
	exps = { 'stress': '1452722752.651508100', 'signal_insert_delays':'1452732970.201413700', 'rapl':'1452743186.881235700','powerclamp':'1452753403.717082000','cpufreq':'1452796934.955382300', 'hypervisor':'1452819943.960146400' }

	for exp in ['hypervisor']:
		date = exps[exp]
		print exp+": "+date
		d = "experiments/"+exp+"/"+date

		#for i in [1,2,3,4]:
		for i in [1]:
			print " server "+str(i)
			data = readMhzFile(d+"/server"+str(i)+"/"+date+".pgadglog",True)
		print ""

