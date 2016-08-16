#!/usr/bin/python

# -*- coding: utf-8 -*-
"""
Created on Fri Jan 08 09:57:53 2016

@author: Josiah
"""

import numpy as np
import json
import gzip
import sys
import cPickle as pickle

def generateQueryTimeFile(expDir, filename, jobGrouping, verbose=False):
	jobs = {}

	fp = gzip.open(expDir+"/sparkEvents.json.gz",'rb')
	j = json.load(fp)
	fp.close()
	minJob = len(j["events"])
	maxJob = 0
	# Collect all of the job events.
	for event in j["events"]:
		if ("eventinfo" in event) and ("Event" in event["eventinfo"]):
			if event["eventinfo"]["Event"] in ["SparkListenerJobStart","SparkListenerJobEnd"]:
				timestamp = float(event["timestamp"])/1000.0
				jobID = int(event["eventinfo"]["Job ID"])

				if jobID < minJob:
					minJob = jobID
				if jobID > maxJob:
					maxJob = jobID

				if event["eventinfo"]["Event"] == "SparkListenerJobStart":
					scopeName = json.loads(event["eventinfo"]["Properties"]["spark.rdd.scope"])["name"]
					if jobID in jobs:
						jobs[jobID]["start"] = timestamp
						jobs[jobID]["name"] = scopeName
					else:
						jobs[jobID] = {"name":scopeName, "start":timestamp}
				else:
					if jobID in jobs:
						jobs[jobID]["end"] = timestamp
					else:
						jobs[jobID] = {"end":timestamp}

	jobID = minJob
	counter = 0
	lenJobGrouping = len(jobGrouping)

	# Remove anything unusual from the beginning
	keys = list(sorted(jobs))
	for key in keys:
		if "name" in jobs[key]:
			if jobs[key]["name"] != jobGrouping[0]:
				del jobs[key]
			else:
				break
		else:
			print "removing %s"%(jobs[key])
			del jobs[key]

	# Remove anything unusual from the end
	keys = list(sorted(jobs,reverse=True))
	for key in keys:
		if "name" in jobs[key]:
			if jobs[key]["name"] != jobGrouping[-1]:
				del jobs[key]
			else:
				break
		else:
			print "removing %s"%(jobs[key])
			del jobs[key]

	assert( len(jobs) > 0 )
	# Go through the jobs and the groupings in order
	queries = []
	#revJobGrouping = list(reversed(jobGrouping))
	for jobID in sorted(jobs,reverse=False):
		if "name" in jobs[jobID]:
			# Check to make sure the order is what we expect
			if jobs[jobID]["name"] == jobGrouping[counter]:

				# This is the first job in the group
				if counter == 0:
					startTime = jobs[jobID]["start"]

				# This is the last job in the group
				elif counter == lenJobGrouping-1:
					endTime = jobs[jobID]["end"]
					queries.append( [startTime, endTime] )

			else:
				raise ValueError("Job %s/%s didn't have the right job grouping: %s"%(expDir,filename, [jobs[jid]["name"] for jid in sorted(jobs)] ))
			counter = np.mod(counter + 1, lenJobGrouping)

	data = np.array(queries)

	fp = open(expDir+"/"+filename,"wb")
	pickle.dump(data,fp,-1)
	fp.close()

	return data

def readQueryTime(expDir,filename,jobGrouping,verbose=False):
	try:
		if verbose:
			print "Loading data from "+expDir+"/"+filename+"..."

		fp = open(expDir+"/"+filename,"rb")
		data = pickle.load(fp)
		fp.close()

	except IOError as err:
		if verbose:
			print "Does not exist (%s). Attempting to create..."%(err)
		data = generateQueryTimeFile(expDir,filename, jobGrouping, verbose)

	return data

if __name__ == "__main__":
	verbose = True

	powerCap = 35
	dim = 28
	knnClass = "KnnHorizontal"
	expID = "Jul28_powercap"
	exp = expID+"/exp%d"%(powerCap)
	i = "%s%d"%(knnClass,dim)
	expDir = "experiments/"+exp+"/"+i

	if knnClass == "KnnArray":
		jobGrouping = ["sortByKey", "take", "collect"]
	elif knnClass == "KnnHorizontal":
		jobGrouping = ["sortByKey", "take", "take"]

	#queries = generateQueryTimeFile(expDir,"job.queries",jobGrouping,verbose)
	queries = readQueryTime("experiments/Aug02_powercap1/exp28/Aggregation-BSI-Synth","job.queries",["sortByKey", "take", "take"],verbose)