verbose = True
saveFig = True
showFig = False



if verbose:
	print "Loading general modules..."
import numpy as np
import gzip
import json
#import exceptions

if verbose:
	print "Loading matplotlib module..."
import matplotlib.pyplot as plt
import os
from scipy.interpolate import interp1d

if verbose:
	print "Loading custom functions..."

#from defaultGetPower import readPowerFile
from my_plot_common import get_cmap, get_ensemblePowerData, get_jobEnergyData

figTemplate = {
	"uncapped_power_vs_time_%s":{
		"title":"Example Uncapped Power for %s",
		"xlabel":"Time (s)",
		"ylabel":"Power (W)",
		"generate": True,
		#"ylim": (100,350),
		#"xlim": (0,25),
		"powerCap": 35 },
	"capped_power_vs_time_%s":{
		"title":"Example Capped Power for %s",
		"xlabel":"Time (s)",
		"ylabel":"Power (W)",
		#"ylim": (100,350),
		#"xlim": (0,25),
		"generate": True,
		"powerCap": 25 } }

powerCaps = np.array(list(set([figTemplate[figName]["powerCap"] for figName in figTemplate])))

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

legacyData = {}
legacyData["powerData"] = get_ensemblePowerData(legacyUIDs,powerCaps,legacyExpNames,verbose)
legacyData["energyData"] = get_jobEnergyData(legacyUIDs,powerCaps,legacyExpNames,verbose)


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

#powerCaps = [22, 35]
expData = {}
expData["powerData"] =  get_ensemblePowerData(uids,powerCaps,expNames,verbose)
expData["energyData"] = get_jobEnergyData(uids,powerCaps,expNames,verbose)

# Update the data to incorporate the old data
for expName in legacyExpNames:
	newExpName = legacyExpNames[expName]
	for e in expData:
		if not (newExpName in expData[e]):
			expData[e][newExpName] = {}
		for powerCap in legacyData[e][expName]:
			expData[e][newExpName][powerCap] = legacyData[e][expName][powerCap]

# Organize the experiment data by according to expDataSorted[dataset][queryType][algorithm] [powerCap][uid]
expDataSorted = {}
for e in expData:
	for expName in expData[e]:
		queryType,algorithm,dataset = expName.split("-")
		if not (dataset in expDataSorted):
			expDataSorted[dataset] = {}
		if not (queryType in expDataSorted[dataset]):
			expDataSorted[dataset][queryType] = {}
		if not (algorithm in expDataSorted[dataset][queryType]):
			expDataSorted[dataset][queryType][algorithm] = {}

		for powerCap in expData[e][expName]:
			if not (powerCap in expDataSorted[dataset][queryType][algorithm]):
				expDataSorted[dataset][queryType][algorithm][powerCap] = {}

			expDataSorted[dataset][queryType][algorithm][powerCap][e] = expData[e][expName][powerCap]


height=7
width=7

# A 360 degree color wheel
cmap = get_cmap(360)

linespecs = [	{ "color":cmap(0),
	"linestyle":"solid",
	"marker":"*",
	"markerfacecolor":cmap(0),
	"markeredgecolor":cmap(0),
	"markersize":4},
	{"color":cmap(250),
	"linestyle":"solid",
	"marker":'',
	"markerfacecolor":None,
	"markeredgecolor":None,
	"markersize":3} ]


baselinePowerToSubtract = 203.721

figs = {}
for dataset in expDataSorted:
	for queryType in expDataSorted[dataset]:
		# This is a single set of plots.
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
				if (dataset == "Higgs"):
					figs[figName]["ylim"] = (100,350)

		# These are the different lines on the plots
		counter = 0
		for algorithm in sorted(expDataSorted[dataset][queryType]):
			edqa = expDataSorted[dataset][queryType][algorithm]

			linespec = linespecs[np.mod(counter,len(linespecs))]
			minx = -2.0
			maxx = float("-inf")
			for figName in newFigs:
				powerCap = figs[figName]["powerCap"]
				if figs[figName]["generate"] and (powerCap in edqa):
					ensemblePower = edqa[powerCap]["powerData"]["power"]
					ensembleTime = edqa[powerCap]["powerData"]["time"]
					figs[figName]["ax"].plot(ensembleTime,
						ensemblePower,
						color=linespec["color"],
						linestyle=linespec["linestyle"],
						marker=linespec["marker"],
						markerfacecolor=linespec["markerfacecolor"],
						markeredgecolor=linespec["markeredgecolor"],
						markersize=linespec["markersize"])

					xlim = figs[figName]["ax"].get_xlim()
					if not ("ylim" in figs[figName]):
						ylim = figs[figName]["ax"].get_ylim()
					else:
						ylim = figs[figName]["ylim"]

					jobDuration = np.mean([ edqa[powerCap]["energyData"][uid]["jobDuration"] for uid in edqa[powerCap]["energyData"] ])

					f = interp1d(ensembleTime,ensemblePower)

					powerAtJobStart = f(0.0)
					figs[figName]["ax"].annotate('Start',
						xy=(0.0, powerAtJobStart),
						xytext=(0.0, ylim[0] + 50),
						arrowprops=dict(edgecolor="black",
							facecolor=linespec["markerfacecolor"],
							shrink=0.05,
							headwidth=1.5,
							width=0.5,
							frac=0.025),
						horizontalalignment="left",
						verticalalignment="bottom" )

					powerAtJobEnd = f(jobDuration)
					figs[figName]["ax"].annotate("%s end"%(algorithm),
						xy=(jobDuration, powerAtJobEnd),
						xytext=(jobDuration, ylim[0] + 50),
						arrowprops=dict(edgecolor="black",
							facecolor=linespec["markerfacecolor"],
							shrink=0.05,
							headwidth=1.5,
							width=0.5,
							frac=0.025),
						horizontalalignment="right",
						verticalalignment="bottom" )

					#if xlim[0] < minx:
					#	minx = xlim[0]
					if xlim[1] > maxx:
						maxx = xlim[1]
					figs[figName]["legend"].append(algorithm)

			for figName in newFigs:
				if figs[figName]["generate"]:
					figs[figName]["ax"].set_xlim( (minx,maxx) )

			counter += 1

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
		ax.legend(figs[figId]["legend"],loc='lower right')
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
		plt.savefig("plots/timePlots/%s.pdf"%(figId))#, bbox_inches='tight')
		#plt.savefig("experiments/higgs_%s_%s.png"%(figId,"-".join(expIDs)) )#, bbox_inches='tight')

	if not showFig:
		plt.close(fig)