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

from defaultGetSetpoints import readSetpointsFile
from defaultGetPower import readPowerFile
from defaultGetResidencies import readResidenciesFile

def reject_outliers(data, m = 2.):
	d = np.abs(data - np.median(data))
	mdev = np.median(d)
	if mdev:
		return data[(d/mdev) < m]
	else:
		return data[:]


def getPlotDataFromExperiment(experiment,date,verbose=False):
	expDir = 'experiments/'+experiment
	currentExpDir = expDir+'/'+date

	serverNums = np.array([1,2,3,4])

	if verbose:
		print "  Reading power file..."
	powerData = readPowerFile(currentExpDir+'/powerlog.log',5000,verbose)
	powerData = powerData[:, [0,4,5,6,7] ]
	ignoreTime = 10

	plotData = []
	for i in serverNums:
		if verbose:
			print "  Reading files for server %d..."%(i)
		setpoints = readSetpointsFile(currentExpDir+('/server%d/%s.testlog'%(i, date)))
		residencies = readResidenciesFile(currentExpDir+('/server%d/%s.pgadglog'%(i, date)))

		residencyChunks = []
		residencyChunksStddev = []

		powerChunks = []
		powerChunksStddev = []
		for j in range(setpoints.shape[0]-1):
			pwr = powerData[np.logical_and(powerData[:,0] >= setpoints[j,0]+ignoreTime, powerData[:,0] < setpoints[j+1,0]),i]
			pwr = reject_outliers(pwr)
			powerChunks.append(np.mean(pwr))
			powerChunksStddev.append(np.std(pwr))

			rs = residencies[np.logical_and(residencies[:,0] >= setpoints[j,0]+ignoreTime, residencies[:,0] < setpoints[j+1,0]),1:]
			resNoOutliers = []
			resNoOutliersStddev = []
			for k in range(rs.shape[1]):
				rsno = reject_outliers(rs[:,k])
				resNoOutliers.append(np.mean(rsno))
				resNoOutliersStddev.append(np.std(rsno))

			residencyChunks.append(resNoOutliers)
			residencyChunksStddev.append(resNoOutliersStddev)

		# Don't allow indices which would produce duplicate setpoints
		sortOrder = []
		prevS = str("inf")
		for j in list(setpoints[:-1,1].argsort()):
			if not (setpoints[int(j),1] == prevS):
				sortOrder.append(int(j))
				prevS = setpoints[int(j),1]

		plotData.append({"setpoints": np.array([setpoints[j,1] for j in sortOrder]), "powerMeans": np.array([powerChunks[j] for j in sortOrder]), "powerStddevs": np.array([powerChunksStddev[j] for j in sortOrder]), "residencyMeans": np.array([residencyChunks[j] for j in sortOrder]), "residencyStddevs": np.array([residencyChunksStddev[j] for j in sortOrder]) })

	return plotData

def experimentToPlotDataFile(experiment,date,newFilename,verbose=False):
	if verbose:
		print "Loading data from experiment..."
	data = getPlotDataFromExperiment(experiment,date,verbose)
	if verbose:
		print "Writing data to file..."
	fp = gzip.open(newFilename,'wb')
	pickle.dump(data,fp,-1)
	fp.close()

	return data

def readPlotData(experiment,date,filename,verbose=False):
	try:
		if verbose:
			print "Loading data from power file..."
		fp = gzip.open(filename,"rb")
		data = pickle.load(fp)
		fp.close()

	except IOError as err:
		if verbose:
			print "%s does not exist (%s). Attempting to create..."%(filename,err)
		data = experimentToPlotDataFile(experiment, date, filename, verbose)

	if verbose:
		print "Got data."

	return data

if __name__ == "__main__":
	dates = {'stress': '1452722752.651508100', 'signal_insert_delays':'1452732970.201413700', 'rapl':'1452743186.881235700','powerclamp':'1452753403.717082000','cpufreq':'1452796934.955382300', 'hypervisor': '1452819943.960146400', 'cgroups': '1456424681.514702900' }

	for experiment in ['stress','signal_insert_delays','powerclamp','hypervisor','rapl','cpufreq','cgroups']:
		date = dates[experiment]

		print experiment+": "+date
		d = "experiments/"+experiment+"/"+date
		data = experimentToPlotDataFile(experiment,date,d+"/plotData_%s_%s.gz"%(experiment,date),True)
		print ""
