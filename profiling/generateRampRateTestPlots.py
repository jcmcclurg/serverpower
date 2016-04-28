verbose = True
saveFig = True
showFig = True

if verbose:
	print "Loading general modules..."
import numpy as np
import sys

if verbose:
	print "Loading matplotlib module..."
import matplotlib.pyplot as plt

if verbose:
	print "Loading custom functions..."
from defaultGetPower import readPowerFile

if verbose:
	print "  Reading power file..."
powerData = readPowerFile('rampRateTest/powerlog.log',1000,verbose)
powerData = powerData[:, [4,5,6,7] ]
ignoreTime = 10

if showFig:
	plt.plot(powerData)
	plt.show()

