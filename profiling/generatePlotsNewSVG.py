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
	print "Loading beanplot module..."
import statsmodels.api as sm

if verbose:
	print "Loading custom functions..."
from defaultGetSetpoints import readSetpointsFile
from defaultGetPower import readPowerFile
from defaultGetResidencies import readResidenciesFile

# 100 seconds per level, interleaved ramp
dates = {'stress': '1452722752.651508100', 'signal_insert_delays':'1452732970.201413700', 'rapl':'1452743186.881235700','powerclamp':'1452753403.717082000','cpufreq':'1452796934.955382300', 'hypervisor': '1452819943.960146400', 'cgroups': '1456424681.514702900' }
formats = {'stress':'%0.3f', 'signal_insert_delays':'%0.3f','rapl':'%0.3f','powerclamp':'%0.0f','cpufreq':'%0.3f','hypervisor':'%0.0f', 'cgroups':'%0.0f'}
multipliers = {'stress':100, 'signal_insert_delays':100,'rapl':1,'powerclamp':1,'cpufreq':1.0e-6,'hypervisor':float(1.0/12.0), 'cgroups':float(1.0/12000.0)}
units = {'stress':'percent work time', 'signal_insert_delays':'percent work time','rapl':'W','powerclamp':'percent idle time','cpufreq':'GHz','hypervisor':'percent work time', 'cgroups':'percent work time'}

def reject_outliers(data, m = 2.):
	d = np.abs(data - np.median(data))
	mdev = np.median(d)
	if mdev:
		return data[(d/mdev) < m]
	else:
		return data[:]

