verbose = True
saveFig = True
showFig = False



if verbose:
	print "Loading general modules..."
import numpy as np
import os.path
#import exceptions

if verbose:
	print "Loading matplotlib module..."
import matplotlib.pyplot as plt

if verbose:
	print "Loading custom functions..."

from my_plot_common import get_cmap, get_mean_with_err, get_jobEnergyData

powerCaps = np.array([22, 35, 25, 32, 28])

# Special case for previously-collected HIGGS dataset
legacyExpNames = {"KnnArray28":"Knn-Arrays-Higgs","KnnHorizontal28":"Knn-BSI-Higgs"}
legacyUIDs = ["Jul27_powercap",
		"Jul28_powercap",
		"Jul18_28_powercap",
		"Jul28_powercap2",
		"Jul28_powercap3",
		"Jul28_powercap4",
		"Jul28_powercap5",
		"Jul29_powercap",
		"Jul29_powercap2",
		"Jul29_powercap3"]

legacyData = get_jobEnergyData(legacyUIDs,powerCaps,legacyExpNames,verbose)

uids =	["Aug03_powercap1",
		"Aug03_powercap2",
		"Aug03_powercap3",
		"Aug03_powercap4",
		"Aug03_powercap5",
		"Aug03_powercap6",
		"Aug03_powercap7",
		"Aug03_powercap8",
		"Aug03_powercap9",
		"Aug03_powercap10" ]


algorithms = ["Arrays","BSI"]
queryTypes = ["Knn","TopK","Aggregation"]
datasets = ["Higgs","Images","Synth","SynthSmall"]

expNames = []
for queryType in queryTypes:
	for algorithm in algorithms:
		for dataset in datasets:
			expName = queryType+"-"+algorithm+"-"+dataset
			if os.path.exists("spark_"+expName+"_template.sh"):
				expNames.append(expName)
#expNames.append("Aggregation-Arrays-Synth")

#powerCaps = [22, 35]
expData = get_jobEnergyData(uids,powerCaps,expNames,verbose)

# Update the data to incorporate the old data
for expName in legacyExpNames:
	for powerCap in legacyData[expName]:
		for uid in legacyData[expName][powerCap]:
			newExpName = legacyExpNames[expName]
			if not (newExpName in expData):
				expData[newExpName] = {}
			if not (powerCap in expData[newExpName]):
				expData[newExpName][powerCap] = {}

			expData[newExpName][powerCap][uid] = legacyData[expName][powerCap][uid]

# Organize the experiment data by according to expDataSorted[dataset][queryType][algorithm] [powerCap][uid]
expDataSorted = {}
for expName in expData:
	queryType,algorithm,dataset = expName.split("-")
	if not (dataset in expDataSorted):
		expDataSorted[dataset] = {}
	if not (queryType in expDataSorted[dataset]):
		expDataSorted[dataset][queryType] = {}

	expDataSorted[dataset][queryType][algorithm] = expData[expName]


height=7
width=7

colorStartIndex = 0

baselinePowerToSubtract = 203.721

figTemplate = {
	"energy_vs_powercap_%s": {
		"title":"Average Energy for %s",
		"xlabel":"Per-Processor Power Cap (W)",
		"ylabel":"Energy (kJ)",
		"xlim": (21,36),
		#"ylim": (0,7),
		"generate": False},
	"duration_vs_powercap_%s": {
		"title":"Average Duration for %s",
		"xlabel":"Per-Processor Power Cap (W)",
		"ylabel":"Duration (s)",
		"xlim": (21,36),
		#"ylim": (0,100),
		"generate": True}}

# A 360 degree color wheel
cmap = get_cmap(360)

linespecs = [ {"color":cmap(0),
	"linestyle":"solid",
	"marker":"*",
	"markerfacecolor":cmap(0),
	"markeredgecolor":"black",
	"markersize":7},
	{"color":cmap(250),
	"linestyle":"dashed",
	"marker":"o",
	"markerfacecolor":cmap(250),
	"markeredgecolor":"black",
	"markersize":5} ]

figs = {}
###################
# Power Cap Plots #
###################

pcorder = np.argsort(powerCaps)

