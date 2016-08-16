verbose = True
saveFig = True
showFig = False



if verbose:
	print "Loading general modules..."
import numpy as np
import os.path
import sys
#import exceptions

if verbose:
	print "Loading matplotlib module..."
import matplotlib.pyplot as plt

if verbose:
	print "Loading custom functions..."

from my_plot_common import get_cmap, get_mean_with_err, get_jobEnergyData, get_ensemblePowerData

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
#expNames.append("Aggregation-Arrays-Synth")

#powerCaps = [22, 35]
expData = {}
expData["powerData"] = get_ensemblePowerData(uids,powerCaps,expNames,verbose)
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


baselinePowerToSubtract = 203.721

###################
# Power Cap Plots #
###################

pcorder = np.argsort(powerCaps)
energies = {}
counter = {}
for dataset in sorted(expDataSorted):
	for queryType in sorted(expDataSorted[dataset]):
		for algorithm in sorted(expDataSorted[dataset][queryType]):
			dur = []
			#mpwr = []
			ppwr = []
			energ = []
			edqa = expDataSorted[dataset][queryType][algorithm]
			for powerCap in sorted(edqa):
				#mpwr.append([ edqa[powerCap]["energyData"][uid]["meanPower"] - baselinePowerToSubtract for uid in edqa[powerCap] ])
				p = edqa[powerCap]["powerData"]["power"]
				ppwr.append( np.max(p[ np.logical_and( p > 0 , p < float("inf") ) ]) )
				dur.append([ edqa[powerCap]["energyData"][uid]["jobDuration"] for uid in edqa[powerCap]["energyData"] ])
				energ.append( [ edqa[powerCap]["energyData"][uid]["jobDuration"]*(edqa[powerCap]["energyData"][uid]["meanPower"] - baselinePowerToSubtract)/1000.0 for uid in edqa[powerCap]["energyData"] ] )

			#power,perr = get_mean_with_err(ppwr,axis=1,ragged=True)
			power = np.array(ppwr)
			energy,eerr = get_mean_with_err(energ,axis=1,ragged=True)
			duration,derr = get_mean_with_err(dur,axis=1,ragged=True)


			if not (queryType in energies):
				energies[queryType] = {}
				counter[queryType] = {"count":0}
			if not (dataset in energies[queryType]):
				energies[queryType][dataset] = {}
				counter[queryType][dataset] = {"count":0}
			if not (algorithm in energies[queryType][dataset]):
				energies[queryType][dataset][algorithm] = {"energy":energy,"duration":duration,"power":power}
				counter[queryType][dataset][algorithm] = {"count":0}

			counter[queryType]["count"]  += 1
			counter[queryType][dataset]["count"]  += 1
			counter[queryType][dataset][algorithm]["count"] += 1

print ""
print ""
print "\\begin{table}"
print "   \\centering"
print "   \\caption{Ratio of Array energy to BSI energy}"
print "   \\begin{tabular}{*7r}\\toprule"

print "\t&\t&\t&\t& "+" &\t".join( [ "Cap %d W"%(e) for e in powerCaps[ np.array([pcorder[0], pcorder[-1]]) ]] )+" &\tPeak power \\\\ \midrule"

