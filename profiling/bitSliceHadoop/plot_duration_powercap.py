verbose = True
saveFig = False
showFig = True



if verbose:
	print "Loading general modules..."
import numpy as np
import gzip
import json
#import exceptions

if verbose:
	print "Loading matplotlib module..."
import matplotlib.pyplot as plt

if verbose:
	print "Loading custom functions..."

from defaultGetPower import readPowerFile, rawFileToPowerFile
from defaultJobEnergy import readJobEnergy, generateJobEnergyFile
from my_plot_common import get_cmap, get_mean_with_err

knnClasses = ['KnnArray','KnnHorizontal','KnnRange']
#knnClasses = ['KnnArray']
#higgsNums = [5,10,15,20,28]
higgsNums = [28,20,15,10,5]

expFullRange = range(22,35+1)

expIDs = ["Jul27_powercap",
		"Jul28_powercap",
		"Jul18_28_powercap",
		"Jul28_powercap2",
		"Jul28_powercap3",
		"Jul28_powercap4",
		"Jul28_powercap5",
		"Jul29_powercap",
		"Jul29_powercap2",
		"Jul29_powercap3"]

#knnClasses = ['KnnHorizontal']
#higgsNums = [15]
#expRange = [22, 35, 23, 34, 24,  33, 25, 32, 26, 31, 27, 30, 28, 29]
expRange = [22, 35, 25, 32, 28]

expData = {}

# Seconds before and after the job start time to plot.
jobTimeBuffer = 10

expIDCounter = 0
for expID in expIDs:
	expData[expID] = {}
	for expNum in expFullRange:
		exp = expID+"/exp%d"%(expNum)

		#knnClass = knnClasses[0]
		#higgsNum = higgsNums[0]

		expDir = "experiments/"+exp
		#if verbose:
		#	print "Loading %s..."%(expDir)

		height=7
		width=10

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
				#expData[expID][expNum][higgsNum][knnClass] = generateJobEnergyFile(expDir+"/"+i,"job.energy",verbose)


numColors=36
cmap = get_cmap(numColors)
colorStartIndex = 0

baselinePowerToSubtract = 203.721

figs = {"uncapped_power_vs_time":{"title":"Example Uncapped Power","xlabel":"Time (s)","ylabel":"Power (W)"},
		"capped_power_vs_time":{"title":"Example Capped Power","xlabel":"Time (s)","ylabel":"Power (W)"},
		"energy_vs_power_cap":{"title":"Average Energy with Power Cap","xlabel":"Per-Processor Power Cap (W)","ylabel":"Energy (kJ)"},
		"duration_vs_power_cap":{"title":"Average Duration with Power Cap","xlabel":"Per-Processor Power Cap (W)","ylabel":"Duration (s)"},
		"energy_vs_dimensionality":{"title":"Average Energy with Dimensionality","xlabel":"Number of Dimensions","ylabel":"Energy (kJ)"},
		"duration_vs_dimensionality":{"title":"Average Duration with Dimensionality","xlabel":"Number of Dimensions","ylabel":"Duration (s)"}}

figs["energy_vs_dimensionality"]["generate"]		= False
figs["duration_vs_dimensionality"]["generate"]	= False
figs["energy_vs_power_cap"]["generate"]			= False
figs["duration_vs_power_cap"]["generate"]		= False
figs["capped_power_vs_time"]["generate"]		= True
figs["uncapped_power_vs_time"]["generate"]		= True





figs["energy_vs_dimensionality"]["powerCap"]   = 35
figs["duration_vs_dimensionality"]["powerCap"] = 35

figs["energy_vs_power_cap"]["dimension"]   = 28
figs["duration_vs_power_cap"]["dimension"] = 28


figs["capped_power_vs_time"]["powerCap"]   = 29
figs["uncapped_power_vs_time"]["powerCap"] = 35

figs["capped_power_vs_time"]["dimension"]   = 28
figs["uncapped_power_vs_time"]["dimension"] = 28

figs["capped_power_vs_time"]["exps"]   = ["Jul28_powercap"]
figs["uncapped_power_vs_time"]["exps"] = ["Jul28_powercap"]


for figId in figs:
	if figs[figId]["generate"]:
		fig = plt.figure()
		ax = fig.add_subplot(111)
		figs[figId]["fig"] = fig
		figs[figId]["ax"] = ax
		figs[figId]["legend"] = []


