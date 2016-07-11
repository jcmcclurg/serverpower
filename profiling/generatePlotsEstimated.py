verbose = True
saveFig = True
showFig = False

if verbose:
	print "Loading general modules..."
import numpy as np
import sys

if verbose:
	print "Loading matplotlib module..."
import matplotlib.pyplot as plt

#if verbose:
#	print "Loading beanplot module..."
#import statsmodels.api as sm

if verbose:
	print "Loading custom functions..."
from defaultGetSetpoints import readSetpointsFile
from defaultGetPower import readPowerFile
from defaultGetResidencies import readResidenciesFile

# 100 seconds per level, interleaved ramp
dates = {'stress': '1452722752.651508100', 'signal_insert_delays':'1452732970.201413700', 'rapl':'1452743186.881235700','powerclamp':'1452753403.717082000','cpufreq':'1452796934.955382300', 'hypervisor': '1452819943.960146400' }
formats = {'stress':'%0.3f', 'signal_insert_delays':'%0.3f','rapl':'%0.3f','powerclamp':'%0.0f','cpufreq':'%0.3f','hypervisor':'%0.0f'}
multipliers = {'stress':100, 'signal_insert_delays':100,'rapl':1,'powerclamp':1,'cpufreq':1.0e-6,'hypervisor':float(1.0/12.0)}
units = {'stress':'percent work time', 'signal_insert_delays':'percent work time','rapl':'W','powerclamp':'percent idle time','cpufreq':'GHz','hypervisor':'percent work time'}

serverNums = np.array([1,2,3,4])

