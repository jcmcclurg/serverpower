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


for experiment in dates:
	if verbose:
		print "Experiment %s"%(experiment)
	date = dates[experiment]

	expDir = 'experiments/'+experiment
	currentExpDir = expDir+'/'+date

	serverNums = np.array([1,2,3,4])
	
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

	for server in [1]:
		plot_opts = {"violin_width":1, "jitter_marker":None,"jitter_marker_size":0.5, "violin_alpha": 1, "violin_ec": '#95A5A6', "violin_fc": '#F9E79F', "violin_lw": 0.5}

		residencyDescriptionsToPlot = [1,2,5,7]
		height = int(np.floor(np.sqrt(len(residencyDescriptionsToPlot))))
		width = int(np.ceil(float(len(residencyDescriptionsToPlot))/float(height)))
		individualWidth = 6
		individualHeight = 6
		numSetpoints = len(setpointData[server]["setpoints"])
		ticksEvery = 8

		pstateMeans = np.array([np.mean(i[:,2]) for i in setpointData[server]["residencyChunks"]])
		power = pstateMeans - np.min(pstateMeans)
		power = (66-51.5)*power/np.max(power) + (51.5 - 37)

		#c1Means = np.array([np.mean(i[:,5]) for i in setpointData[server]["residencyChunks"]])
		#power *= (1.0-(c1Means/100.0))
		#power += (40 - 37)

		c6Means = np.array([np.mean(i[:,7]) for i in setpointData[server]["residencyChunks"]])
		power *= (1.0-(c6Means/100.0))
		power += 37

		measuredPower = np.array([np.mean(i) for i in setpointData[server]["powerChunks"]])

		fig2 = plt.figure()
		ax = fig2.add_subplot(1,1,1)
		ax.plot(measuredPower - power)
		ax.set_title("Estimated c1 residency for %s"%(experiment))
		ax.set_ylabel("Measured power - power (W)")
		ax.set_xticks(range(1,numSetpoints + 1), minor=True)
		ax.set_xticks(range(1,numSetpoints + 1,ticksEvery), minor=False)
		ax.set_xticklabels([ formats[experiment]%(multipliers[experiment]*setpointData[server]["setpoints"][j]) for j in range(0,numSetpoints,ticksEvery)],rotation=45)
		ax.set_xlabel("Interface setpoint (%s)"%(units[experiment]))

		fig2.set_size_inches(individualHeight, individualWidth, forward=True)
		fig2.tight_layout()
		if saveFig:
			path = currentExpDir+'/'+experiment+'_server'+str(server)+'_estimated_c1.png'
			if verbose:
				print "  Wrote: "+path
			plt.savefig(path, bbox_inches='tight')
		if not showFig:
			plt.close(fig2)

		if verbose:
			print ""

if showFig:
	plt.show()