###################
# Power Cap Plots #
###################
if figs["energy_vs_power_cap"]["generate"] or figs["duration_vs_power_cap"]["generate"]:
	rng = np.array(expRange)
	s = np.argsort(rng)
	rng = rng[s]
	enDim = figs["energy_vs_power_cap"]["dimension"]
	durDim = figs["duration_vs_power_cap"]["dimension"]
	classCount = 0
	for knnClass in knnClasses:
		colorIndex = np.mod(colorStartIndex + classCount*numColors/len(knnClasses),numColors)

		dur = []
		mpwr = []
		for expID in expIDs:
			dur.append([expData[expID][e][durDim][knnClass]["jobDuration"] for e in rng])
			mpwr.append([expData[expID][e][enDim][knnClass]["meanPower"] for e in rng])

		dur = np.array(dur)
		mpwr = np.array(mpwr)
		enrg = dur*(mpwr - baselinePowerToSubtract)/1000.0

		duration,derr = get_mean_with_err(dur)
		energy,eerr = get_mean_with_err(enrg)

		color = cmap(colorIndex)

		if figs["energy_vs_power_cap"]["generate"]:
			figs["energy_vs_power_cap"]["ax"].errorbar(rng,energy,yerr=eerr,color=color,linestyle='dashed',marker='o',markerfacecolor=color)
			figs["energy_vs_power_cap"]["ax"].set_xlim([21,36])
			figs["energy_vs_power_cap"]["ax"].set_ylim([0,7])
			figs["energy_vs_power_cap"]["legend"].append("%s (d%d)"%(knnClass,enDim))

		if figs["duration_vs_power_cap"]["generate"]:
			figs["duration_vs_power_cap"]["ax"].errorbar(rng,duration,yerr=derr,color=color,linestyle='dashed',marker='o',markerfacecolor=color)
			figs["duration_vs_power_cap"]["ax"].set_xlim([21,36])
			figs["duration_vs_power_cap"]["ax"].set_ylim([0,100])
			figs["duration_vs_power_cap"]["legend"].append("%s (d%d)"%(knnClass,durDim))

		classCount += 1


###################
# Dimension Plots #
###################
if figs["energy_vs_dimensionality"]["generate"] or figs["duration_vs_dimensionality"]["generate"]:
	rng = np.array(higgsNums)
	s = np.argsort(rng)
	rng = rng[s]
	enCap = figs["energy_vs_dimensionality"]["powerCap"]
	durCap = figs["duration_vs_dimensionality"]["powerCap"]
	classCount = 0
	for knnClass in knnClasses:
		colorIndex = np.mod(colorStartIndex + classCount*numColors/len(knnClasses),numColors)
		dur = []
		mpwr = []

		for expID in expIDs:
			dur.append([expData[expID][durCap][e][knnClass]["jobDuration"] for e in rng])
			mpwr.append([expData[expID][enCap][e][knnClass]["meanPower"] for e in rng])

		dur = np.array(dur)
		mpwr = np.array(mpwr)
		enrg = dur*(mpwr - baselinePowerToSubtract)/1000.0

		duration,derr = get_mean_with_err(dur)
		energy,eerr = get_mean_with_err(enrg)

		color = cmap(colorIndex)

		if figs["energy_vs_dimensionality"]["generate"]:
			figs["energy_vs_dimensionality"]["ax"].errorbar(rng,energy,yerr=eerr,color=color,linestyle='dashed',marker='o',markerfacecolor=color)
			figs["energy_vs_dimensionality"]["ax"].set_xlim([3,30])
			figs["energy_vs_dimensionality"]["ax"].set_ylim([0,7])
			figs["energy_vs_dimensionality"]["legend"].append("%s (c%d)"%(knnClass,enCap))

		if figs["duration_vs_dimensionality"]["generate"]:
			figs["duration_vs_dimensionality"]["ax"].errorbar(rng,duration,yerr=derr,color=color,linestyle='dashed',marker='o',markerfacecolor=color)
			figs["duration_vs_dimensionality"]["ax"].set_xlim([3,30])
			figs["duration_vs_dimensionality"]["ax"].set_ylim([0,100])
			figs["duration_vs_dimensionality"]["legend"].append("%s (c%d)"%(knnClass,durCap))

		classCount += 1


