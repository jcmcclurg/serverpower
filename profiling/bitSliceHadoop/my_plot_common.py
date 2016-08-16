#!/usr/bin/python

# -*- coding: utf-8 -*-
"""
Created on Fri Jan 08 09:57:53 2016

@author: Josiah
"""


import matplotlib.cm as cmx
import matplotlib.colors as colors
import numpy as np
from scipy.interpolate import interp1d

from defaultEnsemblePowerAverage import readEnsemblePowerAverage
from defaultJobEnergy import readJobEnergy

def get_cmap(N):
    '''Returns a function that maps each index in 0, 1, ... N-1 to a distinct
    RGB color.'''
    color_norm  = colors.Normalize(vmin=0, vmax=N-1)
    scalar_map = cmx.ScalarMappable(norm=color_norm, cmap='hsv')
    def map_index_to_rgb_color(index):
        return scalar_map.to_rgba(index)
    return map_index_to_rgb_color

def get_mean_with_err(data,axis=0,ragged=False):
	if ragged and (axis == 1):
		mean = []
		mx = []
		mn = []
		for row in data:
			mean.append(np.mean(row))
			mx.append(np.max(row))
			mn.append(np.min(row))

		mean = np.array(mean)
		mx = np.array(mx)
		mn = np.array(mn)
	else:
		mean = np.mean(data,axis=axis)
		mx = np.max(data,axis=axis)
		mn = np.min(data,axis=axis)

	yerr = [mx - mean, mean - mn]

	return (mean, yerr)

# Experiment unique IDs, power caps, algorithms, dimensions
def get_jobEnergyData_v1(expIDs,expRange,knnClasses,higgsNums,verbose=False):
	expData = {}

	for expID in expIDs:
		expData[expID] = {}
		for expNum in expRange:
			exp = expID+"/exp%d"%(expNum)

			#knnClass = knnClasses[0]
			#higgsNum = higgsNums[0]

			expDir = "experiments/"+exp
			#if verbose:
			#	print "Loading %s..."%(expDir)

			expData[expID][expNum] = {}
			for higgsNum in higgsNums:

				expData[expID][expNum][higgsNum] = {}
				for knnClass in knnClasses:
					i = "%s%d"%(knnClass,higgsNum)
					try:
						expData[expID][expNum][higgsNum][knnClass] = readJobEnergy(expDir+"/"+i,"job.energy",verbose)
					except IOError as error:
						if verbose:
							print "WARNING: (%s)"%(error)
	return expData

# Data organized as data[expName][powerCap][uniqueID], for easy collating
def get_jobEnergyData(uniqueIDs,powerCaps,expNames,verbose=False):
	data = {}

	for uid in uniqueIDs:
		for powerCap in powerCaps:
			for expName in expNames:
				expDir = "experiments/%s/exp%d/%s"%(uid,powerCap,expName)

				try:
					if not (expName in data):
						data[expName] = {}
					if not (powerCap in data[expName]):
						data[expName][powerCap] = {}

					data[expName][powerCap][uid] = readJobEnergy(expDir,"job.energy",verbose)
				except IOError as error:
					if verbose:
						print "WARNING: (%s)"%(error)
					else:
						pass
	return data

def get_ensemblePowerData(uids,powerCaps,expNames,verbose=False):
	data = {}
	padding = 0
	#for uid in uniqueIDs:
	for powerCap in powerCaps:
		for expName in expNames:
			if not (expName in data):
				data[expName] = {}
			#if not (powerCap in data[expName]):
			#	data[expName][powerCap] = {}
			try:
				ensemblePower,ensembleTime,weights,validUIDs = readEnsemblePowerAverage("experiments",expName+"_c%d_p%d"%(powerCap,padding)+".avg_power",expName,powerCap,uids,padding,verbose)
				data[expName][powerCap] = {"power":ensemblePower,"time":ensembleTime}
			except IOError as error:
				if verbose:
					print "WARNING: (%s)"%(error)
				else:
					pass

	return data

def averageAroundMidpoints(x,y,newX):
	if x.size != y.size:
		raise ValueError("X and Y have to be the same size")
	if newX.size > x.size:
		raise ValueError("newX has to have fewer elements than X")
	newY = []

	endpointA = x[0]
	endpointB = newX[0] + (newX[1]-newX[0])/2.0

	interpolatorFunc = interp1d(x,y)

	s = x < endpointB
	yends = interpolatorFunc([endpointA,endpointB])
	newY.append(np.trapz( np.concatenate( ([yends[0]],y[s],[yends[1]]) ) , np.concatenate( ([endpointA],x[s],[endpointB]) ) )/(endpointB-endpointA))
	#print "Endpoints: %s"%( [endpointA,endpointB] )
	for i in range(1,newX.size-1):
		endpointA = newX[i] - (newX[i]-newX[i-1])/2.0
		endpointB = newX[i] + (newX[i+1]-newX[i])/2.0
		s = np.logical_and(x >= endpointA,x < endpointB )
		yends = interpolatorFunc([endpointA,endpointB])
		newY.append(np.trapz( np.concatenate( ([yends[0]],y[s],[yends[1]]) ) , np.concatenate( ([endpointA],x[s],[endpointB]) ) )/(endpointB-endpointA))
		#print "Endpoints: %s"%( [endpointA,endpointB] )

	endpointA = newX[-1] - (newX[-1] - newX[-2])/2.0
	endpointB = x[-1]
	#print "Endpoints: %s"%( [endpointA,endpointB] )
	s = x >= endpointA
	yends = interpolatorFunc([endpointA,endpointB])
	newY.append(np.trapz( np.concatenate( ([yends[0]],y[s],[yends[1]]) ) , np.concatenate( ([endpointA],x[s],[endpointB]) ) )/(endpointB-endpointA))

	return np.array(newY)

if __name__ == "__main__":
	x 	= np.array([1,3.49999,3.5, 4,5,6,7, 8,9,10])
	y 	= np.array([8,8,3, 0,1,2,3, 6,7,11])
	newX = np.array([  2,     5,         10])
	newY = averageAroundMidpoints(x,y,newX)
	print newY