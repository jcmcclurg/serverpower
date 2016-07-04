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

if verbose:
	print "Loading custom functions..."
from getPlotData import readPlotData

# 100 seconds per level, interleaved ramp
dates = {'stress': '1452722752.651508100', 'signal_insert_delays':'1452732970.201413700', 'rapl':'1452743186.881235700','powerclamp':'1452753403.717082000','cpufreq':'1452796934.955382300', 'hypervisor': '1452819943.960146400', 'cgroups': '1456424681.514702900' }
formats = {'stress':'%0.3f', 'signal_insert_delays':'%0.3f','rapl':'%0.3f','powerclamp':'%0.0f','cpufreq':'%0.3f','hypervisor':'%0.0f', 'cgroups':'%0.0f'}
multipliers = {'stress':100, 'signal_insert_delays':100,'rapl':1,'powerclamp':1,'cpufreq':1.0e-6,'hypervisor':float(1.0/12.0), 'cgroups':float(1.0/12000.0)}
units = {'stress':'percent work time', 'signal_insert_delays':'percent work time','rapl':'W','powerclamp':'percent idle time','cpufreq':'GHz','hypervisor':'percent work time', 'cgroups':'percent work time'}

residencyDescriptions = ["Avg_MHz", "Busy time (percent)","Average p-state (MHz)","TSC_MHz","SMI","Time in c-state 1 (percent)","CPU%c3","Time in c-state 6 (percent)","CPU%c7","CoreTmp","PkgTmp","Pkg%pc2","Pkg%pc3","Pkg%pc6","Pkg%pc7","PkgWatt","CorWatt","RAMWatt","PKG_%","RAM_%"]

colors = {'stress': 'b-', 'signal_insert_delays':'g-', 'rapl':'r-','powerclamp':'c-','cpufreq':'m-', 'hypervisor': 'y-', 'cgroups': 'k-' }

expList = ['stress','signal_insert_delays','powerclamp','hypervisor','rapl','cpufreq','cgroups']


plotData = {}
for experiment in expList:
	if verbose:
		print "Experiment %s"%(experiment)
	date = dates[experiment]

	plotData[experiment] = readPlotData(experiment,date,"experiments/"+experiment+"/"+date+"/plotData_%s_%s.gz"%(experiment,date))

	if verbose:
		print "  Done reading files."

residencyDescriptionsToPlot = [1,2,5,7]

height = int(np.floor(np.sqrt(len(residencyDescriptionsToPlot))))
width = int(np.ceil(float(len(residencyDescriptionsToPlot))/float(height)))
individualWidth = 6
individualHeight = 6

if verbose:
	print "Plotting..."

	#serverNums = np.array([1,2,3,4])
	serverNums = np.array([4])

	for server in serverNums:
		#########################
		# Residency plots
		#########################
		fig = plt.figure()
		figStddev = plt.figure()

		counter = 1
		for j in residencyDescriptionsToPlot:
			ax = fig.add_subplot(width,height,counter)
			axStddev = figStddev.add_subplot(width,height,counter)

			residencyDescription = residencyDescriptions[j]

			for experiment in expList:
				sp = plotData[experiment][server-1]["setpoints"]
				if experiment == "powerclamp":
					sp = sp[::-1]

				mn = np.min(sp)
				mx = np.max(sp)

				ax.plot((sp-mn)/(mx-mn),plotData[experiment][server-1]["residencyMeans"][:,j], colors[experiment],label=experiment)
				axStddev.plot((sp-mn)/(mx-mn),plotData[experiment][server-1]["residencyStddevs"][:,j], colors[experiment],label=experiment)

			if counter == int(np.ceil(width/2)):
				ax.set_title("Residency means for all interfaces (server %d)"%(server))
				axStddev.set_title("Residency stddevs for all interfaces (server %d)"%(server))
			ax.set_ylabel("Mean of "+str(residencyDescription))
			axStddev.set_ylabel("Stddev of "+str(residencyDescription))

			for axx in [ax,axStddev]:
				lim = axx.get_ylim()
				bottom = lim[0]
				top = lim[1]
				if lim[1]-lim[0] < 1e3:
					bottom = lim[0]*0.5
					top = lim[1]*1.05

				if lim[0] < 0:
					bottom = -0.01

				if lim[1] < 0.1:
					top = 0.1

				axx.set_ylim(bottom,top)

				if counter > len(residencyDescriptionsToPlot) - width:
					axx.set_xlabel("Normalized interface setpoint")
				else:
					axx.set_xticklabels([])

				if counter == len(residencyDescriptionsToPlot):
					axx.legend(loc='upper left',bbox_to_anchor=(1, 0.5))

			counter += 1
		for figg in [fig,figStddev]:
			figg.set_size_inches(individualHeight*width, individualWidth*height, forward=True)
			figg.tight_layout()

		if saveFig:
			plt.figure(fig.number)
			path = 'experiments/server'+str(server)+'_residency_means.svg'
			if verbose:
				print "  Wrote: "+path
			plt.savefig(path, bbox_inches='tight')

		if not showFig:
			plt.close(fig)

		if saveFig:
			plt.figure(figStddev.number)
			path = 'experiments/server'+str(server)+'_residency_stddevs.svg'
			if verbose:
				print "  Wrote: "+path
			plt.savefig(path, bbox_inches='tight')

		if not showFig:
			plt.close(figStddev)

		#####################
		# Power plots
		#####################
		fig = plt.figure()
		ax = fig.add_subplot(1,1,1)

		figStddev = plt.figure()
		axStddev = figStddev.add_subplot(1,1,1)
		for experiment in expList:
				sp = plotData[experiment][server-1]["setpoints"]
				if experiment == "powerclamp":
					sp = sp[::-1]

				mn = np.min(sp)
				mx = np.max(sp)

				ax.plot((sp-mn)/(mx-mn),plotData[experiment][server-1]["powerMeans"], colors[experiment],label=experiment)
				axStddev.plot((sp-mn)/(mx-mn),plotData[experiment][server-1]["powerStddevs"], colors[experiment],label=experiment)

		ax.set_title("Power means for all interfaces (server %d)"%(server))
		ax.set_ylabel("Mean of power (W)")
		axStddev.set_title("Power stddevs for all interfaces (server %d)"%(server))
		axStddev.set_ylabel("Stddev of power (W)")

		for axx in [ax, axStddev]:
			axx.set_xlabel("Normalized interface setpoint")
			axx.legend(loc='upper left',bbox_to_anchor=(1, 0.5))
			#lim = ax.get_ylim()
			#bottom = lim[0]
			#top = lim[1]
			#if lim[1]-lim[0] < 1e3:
			#	bottom = lim[0]*0.5
			#	top = lim[1]*1.05

			#if lim[0] < 0:
			#	bottom = -0.01

			#if lim[1] < 0.1:
			#	top = 0.1

			#axx.set_ylim(bottom,top)

		for figg in [fig,figStddev]:
			figg.set_size_inches(individualHeight, individualWidth, forward=True)
			figg.tight_layout()

		if saveFig:
			plt.figure(fig.number)
			path = 'experiments/server'+str(server)+'_power_means.svg'
			if verbose:
				print "  Wrote: "+path
			plt.savefig(path, bbox_inches='tight')

		if not showFig:
			plt.close(fig)

		if saveFig:
			plt.figure(figStddev.number)
			path = 'experiments/server'+str(server)+'_power_stddevs.svg'
			if verbose:
				print "  Wrote: "+path
			plt.savefig(path, bbox_inches='tight')

		if not showFig:
			plt.close(figStddev)

		if verbose:
			print ""

if showFig:
	plt.show()
