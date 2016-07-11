verbose = True
saveFig = False
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
from defaultGetSetpoints import readSetpointsFile
from defaultGetPower import readPowerFile

# 100 seconds per level, interleaved ramp
dates = {'stress': '1452722752.651508100', 'signal_insert_delays':'1452732970.201413700', 'rapl':'1452743186.881235700','powerclamp':'1452753403.717082000','cpufreq':'1452796934.955382300', 'hypervisor': '1452819943.960146400', 'cgroups': '1456424681.514702900' }
formats = {'stress':'%0.3f', 'signal_insert_delays':'%0.3f','rapl':'%0.3f','powerclamp':'%0.0f','cpufreq':'%0.3f','hypervisor':'%0.0f', 'cgroups':'%0.0f'}
multipliers = {'stress':100, 'signal_insert_delays':100,'rapl':1,'powerclamp':1,'cpufreq':1.0e-6,'hypervisor':float(1.0/12.0), 'cgroups':float(1.0/12000.0)}
units = {'stress':'percent work time', 'signal_insert_delays':'percent work time','rapl':'W','powerclamp':'percent idle time','cpufreq':'GHz','hypervisor':'percent work time', 'cgroups':'percent work time'}

#for experiment in ['stress','signal_insert_delays','powerclamp','hypervisor','rapl','cpufreq']:
for experiment in ['signal_insert_delays']:
	if verbose:
		print "Experiment %s"%(experiment)
	date = dates[experiment]

	expDir = 'experiments/'+experiment
	currentExpDir = expDir+'/'+date

	serverNums = np.array([1,2,3,4])

	if verbose:
		print "  Reading power file..."
	powerData = readPowerFile(currentExpDir+'/powerlog.log',1000,verbose)
	powerData = powerData[:, [0,4,5,6,7] ]

	# Compute the minimum and maximum for all the servers (really, the 0.1 and 99.9 percentiles of the power)
	# This is used to make the linear setpoint model:
	# Desired power = (Pmax - Pmin)*(setpoint - setpoint min)/(setpoint max - setpoint min) + Pmin
	powerRange = np.percentile(powerData[:,1:],[0.1,99.9],axis=0)

	ignoreTime = 10

	sortedSetpoints = []
	sortedErrorOfMean = []
	sortedRMSE = []
	precisionScores = []
	minPrecisionScores = []
	for i in serverNums:
		if verbose:
			print "  Reading files for server %d..."%(i)
		setpoints = readSetpointsFile(currentExpDir+('/server%d/%s.testlog'%(i, date)))
		setpointRange = [np.min(setpoints[:,1]), np.max(setpoints[:,1])]

		powerChunks = []
		for j in range(setpoints.shape[0]-1):
			powerChunks.append(powerData[np.logical_and(powerData[:,0] >= setpoints[j,0]+ignoreTime, powerData[:,0] < setpoints[j+1,0]),i])

		# Don't allow indices which would produce duplicate setpoints
		sortOrder = []
		prevS = str("inf")
		for j in list(setpoints[:-1,1].argsort()):
			if not (setpoints[int(j),1] == prevS):
				sortOrder.append(int(j))
				prevS = setpoints[int(j),1]

		powerSetpoints = ((powerRange[1,i-1] - powerRange[0,i-1])*(setpoints[:,1] - setpointRange[0])/(setpointRange[1] - setpointRange[0]) + powerRange[0,i-1])

		#sortedSetpoints.append([powerSetpoints[j] for j in sortOrder])
		sortedSetpoints.append([setpoints[j,1] for j in sortOrder])
		sortedErrorOfMean.append([(np.mean(powerChunks[j])-powerSetpoints[j]) for j in sortOrder])
		sortedRMSE.append([np.sqrt(np.mean((powerChunks[j]-powerSetpoints[j])**2)) for j in sortOrder])

		precisionScore = []
		for j in range(setpoints.shape[0]-1):
			precisionScore.append(np.mean(np.abs((powerChunks[j] - powerSetpoints[j])/powerSetpoints[j])))
		precisionScores.append(1 - np.abs(np.mean(precisionScore)))
		minPrecisionScores.append( np.min(1.0-np.array(precisionScore)))

	if verbose:
		print "  Done reading files."

	if verbose:
		print "  Conditioning Data..."

	if verbose:
		print "  Plotting..."

if showFig:

	for i in range(4):
		plt.plot(sortedSetpoints[i],sortedRMSE[i],'o')
		#plt.plot(sortedSetpoints[i],sortedErrorOfMean[i],'-')

	plt.title('RMSE tracking error')
	plt.xlabel('Interface setpoint')
	plt.ylabel('Error (W)')
	#plt.legend( ['RMSE','Error of mean'] )
	plt.legend( ['Server 1','Server 2','Server 3','Server 4'],loc='best')
	plt.show()
