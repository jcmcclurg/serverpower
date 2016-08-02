verbose = True
saveFig = True
showFig = False

if verbose:
	print "Loading general modules..."
import numpy as np

#import exceptions

if verbose:
	print "Loading matplotlib module..."
import matplotlib.pyplot as plt

if verbose:
	print "Loading custom functions..."

#from defaultGetPower import readPowerFile, rawFileToPowerFile
from defaultJobEnergy import readJobEnergy, generateJobEnergyFile
from my_plot_common import get_cmap, get_mean_with_err

knnClasses = ['KnnArray','KnnHorizontal','KnnRange']
#knnClasses = ['KnnArray']
#higgsNums = [5,10,15,20,28]
higgsNums = [28,20,15,10,5]
expRange = range(22,35+1)

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
	for expNum in expRange:
		exp = expID+"/exp%d"%(expNum)

		#knnClass = knnClasses[0]
		#higgsNum = higgsNums[0]

		expDir = "experiments/"+exp

		height=7
		width=10

		expData[expID][expNum] = {}
		for higgsNum in higgsNums:

			expData[expID][expNum][higgsNum] = {}
			for knnClass in knnClasses:
				i = "%s%d"%(knnClass,higgsNum)
				expData[expID][expNum][higgsNum][knnClass] = readJobEnergy(expDir+"/"+i,"job.energy",verbose)


rng = np.array(expRange)

s = np.argsort(rng)
rng = rng[s]

legends = []

numColors=36
cmap = get_cmap(numColors)
colorStartIndex = 0

baselinePowerToSubtract = 203.721
fig = plt.figure()
ax = fig.add_subplot(111)

fig2 = plt.figure()
ax2 = fig2.add_subplot(111)
classCount = 0
for knnClass in knnClasses:
	colorIndex = np.mod(colorStartIndex + classCount*numColors/len(knnClasses),numColors)
	for higgsNum in higgsNums:
		dur = []
		mpwr = []
		for expID in expIDs:
			dur.append([expData[expID][e][higgsNum][knnClass]["jobDuration"] for e in rng])
			mpwr.append([expData[expID][e][higgsNum][knnClass]["meanPower"] for e in rng])

		dur = np.array(dur)
		mpwr = np.array(mpwr)
		enrg = dur*(mpwr - baselinePowerToSubtract)/1000.0

		duration,derr = get_mean_with_err(dur)
		energy,eerr = get_mean_with_err(enrg)

		color = cmap(colorIndex)

		ax.errorbar(rng,energy,yerr=eerr,color=color,linestyle='dashed',marker='o',markerfacecolor=color)
		ax2.errorbar(rng,duration,yerr=derr,color=color,linestyle='dashed',marker='o',markerfacecolor=color)

		legends.append("%s%d"%(knnClass,higgsNum))
		colorIndex += 1
	#plt.plot(rng,meanPower)
	classCount += 1

fig.set_size_inches(width, height, forward=True)
#fig.tight_layout()

fig2.set_size_inches(width, height, forward=True)
#fig2.tight_layout()

box = ax.get_position()
ax.set_position([box.x0, box.y0, box.width * 0.75, box.height])
ax.legend(legends,loc='center left', bbox_to_anchor=(1, 0.5))

box = ax2.get_position()
ax2.set_position([box.x0, box.y0, box.width * 0.75, box.height])
ax2.legend(legends,loc='center left', bbox_to_anchor=(1, 0.5))

ax.set_title("Job energy for Higgs dataset")
ax.set_xlabel('Processor power cap (W)')
ax.set_ylabel('Job energy (kJ)')
ax.set_ylim(0,7)
ax.set_xlim(21,36)

ax2.set_title("Job duration for Higgs dataset")
ax2.set_xlabel('Processor power cap (W)')
ax2.set_ylabel('Job duration (s)')
ax2.set_xlim(21,36)
ax2.set_ylim(0,100)

fig.canvas.draw()
fig2.canvas.draw()

plt.show(fig)
plt.show(fig2)

if saveFig:
	plt.figure(fig.number)
	#plt.savefig("experiments/higgs_energy_%s.png"%("-".join(expIDs)) )#, bbox_inches='tight')
	plt.savefig("experiments/higgs_energy.png")

	plt.figure(fig2.number)
	#plt.savefig("experiments/higgs_duration_%s.png"%("-".join(expIDs)) )#, bbox_inches='tight')
	plt.savefig("experiments/higgs_duration.png")

if not showFig:
	plt.close(fig)
	plt.close(fig2)
