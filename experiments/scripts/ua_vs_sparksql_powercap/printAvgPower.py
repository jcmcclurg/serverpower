#!/usr/bin/python

# -*- coding: utf-8 -*-
"""
Created on Fri Jan 08 09:57:53 2016

@author: Josiah
"""

import defaultGetPower
import sys
import numpy as np

if __name__ == "__main__":
	stage = int(sys.argv[1])
	filename = sys.argv[2]
	newFilename = sys.argv[3]
	blockLen = int(sys.argv[4])
	data = defaultGetPower.rawFileToPowerFile(filename,newFilename,blockLen)
	serverPower = np.sum(data[:,4:],1)
	
	minServerPower = np.min(serverPower)
	maxServerPower = np.max(serverPower)
	meanServerPower = np.mean(serverPower)
	medServerPower = np.median(serverPower)
	stddevServerPower = np.std(serverPower)
	precisionScore = 1.0 - np.mean(np.abs((serverPower - meanServerPower) / meanServerPower))

	print "stage %d\nminPower %g\nmaxPower %g\nmeanPower %g\nmedPower %g\nstdPower %g\nprecisionScore %g"%(stage,minServerPower,maxServerPower,meanServerPower,medServerPower,stddevServerPower,precisionScore)

