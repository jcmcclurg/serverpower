#!/usr/bin/python

# -*- coding: utf-8 -*-
"""
Created on Fri Jan 08 09:57:53 2016

@author: Josiah
"""

import cPickle as pickle
import numpy as np
import gzip
import os


def generateTurbostatDataFile(filename,newFilename,verbose=False):
	f = open(filename,"rb")
	if(verbose):
		print "Opened raw file %s."%(filename)

	fileEnded = False

	colHeaders = f.readline().strip().split()
	data = []

	while not fileEnded:
		try:
			line = f.readline()
			if line != "":
				lineStrs = line.strip().split()
				if len(lineStrs) == len(colHeaders):
					data.append([float(s) for s in lineStrs])
			else:
				fileEnded = True
		except EOFError:
			fileEnded = True

	f.close()
	data = np.array(data)

	fp = gzip.open(newFilename,'wb')
	pickle.dump(data,fp,-1)
	pickle.dump(colHeaders,fp,-1)
	fp.close()

	return (data,colHeaders)

def readTurbostatDataFile(filename,newFile=None,verbose=False,needsReload=False):
	if newFile == None:
		newFile = filename+"_tsdata.gz"

	if not needsReload:
		try:
			if verbose:
				print "Loading data from power file..."
			fp = gzip.open(newFile,"rb")
			data = pickle.load(fp)
			colHeaders = pickle.load(fp)
			fp.close()

		except IOError as err:
			if verbose:
				print "Does not exist (%s). Attempting to create..."%(err)
			needsReload = True

	if needsReload:
		data,colHeaders = generateTurbostatDataFile(filename, newFile, verbose)

	if verbose:
		print "Got %d blocks."%(data.shape[0])

	return (data,colHeaders)

if __name__ == "__main__":

	verbose = True
	uniqueIDs = ["Aug12_monitored1"]
	powerCaps = [22,25,28,32,35]
	queryTypes = ["Aggregation","Knn","TopK"]
	algorithms = ["Arrays","BSI"]
	datasets = ["Higgs","Images","SynthSmall","Synth"]

	basePath = "experiments"
	for uniqueID in uniqueIDs:
		dir1 = basePath+"/"+uniqueID
		if os.path.exists(dir1):
			for powerCap in powerCaps:
				dir2 = dir1+"/exp%d"%(powerCap)
				if os.path.exists(dir2):
					for queryType in queryTypes:
						for algorithm in algorithms:
							for dataset in datasets:
								dir3 = dir2+"/"+queryType+"-"+algorithm+"-"+dataset
								turboStatFile = dir3+"/turbostat.out"
								if os.path.exists(turboStatFile):
									print "File: "+turboStatFile
									data,colHeaders = readTurbostatDataFile(turboStatFile,verbose=verbose)
									time = data[:,0] - data[0,0]
									pkgWatt = data[:,-5]
									corWatt = data[:,-4]
									ramWatt = data[:,-3]
									plt.plot(time,pkgWatt)
									plt.plot(time,corWatt)
									plt.plot(time,ramWatt)
									raise Exception("Done!")
								else:
									print "Skipping "+dir3
				else:
					print ">>"+dir2+" does not exist..."
		else:
			print ">"+dir1+" does not exist..."