for queryType in sorted(energies):
	print "\\multirow{%d}{*}{\\rotatebox[origin=c]{90}{%s}}"%(counter[queryType]["count"],queryType)
	datasetCount = 0
	for dataset in sorted(energies[queryType]):
		print "\t&\t\\multirow{%d}{*}{%s}"%(counter[queryType][dataset]["count"],dataset)
		energyRatio = energies[queryType][dataset]["Arrays"]["energy"]/energies[queryType][dataset]["BSI"]["energy"]
		durationRatio = energies[queryType][dataset]["Arrays"]["duration"]/energies[queryType][dataset]["BSI"]["duration"]

		#print queryType+extraTab1+"\t& "+dataset+extraTab2+"\t& energy\t& "+" & ".join(["%5.1f"%(e) for e in energyRatio ])+" \\\\"
		#print queryType+extraTab1+"\t& "+dataset+extraTab2+"\t& duration\t& "+" & ".join(["%5.1f"%(e) for e in durationRatio ])+" \\\\"
		#if datasetCount == 0:
		#	prefix = "\t\t&\t"
		#else:
		#	prefix = "\t&\t&\t"

		if datasetCount == len(energies[queryType]):
			rule = "\\midrule"
		else:
			rule = "\\cmidrule{4-5}\\cmidrule{6-6}"

		arraysPeakPower = energies[queryType][dataset]["Arrays"]["power"][-1] - baselinePowerToSubtract
		bsiPeakPower = energies[queryType][dataset]["BSI"]["power"][-1] - baselinePowerToSubtract

		print "\t\t&\tenergy\t&\t"+" &\t".join(["%.1f"%(e) for e in energyRatio[ [0,-1] ] ])+" &\t"+"Array &\t%.1f W \\\\"%(arraysPeakPower)
		print "\t&\t&\t"+"time\t&\t"+" &\t".join(["%.1f"%(e) for e in durationRatio[ [0,-1] ] ])+" &\t"+"BSI &\t%.1f W \\\\ "%(bsiPeakPower)+rule
		datasetCount += 1
print "   \\bottomrule"
print "   \\end{tabular}"
print "\\end{table}"

"""
print ""
print ""
print "\\begin{table}"
print "   \\centering"
print "   \\caption{Average ratio of Array to BSI energy and duration across all power caps}"
print "   \\begin{tabular}{*8r}\\toprule"

firstLine = []
secondLine = []
thirdLine = []
fourthLine = []
for dataset in sorted(energies):
	for queryType in sorted(energies[dataset]):
		firstLine.append(dataset)
		secondLine.append(queryType)

		# Ratio of averages
		#arrayEnergy = np.mean(energies[dataset][queryType]["array"]["energy"])
		#bsiEnergy = np.mean(energies[dataset][queryType]["bsi"]["energy"])
		#arrayDuration = np.mean(energies[dataset][queryType]["array"]["dur"])
		#bsiDuration = np.mean(energies[dataset][queryType]["bsi"]["dur"])
		#thirdLine.append("%0.2f"%(arrayEnergy/bsiEnergy))
		#fourthLine.append("%0.2f"%(arrayDuration/bsiDuration))

		# Average of ratios
		arrayEnergy = energies[dataset][queryType]["array"]["energy"]
		bsiEnergy = energies[dataset][queryType]["bsi"]["energy"]
		arrayDuration = energies[dataset][queryType]["array"]["dur"]
		bsiDuration = energies[dataset][queryType]["bsi"]["dur"]
		thirdLine.append("%0.2f"%(np.mean(arrayEnergy/bsiEnergy)))
		fourthLine.append("%0.2f"%(np.mean(arrayDuration/bsiDuration)))

print " & "+" & ".join(firstLine)+" \\\\ "
print " & "+" & ".join(secondLine)+" \\\\ \\midrule"
print "energy & "+" & ".join(thirdLine)+" \\\\ "
print "duration & "+" & ".join(fourthLine)+" \\\\ "
print "   \\bottomrule"
print "   \\end{tabular}"
print "\\end{table}"

print ""
print ""
print "\\begin{table}"
print "   \\centering"
print "   \\caption{Peak of average unconstrained power}"
print "   \\begin{tabular}{*9r}\\toprule"


for queryType in sorted(energies):
	queryTypes = []
	datasets = []
	arrayPowers = []
	bsiPowers = []
	for dataset in sorted(energies[queryType]):
		queryTypes.append(queryType)
		datasets.append(dataset)
		arrayPowers.append("%6.2f"%(energies[queryType][dataset]["Arrays"]["power"][-1]))
		bsiPowers.append("%6.2f"%(energies[queryType][dataset]["BSI"]["power"][-1]))

	if queryType == "Aggregation":
		print "\t& "+" & ".join(datasets)+" \\\\ \\midrule"
	print "\multirow{2}{*}{"+queryType+"}"
	print "\t & Arrays\t& "+" & ".join(arrayPowers)+" \\\\ "
	print "\t & BSI\t\t& "+" & ".join(bsiPowers)+" \\\\ \\midrule "
print "   \\bottomrule"
print "   \\end{tabular}"
print "\\end{table}"

"""