#for experiment in ['stress','signal_insert_delays','powerclamp','hypervisor','rapl','cpufreq']:
#for experiment in ['stress','signal_insert_delays','powerclamp','hypervisor','rapl','cpufreq','cgroups']:
for experiment in ['cgroups']:
	if verbose:
		print "Experiment %s"%(experiment)
	date = dates[experiment]

	expDir = 'experiments/'+experiment
	currentExpDir = expDir+'/'+date

	serverNums = np.array([4])
	#serverNums = np.array([1,2,3,4])
	#serverNums = np.array([1,2,3])

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
			powerChunks.append(reject_outliers(powerData[np.logical_and(powerData[:,0] >= setpoints[j,0]+ignoreTime, powerData[:,0] < setpoints[j+1,0]),i]))

			rs = residencies[np.logical_and(residencies[:,0] >= setpoints[j,0]+ignoreTime, residencies[:,0] < setpoints[j+1,0]),1:]
			resNoOutliers = []
			for k in range(rs.shape[1]):
				resNoOutliers.append(reject_outliers(rs[:,k]))

			residencyChunks.append(resNoOutliers)

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

	if verbose:
		print "  Conditioning Data..."

	# The columns of each residency chunk are:
	residencyDescriptions = ["Avg_MHz", "%Busy","Bzy_MHz","TSC_MHz","SMI","CPU%c1","CPU%c3","CPU%c6","CPU%c7","CoreTmp","PkgTmp","Pkg%pc2","Pkg%pc3","Pkg%pc6","Pkg%pc7","PkgWatt","CorWatt","RAMWatt","PKG_%","RAM_%"]
	#residencyDescriptions = ["Avg_MHz", "Busy time (percent)","Average p-state (MHz)","TSC_MHz","SMI","Time in c-state 1 (percent)","CPU%c3","Time in c-state 6 (percent)","CPU%c7","CoreTmp","PkgTmp","Pkg%pc2","Pkg%pc3","Pkg%pc6","Pkg%pc7","PkgWatt","CorWatt","RAMWatt","PKG_%","RAM_%"]
	for server in serverNums:
		adjusted = False
		# k is the chunk index
		for k in range(len(setpointData[server]["residencyChunks"])):
			# j is the residency description index
			for j in range(len(setpointData[server]["residencyChunks"][k])):
				# Make sure that the data doesn't have all identical values. This messes up the beanplot.
				if np.std(setpointData[server]["residencyChunks"][k][j]) <= 0.0:
					adjusted = True
					setpointData[server]["residencyChunks"][k][j] += np.random.rand(setpointData[server]["residencyChunks"][k][j].shape[0])/1e9 - (0.5/1.0e9)
					assert(np.std(setpointData[server]["residencyChunks"][k][j]) > 0.0)

		if verbose and adjusted:
			print "  Server %d: Had to adjust residency values."%(server)

	if verbose:
		print "  Plotting..."

	for server in serverNums:
		fig1 = plt.figure()
		plot_opts = {"violin_width":1, "jitter_marker":None,"jitter_marker_size":0.5, "violin_alpha": 1, "violin_ec": '#95A5A6', "violin_fc": '#F9E79F', "violin_lw": 0.5}
		#plot_opts = {"violin_width":1, "jitter_marker":None,"jitter_marker_size":0, "violin_alpha": 1, "violin_ec": '#95A5A6', "violin_fc": '#F9E79F', "violin_lw": 0.5, "bean_mean_lw":1, "bean_mean_size":2}

		residencyDescriptionsToPlot = [1,2,5,7]
		height = int(np.floor(np.sqrt(len(residencyDescriptionsToPlot))))
		width = int(np.ceil(float(len(residencyDescriptionsToPlot))/float(height)))
		individualWidth = 6
		individualHeight = 6
		numSetpoints = len(setpointData[server]["setpoints"])
		ticksEvery = 8
		counter = 1
		for j in residencyDescriptionsToPlot:
			residencyDescription = residencyDescriptions[j]
			ax = fig1.add_subplot(height,width,counter)

			try:
				sm.graphics.beanplot([i[j] for i in setpointData[server]["residencyChunks"]], ax=ax, jitter=True, plot_opts=plot_opts)
				print "  Plotted %s"%(residencyDescription)
			except Exception as e:
				print "  Error (%s) with %s (%d)"%(e,residencyDescription,j)
				print min([np.std(setpointData[server]["residencyChunks"][k][j]) for k in range(len(setpointData[server]["residencyChunks"]))])

			if counter == int(np.ceil(width/2)):
				ax.set_title("State distribution for %s interface"%(experiment),fontsize=14)
			#ax.set_title(str(residencyDescription))
			ax.set_ylabel(str(residencyDescription))
			lim = ax.get_ylim()
			bottom = lim[0]
			top = lim[1]
			if lim[1]-lim[0] < 1e3:
				bottom = lim[0]*0.5
				top = lim[1]*1.05

			if lim[0] < 0:
				bottom = -0.01

			if lim[1] < 0.1:
				top = 0.1

			ax.set_ylim(bottom,top)
			xticks = np.array(range(1,numSetpoints + 1))
			ax.set_xticks(xticks, minor=True)

			xtickValues = np.round(multipliers[experiment]*np.array(setpointData[server]["setpoints"]))
			xmajTicks = xticks[np.nonzero(np.mod(xtickValues,10) == 0)]
			labels = [ formats[experiment]%(xtickValues[k-1]) for k in xmajTicks ]

			ax.set_xticks(xmajTicks, minor=False)
			if counter > len(residencyDescriptionsToPlot) - width:
				ax.set_xticklabels(labels,rotation=45)
				ax.set_xlabel("Interface setpoint (%s)"%(units[experiment]))
			else:
				ax.set_xticklabels([])
			counter += 1
		fig1.set_size_inches(height*individualHeight, width*individualWidth, forward=True)
		fig1.tight_layout()
		if saveFig:
			path = currentExpDir+'/'+experiment+'_server'+str(server)+'_residency_short.svg'
			if verbose:
				print "  Wrote: "+path
			plt.savefig(path, bbox_inches='tight')
		if not showFig:
			plt.close(fig1)

		fig2 = plt.figure()
		ax = fig2.add_subplot(1,1,1)
		fig2.set_size_inches(individualHeight, individualWidth, forward=True)

		sm.graphics.beanplot(setpointData[server]["powerChunks"], ax=ax, jitter=True, plot_opts=plot_opts)
		ax.set_title("Power distribution for %s interface"%(experiment))
		ax.set_ylabel("Power distribution (W)")
		ax.set_ylim(30,90)
		xticks = np.array(range(1,numSetpoints + 1))
		ax.set_xticks(xticks, minor=True)

		xtickValues = np.round(multipliers[experiment]*np.array(setpointData[server]["setpoints"]))
		xmajTicks = xticks[np.nonzero(np.mod(xtickValues,10) == 0)]
		labels = [ formats[experiment]%(xtickValues[k-1]) for k in xmajTicks ]

		ax.set_xticks(xmajTicks, minor=False)
		ax.set_xticklabels(labels,rotation=45)
		ax.set_xlabel("Interface setpoint (%s)"%(units[experiment]))

		ax.set_xlabel("Interface setpoint (%s)"%(units[experiment]))

		fig2.tight_layout()
		print ax.get_ylim()
		if saveFig:
			path = currentExpDir+'/'+experiment+'_server'+str(server)+'_power.svg'
			if verbose:
				print "  Wrote: "+path
			plt.savefig(path, bbox_inches='tight')
		if not showFig:
			plt.close(fig2)

		if verbose:
			print ""

if showFig:
	plt.show()