for experiment in ['cpufreq','rapl','powerclamp','hypervisor','stress','signal_insert_delays']:
	if verbose:
		print "Experiment %s"%(experiment)
	date = dates[experiment]

	expDir = 'experiments/'+experiment
	currentExpDir = expDir+'/'+date

	
	if verbose:
		print "  Reading power file..."
	powerData = readPowerFile(currentExpDir+'/powerlog.log',5000,verbose)
	powerData = powerData[:, [0,4,5,6,7] ]
	ignoreTime = 10

	setpointData = {}
	for i in serverNums:
		if verbose:
			print "  Reading files for server %d..."%(i)
		setpoints = readSetpointsFile(currentExpDir+('/server%d/%s.testlog'%(i, date)))
		residencies = readResidenciesFile(currentExpDir+('/server%d/%s.pgadglog'%(i, date)))

		residencyChunks = []
		powerChunks = []
		for j in range(setpoints.shape[0]-1):
			powerChunks.append(powerData[np.logical_and(powerData[:,0] >= setpoints[j,0]+ignoreTime, powerData[:,0] < setpoints[j+1,0]),i])
			residencyChunks.append(residencies[np.logical_and(residencies[:,0] >= setpoints[j,0]+ignoreTime, residencies[:,0] < setpoints[j+1,0]),1:])

		# Don't allow indices which would produce duplicate setpoints
		sortOrder = []
		prevS = str("inf")
		for j in list(setpoints[:-1,1].argsort()):
			if not (setpoints[int(j),1] == prevS):
				sortOrder.append(int(j))
				prevS = setpoints[int(j),1]

		setpointData[i] = {"setpoints": [setpoints[j,1] for j in sortOrder], "powerChunks": [powerChunks[j] for j in sortOrder], "residencyChunks": [residencyChunks[j] for j in sortOrder] }

	if verbose:
		print "  Done reading files."

	# The columns of each residency chunk are:
	#residencyDescriptions = ["Avg_MHz", "%Busy","Bzy_MHz","TSC_MHz","SMI","CPU%c1","CPU%c3","CPU%c6","CPU%c7","CoreTmp","PkgTmp","Pkg%pc2","Pkg%pc3","Pkg%pc6","Pkg%pc7","PkgWatt","CorWatt","RAMWatt","PKG_%","RAM_%"]
	residencyDescriptions = ["Avg_MHz", "Busy time (percent)","Average p-state (MHz)","TSC_MHz","SMI","Time in c-state 1 (percent)","CPU%c3","Time in c-state 6 (percent)","CPU%c7","CoreTmp","PkgTmp","Pkg%pc2","Pkg%pc3","Pkg%pc6","Pkg%pc7","PkgWatt","CorWatt","RAMWatt","PKG_%","RAM_%"]

	if verbose:
		print "  Plotting..."

	for server in serverNums:
		plot_opts = {"violin_width":1, "jitter_marker":None,"jitter_marker_size":0.5, "violin_alpha": 1, "violin_ec": '#95A5A6', "violin_fc": '#F9E79F', "violin_lw": 0.5}

		residencyDescriptionsToPlot = [1,2,5,7]
		height = int(np.floor(np.sqrt(len(residencyDescriptionsToPlot))))
		width = int(np.ceil(float(len(residencyDescriptionsToPlot))/float(height)))
		individualWidth = 6
		individualHeight = 6
		numSetpoints = len(setpointData[server]["setpoints"])
		ticksEvery = 8

		
		measuredPower = np.array([np.median(i) for i in setpointData[server]["powerChunks"]])
		if experiment == 'cpufreq':
			mxTurboFreq = np.median(setpointData[server]["residencyChunks"][-1][:,2])
			mxFreq = np.median(setpointData[server]["residencyChunks"][-2][:,2])
			mnFreq = np.median(setpointData[server]["residencyChunks"][0][:,2])

			mxTurbo = measuredPower[-1]
			mx = measuredPower[-2]
			mn = measuredPower[0]
			idlePower = 37.0
			print [mxTurboFreq, mxFreq, mnFreq, mxTurbo, mx, mn]

		estimatedPower = []
		c = 0
		for i in setpointData[server]["residencyChunks"]:
			c1Means = i[:,5]
			sleepPower1 = (mn)*(c1Means/100.0)

			c6Means = i[:,7]
			sleepPower2 = idlePower*(c6Means/100.0)

			activePower = i[:,2]
			turboRange = activePower > mxFreq
			nonTurboRange = activePower <= mxFreq
			#print "%g/%g..."%(np.sum(turboRange), np.sum(nonTurboRange))
			activePower[turboRange] = (mxTurbo - mx)*(activePower[turboRange] - mxFreq)/(mxTurboFreq - mxFreq) + mx
			activePower[nonTurboRange] = (mx - mn)*(activePower[nonTurboRange] - mnFreq)/(mxFreq - mnFreq) + mn
			activePower = activePower*(100.0 - c1Means - c6Means)/100.0

			power = activePower + sleepPower1 + sleepPower2
			estimatedPower.append(np.median(power))

		estimatedPower = np.array(estimatedPower)

		fig2 = plt.figure()
		ax = fig2.add_subplot(1,1,1)
		fig2.set_size_inches(individualHeight, individualWidth, forward=True)
		mp, = ax.plot(measuredPower)
		ep, = ax.plot(estimatedPower)
		ax.set_title("Residency-based power estimation for %s"%(experiment))
		ax.set_ylabel("Median power (W)")
		ax.set_ylim(35,75)
		ax.legend([mp,ep],['Measured','Estimated'],loc='best')
		ax.set_xticks(range(1,numSetpoints + 1), minor=True)
		ax.set_xticks(range(1,numSetpoints + 1,ticksEvery), minor=False)
		ax.set_xticklabels([ formats[experiment]%(multipliers[experiment]*setpointData[server]["setpoints"][j]) for j in range(0,numSetpoints,ticksEvery)],rotation=45)
		ax.set_xlabel("Interface setpoint (%s)"%(units[experiment]))

		fig2.tight_layout()
		if saveFig:
			path = currentExpDir+'/'+experiment+'_server'+str(server)+'_estimated.png'
			if verbose:
				print "  Wrote: "+path
			plt.savefig(path, bbox_inches='tight')
		if not showFig:
			plt.close(fig2)

		if verbose:
			print ""

if showFig:
	plt.show()
