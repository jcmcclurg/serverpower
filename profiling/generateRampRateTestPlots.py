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

littleFile = 'rampRateTest/powerlog1.log'
#bigFile = 'experiments/cgroups/1456424681.514702900/powerlog.log'
bigFile = 'experiments/signal_insert_delays/1452732970.201413700/powerlog.log'
#type = 'big'
type = 'little'
if type == 'little':
	file = littleFile
else:
	file = bigFile

powerData = readPowerFile(file,1000,verbose)
#powerData = powerData[:, [0,4,5,6,7] ]
powerData = powerData[:, [0,4,5,6,7] ]
ignoreTime = 10

if showFig:

	if type == 'little':
		time = np.linspace(0,powerData[-1,0]-powerData[0,0],powerData.shape[0])
		data = np.zeros(time.shape[0])
		data[:] = np.sum(powerData[:,1:],axis=1)
	else:
		start = 30000+2950
		len = 150
		#start = 0
		#len=10000
		# The data is ten samples separated.
		data = np.zeros(len)
		for i in range(4):
			data += powerData[start+10*i:start+10*i+len,i+1]
		time = np.linspace(0,powerData[start+len-1,0]-powerData[start,0],len)

	plt.plot(time,data)
	plt.title('Example of fast ramp rate for four-server cluster')
	plt.xlabel('Time (seconds)')
	plt.ylabel('Cluster power (W)')
	plt.show()