##############
# Time Plots #
##############
if figs["uncapped_power_vs_time"]["generate"] or figs["capped_power_vs_time"]["generate"]:
	classCount = 0

	# Seconds before and after the job start time to plot.
	jobTimeBuffer = 10
	for knnClass in knnClasses:

		for figId in ["uncapped_power_vs_time","capped_power_vs_time"]:
			if figs[figId]["generate"]:
				colorIndex = np.mod(colorStartIndex + classCount*numColors/len(knnClasses),numColors)
				for expID in figs[figId]["exps"]:
					powerCap = figs[figId]["powerCap"]
					dim = figs[figId]["dimension"]

					exp = expID+"/exp%d"%(powerCap)
					i = "%s%d"%(knnClass,dim)
					expDir = "experiments/"+exp+"/"+i
					f = expDir+"/powerlog.log"

					blocklen = 10000
					powerData = readPowerFile(f,blocklen,verbose)
					#powerData = rawFileToPowerFile(f,f+"_%d.gz"%(blocklen),blocklen,verbose)
					powerData = powerData[:, [0,3,4,5,6,7] ]
					powerStartTime = powerData[0,0]
					powerEndTime = powerData[-1,0]

					jobStartTime = expData[expID][powerCap][dim][knnClass]["jobStartTime"]
					jobEndTime = expData[expID][powerCap][dim][knnClass]["jobEndTime"]
					assert(jobEndTime != -float("inf"))
					assert(jobStartTime != float("inf"))

					powerRange = np.logical_and(powerData[:,0] >= jobStartTime - jobTimeBuffer,powerData[:,0] <= jobEndTime + jobTimeBuffer)
					rng = powerData[powerRange,0] - jobStartTime

					fp = gzip.open(expDir+"/sparkEvents.json.gz",'rb')
					j = json.load(fp)
					fp.close()
					# Collect task starts
					taskInfo = {}
					for event in j["events"]:
						if ("eventinfo" in event) and ("Event" in event["eventinfo"]) and ("Task Info" in event["eventinfo"]) and ("Task ID" in event["eventinfo"]["Task Info"]) and ("timestamp" in event):
							taskID = int(event["eventinfo"]["Task Info"]["Task ID"])
							timestamp = float(event["timestamp"])/1000.0
							eventType = event["eventinfo"]["Event"]

							if taskID in taskInfo:
								if eventType == "SparkListenerTaskStart":
									taskInfo[taskID]["start"] = timestamp
								elif eventType == "SparkListenerTaskEnd":
									taskInfo[taskID]["end"] = timestamp
							else:
								if eventType == "SparkListenerTaskStart":
									taskInfo[taskID] = {"start":timestamp}
								elif eventType == "SparkListenerTaskEnd":
									taskInfo[taskID] = {"end":timestamp}


					color = cmap(colorIndex)
					#color2 = (color[0],color[1],color[2],0.5)
					#line, = figs[figId]["ax"].plot(rng,np.sum(powerData[powerRange,1:], axis=1),color=color2,linestyle='',marker='.',markerfacecolor=color2)
					line, = figs[figId]["ax"].plot(rng,np.sum(powerData[powerRange,1:], axis=1),color=color,linestyle='-',marker='')
					figs[figId]["legend"].append("%s (d%d c%d)"%(knnClass,dim,powerCap))

					ymin = 180
					ymax = 400

					taskYmin = 200
					taskYmax = 250
					yvals= np.linspace(taskYmin,taskYmax,len(taskInfo))
					counter = 0
					for taskID in taskInfo:
						if ("start" in taskInfo[taskID]) and ("end" in taskInfo[taskID]):
							start = taskInfo[taskID]["start"]
							stop = taskInfo[taskID]["end"]
							offset = yvals[counter]
							assert(start <= stop)
							figs[figId]["ax"].annotate("",xy=(start-jobStartTime, offset), xycoords='data',xytext=(stop-jobStartTime, offset), textcoords='data', color=color, arrowprops=dict(arrowstyle="<->",connectionstyle="arc3"))
						else:
							print "WARNING: Task %d only has %s"%(taskID , [k for k in taskInfo[taskID]] )

					figs[figId]["ax"].set_xlim([-10,90])
					figs[figId]["ax"].set_ylim([180,400])

					colorIndex += 1

		classCount += 1

####################
# Save the figures #
####################
for figId in figs:
	if figs[figId]["generate"]:
		fig = figs[figId]["fig"]
		ax = figs[figId]["ax"]

		fig.set_size_inches(width, height, forward=True)

		if len(figs[figId]["legend"]) > 0:
			box = ax.get_position()
			ax.set_position([box.x0, box.y0, box.width * 0.75, box.height])
			ax.legend(figs[figId]["legend"],loc='center left', bbox_to_anchor=(1, 0.5))

		ax.set_title(figs[figId]["title"])
		ax.set_xlabel(figs[figId]["xlabel"])
		ax.set_ylabel(figs[figId]["ylabel"])
		fig.canvas.draw()
		plt.show(fig)

		if saveFig:
			plt.figure(fig.number)
			plt.savefig("experiments/higgs_%s.png"%(figId))#, bbox_inches='tight')
			#plt.savefig("experiments/higgs_%s_%s.png"%(figId,"-".join(expIDs)) )#, bbox_inches='tight')

		if not showFig:
			plt.close(fig)

