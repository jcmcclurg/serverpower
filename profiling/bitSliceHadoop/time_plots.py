verbose = True
saveFig = True
showFig = False

if verbose:
	print "Loading general modules..."
import numpy as np
import json
import gzip
import sys

if verbose:
	print "Loading matplotlib module..."
import matplotlib.pyplot as plt

if verbose:
	print "Loading custom functions..."
from defaultGetPower import readPowerFile, rawFileToPowerFile

if verbose:
	print "  Reading power file..."

knnClasses = ['KnnHorizontal','KnnRange']
higgsNums = [5,10,15,20,28]
expRange = range(22,35+1)

#knnClasses = ['KnnHorizontal']
#higgsNums = [15]
expRange = [22, 35, 23, 34, 24, 33, 25, 32, 26, 31, 27, 30, 28, 29]
#expRange = [25, 32, 26, 31, 27, 30, 28, 29]

# Seconds before and after the job start time to plot.
jobTimeBuffer = 10
for expNum in expRange:
	exp = "Jul18_powercap/exp%d"%(expNum)

	#knnClass = knnClasses[0]
	#higgsNum = higgsNums[0]

	expDir = "experiments/"+exp

	height=7
	width=10

	for higgsNum in higgsNums:
		fig = plt.figure()
		ax = fig.add_subplot(111)

		for knnClass in knnClasses:
			i = "%s%d"%(knnClass,higgsNum)
			f = expDir+"/"+i+"/powerlog.log"

			blocklen = 2000
			powerData = readPowerFile(f,blocklen,verbose)
			#powerData = rawFileToPowerFile(f,f+"_%d.gz"%(blocklen),blocklen,verbose)
			powerData = powerData[:, [0,3,4,5,6,7] ]
			powerStartTime = powerData[0,0]
			powerEndTime = powerData[-1,0]
			#powerData[:,0] -= powerStartTime

			fp = gzip.open(expDir+'/'+i+'/sparkEvents.json.gz','rb')
			j = json.load(fp)
			fp.close()
			jobEndTime = -float("inf")
			jobStartTime = float("inf")
			for event in j["events"]:
				if ("eventinfo" in event) and ("Event" in event["eventinfo"]) and ("timestamp" in event):
					if event["eventinfo"]["Event"] == "SparkListenerJobEnd":
						jobEndTime = np.max([jobEndTime, float(event["timestamp"])/1000.0])
					elif event["eventinfo"]["Event"] == "SparkListenerJobStart":
						jobStartTime = np.min([jobStartTime, float(event["timestamp"])/1000.0])

			assert(jobEndTime != -float("inf"))
			assert(jobStartTime != float("inf"))

			assert(jobStartTime > powerStartTime)
			assert(jobEndTime < powerEndTime)

			powerRange = np.logical_and(powerData[:,0] >= jobStartTime - jobTimeBuffer,powerData[:,0] <= jobEndTime + jobTimeBuffer)

			line, = ax.plot(powerData[powerRange,0]-jobStartTime, np.sum(powerData[powerRange,1:], axis=1))

			ax.annotate('Jobs Start', xy=(0, 200), xytext=(0, 175), arrowprops=dict(facecolor=line.get_color(), shrink=0.05) )
			ax.annotate('Jobs End', xy=(jobEndTime-jobStartTime, 200), xytext=(jobEndTime-jobStartTime, 175), arrowprops=dict(facecolor=line.get_color(), shrink=0.05) )

		ax.legend(knnClasses)
		ax.set_title("Higgs dataset with argument = %d"%(higgsNum))
		ax.set_xlabel('Time (seconds)')
		ax.set_ylabel('Cluster power (W)')
		ax.set_ylim(150,450)
		fig.canvas.draw()

		fig.set_size_inches(width, height, forward=True)
		fig.tight_layout()
		plt.show(fig)

		if saveFig:
			plt.savefig(expDir+"/higgs%d.png"%(higgsNum) )#, bbox_inches='tight')

		if not showFig:
			plt.close(fig)