for dataset in expDataSorted:
	for queryType in expDataSorted[dataset]:
		# This is a single set of plots
		newFigs = {}
		for plotType in figTemplate:
			figName = plotType%(queryType+"-"+dataset)

			if figTemplate[plotType]["generate"]:
				newFigs[figName] = figTemplate[plotType].copy()
				newFigs[figName]["title"] = figTemplate[plotType]["title"]%(queryType+" "+dataset)

				fig = plt.figure()
				ax = fig.add_subplot(111)
				newFigs[figName]["fig"] = fig
				newFigs[figName]["ax"] = ax
				newFigs[figName]["legend"] = []

				figs[figName] = newFigs[figName]

		# These are the different lines on the plot
		counter = 0
		for algorithm in sorted(expDataSorted[dataset][queryType]):
			# The rows are different power caps (increasing cap)
			# The columns are different experiments
			dur = []
			mpwr = []
			energ = []
			edqa = expDataSorted[dataset][queryType][algorithm]
			for powerCap in sorted(edqa):
				dur.append([ edqa[powerCap][uid]["jobDuration"] for uid in edqa[powerCap] ])
				mpwr.append([ edqa[powerCap][uid]["meanPower"] for uid in edqa[powerCap] ])
				energ.append( [ edqa[powerCap][uid]["jobDuration"]*(edqa[powerCap][uid]["meanPower"] - baselinePowerToSubtract)/1000.0 for uid in edqa[powerCap] ] )

			#dur = np.array(dur)
			#mpwr = np.array(mpwr)
			#enrg = dur*(mpwr - baselinePowerToSubtract)/1000.0

			duration,derr = get_mean_with_err(dur,axis=1,ragged=True)
			energy,eerr = get_mean_with_err(energ,axis=1,ragged=True)

			linespec = linespecs[np.mod(counter,len(linespecs))]

			for figName in newFigs:
				if figName.startswith("energy"):
					figs[figName]["ax"].errorbar(powerCaps[pcorder],
						energy,
						yerr=eerr,
						color=linespec["color"],
						linestyle=linespec["linestyle"],
						marker=linespec["marker"],
						markerfacecolor=linespec["markerfacecolor"],
						markeredgecolor=linespec["markeredgecolor"],
						markersize=linespec["markersize"])
					figs[figName]["legend"].append(algorithm)

				elif figName.startswith("duration"):
					figs[figName]["ax"].errorbar(powerCaps[pcorder],
						duration,
						yerr=derr,
						color=linespec["color"],
						linestyle=linespec["linestyle"],
						marker=linespec["marker"],
						markerfacecolor=linespec["markerfacecolor"],
						markeredgecolor=linespec["markeredgecolor"],
						markersize=linespec["markersize"])
					figs[figName]["legend"].append(algorithm)

			counter += 1

figs["duration_vs_powercap_Knn-Higgs"]["ylim"] = (0,100)
figs["duration_vs_powercap_TopK-Higgs"]["ylim"] = (0,100)
figs["duration_vs_powercap_Aggregation-Higgs"]["ylim"] = (0,25)

####################
# Save the figures #
####################
for figId in figs:
	fig = figs[figId]["fig"]
	ax = figs[figId]["ax"]

	fig.set_size_inches(width, height, forward=True)
	if "ylim" in figs[figId]:
		ax.set_ylim(figs[figId]["ylim"])
	if "xlim" in figs[figId]:
		ax.set_xlim(figs[figId]["xlim"])
	if "legend" in figs[figId]:
		#box = ax.get_position()
		#ax.set_position([box.x0, box.y0, box.width * 0.75, box.height])
		#ax.legend(figs[figId]["legend"],loc='center left', bbox_to_anchor=(1, 0.5))
		ax.legend(figs[figId]["legend"],loc='upper right')

	if "title" in figs[figId]:
		ax.set_title(figs[figId]["title"])
	if "xlabel" in figs[figId]:
		ax.set_xlabel(figs[figId]["xlabel"])
	if "ylabel" in figs[figId]:
		ax.set_ylabel(figs[figId]["ylabel"])
	fig.canvas.draw()
	plt.show(fig)

	if saveFig:
		plt.figure(fig.number)
		plt.savefig("plots/durationPlots/%s.pdf"%(figId))#, bbox_inches='tight')
		#plt.savefig("experiments/higgs_%s_%s.png"%(figId,"-".join(expIDs)) )#, bbox_inches='tight')

	if not showFig:
		plt.close(fig)